#include "master_controller.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <kickcat/Error.h>
#include <sstream>
#include <vector>

#include "kickcat/Bus.h"
#include "kickcat/DebugHelpers.h"
#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"

#include "bus/master_socket.h"
#include "framework/logger/logger.h"

using namespace std::chrono_literals;

namespace ethercat_sim::app::master
{

namespace
{

constexpr uint8_t kStateMask = 0x0F;

kickcat::State normalizeState(uint8_t raw)
{
    return static_cast<kickcat::State>(raw & kStateMask);
}

std::string stateToString(kickcat::State state)
{
    state = normalizeState(static_cast<uint8_t>(state));
    switch (state)
    {
    case kickcat::State::INIT:
        return "INIT";
    case kickcat::State::PRE_OP:
        return "PRE_OP";
    case kickcat::State::BOOT:
        return "BOOT";
    case kickcat::State::SAFE_OP:
        return "SAFE_OP";
    case kickcat::State::OPERATIONAL:
        return "OP";
    default:
        return "INVALID";
    }
}

std::string formatAlStatusCode(uint16_t code)
{
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << code;
    return oss.str();
}

SlavesRow makeRow(kickcat::Slave const& slave)
{
    SlavesRow row;
    row.address = slave.address;
    row.state   = stateToString(normalizeState(slave.al_status));
    row.al_code = formatAlStatusCode(slave.al_status_code);
    return row;
}

} // namespace

MasterController::MasterController(std::string endpoint, int cycle_us)
    : endpoint_(std::move(endpoint)), cycle_us_(cycle_us)
{
}

MasterController::~MasterController()
{
    stop();
}

void MasterController::start()
{
    if (th_.joinable())
    {
        return;
    }
    stop_.store(false);
    th_ = std::thread(&MasterController::run_, this);
}

void MasterController::stop()
{
    stop_.store(true);
    if (th_.joinable())
    {
        th_.join();
    }
}

void MasterController::ensureBus_()
{
    if (bus_)
    {
        return;
    }
    sock_ = std::make_shared<bus::MasterSocket>(endpoint_);
    sock_->open("");
    sock_->setTimeout(200ms);
    auto redundancy = std::make_shared<::kickcat::SocketNull>();
    link_           = std::make_shared<kickcat::Link>(sock_, redundancy, [] {});
    link_->setTimeout(200ms);
    bus_ = std::make_unique<kickcat::Bus>(link_);
    bus_->configureWaitLatency(1ms, 20ms);
}

int32_t MasterController::detectSlavesWithRetries_(int attempts, std::chrono::milliseconds delay)
{
    int32_t detected = 0;
    for (int i = 0; i < attempts; ++i)
    {
        detected = bus_->detectSlaves();
        model_->setDetectedSlaves(detected);
        if (detected > 0)
        {
            bus_->processAwaitingFrames();
            return detected;
        }
        bus_->sendNop([](auto const&) {});
        bus_->finalizeDatagrams();
        bus_->processAwaitingFrames();
        if (delay.count() > 0)
        {
            std::this_thread::sleep_for(delay);
        }
    }
    return detected;
}

void MasterController::refreshSlaveAlStatusUnlocked_()
{
    if (!bus_ || !link_)
    {
        return;
    }
    for (auto& slave : bus_->slaves())
    {
        uint8_t al_status       = slave.al_status;
        uint16_t al_status_code = slave.al_status_code;
        try
        {
            kickcat::sendGetRegister(*link_, slave.address, 0x0130, al_status);
            kickcat::sendGetRegister(*link_, slave.address, 0x0134, al_status_code);
            slave.al_status      = al_status;
            slave.al_status_code = al_status_code;
        }
        catch (...)
        {
            // keep cached value when register read fails
        }
    }
}

std::vector<SlavesRow> MasterController::snapshotSlavesUnlocked_()
{
    std::vector<SlavesRow> rows;
    if (!bus_)
    {
        return rows;
    }
    auto const& slaves = bus_->slaves();
    rows.reserve(slaves.size());
    for (auto const& slave : slaves)
    {
        rows.push_back(makeRow(slave));
    }
    return rows;
}

bool MasterController::allSlavesInStateUnlocked_(kickcat::State state) const
{
    if (!bus_)
    {
        return false;
    }
    auto const& slaves = bus_->slaves();
    if (slaves.empty())
    {
        return false;
    }
    return std::all_of(slaves.begin(), slaves.end(),
                       [state](auto const& s) { return normalizeState(s.al_status) == state; });
}

bool MasterController::requestAndWaitStateUnlocked_(kickcat::State target,
                                                    std::chrono::milliseconds timeout)
{
    if (!bus_)
    {
        return false;
    }

    auto const state_name = stateToString(target);

    if (allSlavesInStateUnlocked_(target))
    {
        return true;
    }

    try
    {
        bus_->requestState(target);
        bus_->finalizeDatagrams();
        bus_->processAwaitingFrames();
        bus_->waitForState(target, timeout,
                           [&]
                           {
                               bus_->sendNop([](auto const&) {});
                               bus_->finalizeDatagrams();
                               bus_->processAwaitingFrames();
                           });
        refreshSlaveAlStatusUnlocked_();
    }
    catch (std::exception const& e)
    {
        ethercat_sim::framework::logger::Logger::error("requestState(%s) failed: %s",
                                                       state_name.c_str(), e.what());
        refreshSlaveAlStatusUnlocked_();
        if (allSlavesInStateUnlocked_(target))
        {
            ethercat_sim::framework::logger::Logger::warn(
                "requestState(%s) threw, but all slaves already report target state",
                state_name.c_str());
            return true;
        }
        return false;
    }
    catch (...)
    {
        ethercat_sim::framework::logger::Logger::error("requestState(%s) threw unknown exception",
                                                       state_name.c_str());
        refreshSlaveAlStatusUnlocked_();
        if (allSlavesInStateUnlocked_(target))
        {
            ethercat_sim::framework::logger::Logger::warn(
                "requestState(%s) threw unknown exception, but target state already reached",
                state_name.c_str());
            return true;
        }
        return false;
    }
    return allSlavesInStateUnlocked_(target);
}

void MasterController::scan()
{
    try
    {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        int32_t n = detectSlavesWithRetries_(10, 100ms);
        if (n <= 0)
        {
            model_->setSlaves({});
            model_->setStatus("scan err: no slave detected");
            return;
        }
        refreshSlaveAlStatusUnlocked_();
        model_->setSlaves(snapshotSlavesUnlocked_());
        model_->setStatus("scan ok");
    }
    catch (std::exception const& e)
    {
        model_->setStatus(std::string("scan err: ") + e.what());
    }
}

void MasterController::initPreop()
{
    try
    {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        if (bus_->slaves().empty())
        {
            auto detected = detectSlavesWithRetries_(10, 100ms);
            if (detected <= 0)
            {
                model_->setSlaves({});
                model_->setPreop(false);
                model_->setStatus("preop err: no slave detected");
                return;
            }
        }

        bool initReached  = false;
        bool preopReached = false;

        try
        {
            ethercat_sim::framework::logger::Logger::info("Starting bus->init()");
            bus_->init();
            ethercat_sim::framework::logger::Logger::info("bus->init() completed");

            refreshSlaveAlStatusUnlocked_();
            preopReached = allSlavesInStateUnlocked_(kickcat::State::PRE_OP);
            ethercat_sim::framework::logger::Logger::info(
                std::string("After bus->init(): preopReached = ") +
                (preopReached ? "true" : "false"));
            initReached = preopReached || allSlavesInStateUnlocked_(kickcat::State::INIT);
            ethercat_sim::framework::logger::Logger::info(
                std::string("After bus->init(): initReached = ") +
                (initReached ? "true" : "false"));
            if (!preopReached)
            {
                ethercat_sim::framework::logger::Logger::warn(
                    "bus->init() completed but PRE_OP not confirmed, retrying manually");
            }
        }
        catch (std::exception const& init_err)
        {
            ethercat_sim::framework::logger::Logger::error("bus->init() failed: %s",
                                                           init_err.what());
            if (bus_)
            {
                for (auto const& slave : bus_->slaves())
                {
                    ethercat_sim::framework::logger::Logger::error(
                        "bus->init() diagnostics: addr=%u al_status=0x%02X al_code=0x%04X",
                        slave.address, static_cast<unsigned>(slave.al_status),
                        static_cast<unsigned>(slave.al_status_code));
                }
            }
        }

        constexpr auto state_timeout = 8000ms;
        if (!preopReached)
        {
            initReached =
                requestAndWaitStateUnlocked_(kickcat::State::INIT, state_timeout) || initReached;
            preopReached = requestAndWaitStateUnlocked_(kickcat::State::PRE_OP, state_timeout);
        }

        refreshSlaveAlStatusUnlocked_();
        initReached  = initReached || allSlavesInStateUnlocked_(kickcat::State::INIT);
        preopReached = preopReached || allSlavesInStateUnlocked_(kickcat::State::PRE_OP);

        auto rows = snapshotSlavesUnlocked_();
        model_->setSlaves(std::move(rows));
        model_->setPreop(preopReached);

        if (preopReached)
        {
            model_->setStatus("preop ok");
        }
        else if (!initReached)
        {
            model_->setStatus("preop err: INIT state not reached");
        }
        else
        {
            model_->setStatus("preop err: PRE_OP state mismatch");
        }
    }
    catch (std::exception const& e)
    {
        model_->setStatus(std::string("preop err: ") + e.what());
    }
    catch (...)
    {
        model_->setStatus("preop err: unknown exception");
    }
}

void MasterController::requestOperational()
{
    try
    {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        if (bus_->slaves().empty())
        {
            auto detected = detectSlavesWithRetries_(10, 100ms);
            if (detected <= 0)
            {
                model_->setStatus("op err: no slave detected");
                return;
            }
        }
        std::vector<uint8_t> iomap(4096, 0);
        bus_->createMapping(iomap.data());
        bus_->requestState(kickcat::State::OPERATIONAL);
        bus_->waitForState(kickcat::State::OPERATIONAL, 2000ms,
                           [&] { bus_->processDataReadWrite([](auto const&) {}); });
        refreshSlaveAlStatusUnlocked_();
        model_->setSlaves(snapshotSlavesUnlocked_());
        model_->setOperational(true);
        model_->setStatus("op ok");
    }
    catch (std::exception const& e)
    {
        model_->setStatus(std::string("op err: ") + e.what());
    }
}

void MasterController::run_()
{
    model_->setStatus("ready - press 's' to scan");

    auto period = std::chrono::microseconds(cycle_us_);
    while (!stop_.load())
    {
        try
        {
            std::vector<SlavesRow> rows;
            {
                std::lock_guard<std::mutex> guard(bus_mutex_);
                ensureBus_();
                bus_->sendNop([](auto const&) {});
                bus_->finalizeDatagrams();
                bus_->processAwaitingFrames();
                rows = snapshotSlavesUnlocked_();
            }
            model_->setSlaves(std::move(rows));
        }
        catch (...)
        {
            // ignore transient errors
        }
        std::this_thread::sleep_for(period);
    }
}

bool MasterController::sdoUpload(int slave_index, uint16_t index, uint8_t subindex, uint32_t& value)
{
    try
    {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        auto& vec = bus_->slaves();
        if (slave_index < 0 || slave_index >= static_cast<int>(vec.size()))
        {
            model_->setSdoStatus("invalid slave index");
            return false;
        }
        uint32_t size = sizeof(uint32_t);
        bus_->readSDO(vec[slave_index], index, subindex, kickcat::Bus::Access::COMPLETE, &value,
                      &size, std::chrono::milliseconds(500));
        char buf[16];
        std::snprintf(buf, sizeof(buf), "0x%08X", value);
        model_->setSdoValueHex(buf);
        model_->setSdoStatus("read ok");
        return true;
    }
    catch (std::exception const& e)
    {
        model_->setSdoStatus(std::string("read err: ") + e.what());
        return false;
    }
}

bool MasterController::sdoDownload(int slave_index, uint16_t index, uint8_t subindex,
                                   uint32_t value)
{
    try
    {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        auto& vec = bus_->slaves();
        if (slave_index < 0 || slave_index >= static_cast<int>(vec.size()))
        {
            model_->setSdoStatus("invalid slave index");
            return false;
        }
        bus_->writeSDO(vec[slave_index], index, subindex, false, &value, sizeof(uint32_t),
                       std::chrono::milliseconds(500));
        model_->setSdoStatus("write ok");
        return true;
    }
    catch (std::exception const& e)
    {
        model_->setSdoStatus(std::string("write err: ") + e.what());
        return false;
    }
}

} // namespace ethercat_sim::app::master
