#include "mvc/controller/master_controller.h"

#include "bus/master_socket.h"
#include "kickcat/Bus.h"
#include "kickcat/Link.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <cstdio>

using namespace std::chrono_literals;

namespace ethercat_sim::app::master {

MasterController::MasterController(std::string endpoint, int cycle_us)
    : endpoint_(std::move(endpoint)), cycle_us_(cycle_us) {}

MasterController::~MasterController()
{
    stop();
}

void MasterController::start()
{
    if (th_.joinable()) return;
    stop_.store(false);
    th_ = std::thread(&MasterController::run_, this);
}

void MasterController::stop()
{
    stop_.store(true);
    if (th_.joinable()) th_.join();
}

void MasterController::ensureBus_()
{
    if (bus_) return;
    sock_ = std::make_shared<bus::MasterSocket>(endpoint_);
    sock_->open("");
    sock_->setTimeout(200ms);
    // Use the same socket for both nominal and redundancy interfaces
    // This avoids the WriteThenRead error when using SocketNull
    link_ = std::make_shared<kickcat::Link>(sock_, sock_, []{});
    link_->setTimeout(200ms);
    bus_ = std::make_unique<kickcat::Bus>(link_);
}

void MasterController::scan()
{
    try {
        ensureBus_();
        int32_t n = bus_->detectSlaves();
        model_->setDetectedSlaves(n);
        // snapshot slaves
        std::vector<SlaveRow> rows;
        rows.reserve(bus_->slaves().size());
        for (auto const& s : bus_->slaves()) {
            SlaveRow r;
            r.address = s.address;
            // Map state to text
            switch (static_cast<kickcat::State>(s.al_status)) {
                case kickcat::State::INIT: r.state = "INIT"; break;
                case kickcat::State::PRE_OP: r.state = "PRE_OP"; break;
                case kickcat::State::BOOT: r.state = "BOOT"; break;
                case kickcat::State::SAFE_OP: r.state = "SAFE_OP"; break;
                case kickcat::State::OPERATIONAL: r.state = "OP"; break;
                default: r.state = "INVALID"; break;
            }
            std::ostringstream oss; oss << "0x" << std::hex << std::uppercase << s.al_status_code;
            r.al_code = oss.str();
            rows.push_back(std::move(r));
        }
        model_->setSlaves(std::move(rows));
        model_->setStatus("scan ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("scan err: ") + e.what());
    }
}

void MasterController::initPreop()
{
    try {
        ensureBus_();
        std::cout << "[a-master] Calling bus->init()\n";
        bus_->init();
        std::cout << "[a-master] bus->init() completed\n";
        
        // Wait a bit for slaves to process state change
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // refresh snapshot
        std::vector<SlaveRow> rows;
        rows.reserve(bus_->slaves().size());
        std::cout << "[a-master] Checking " << bus_->slaves().size() << " slaves after init\n";
        for (auto const& s : bus_->slaves()) {
            SlaveRow r;
            r.address = s.address;
            std::cout << "[a-master] Slave " << s.address << " AL status: 0x" 
                      << std::hex << static_cast<int>(s.al_status) 
                      << " code: 0x" << s.al_status_code << std::dec << "\n";
            switch (static_cast<kickcat::State>(s.al_status)) {
                case kickcat::State::INIT: r.state = "INIT"; break;
                case kickcat::State::PRE_OP: r.state = "PRE_OP"; break;
                case kickcat::State::BOOT: r.state = "BOOT"; break;
                case kickcat::State::SAFE_OP: r.state = "SAFE_OP"; break;
                case kickcat::State::OPERATIONAL: r.state = "OP"; break;
                default: r.state = "INVALID"; break;
            }
            std::ostringstream oss; oss << "0x" << std::hex << std::uppercase << s.al_status_code;
            r.al_code = oss.str();
            rows.push_back(std::move(r));
        }
        model_->setSlaves(std::move(rows));
        model_->setPreop(bus_->slaves().size() > 0 && bus_->slaves()[0].al_status == static_cast<uint8_t>(kickcat::State::PRE_OP));
        model_->setStatus("preop ok");
    } catch (std::exception const& e) {
        std::cerr << "[a-master] initPreop exception: " << e.what() << "\n";
        model_->setStatus(std::string("preop err: ") + e.what());
    } catch (...) {
        std::cerr << "[a-master] initPreop unknown exception\n";
        model_->setStatus("preop err: unknown exception");
    }
}

void MasterController::requestOperational()
{
    try {
        ensureBus_();
        std::vector<uint8_t> iomap(4096, 0);
        bus_->createMapping(iomap.data());
        bus_->requestState(kickcat::State::OPERATIONAL);
        bus_->waitForState(kickcat::State::OPERATIONAL, 500ms, [&]{ bus_->processDataReadWrite([](auto const&){}); });
        // refresh snapshot
        std::vector<SlaveRow> rows;
        rows.reserve(bus_->slaves().size());
        for (auto const& s : bus_->slaves()) {
            SlaveRow r;
            r.address = s.address;
            switch (static_cast<kickcat::State>(s.al_status)) {
                case kickcat::State::INIT: r.state = "INIT"; break;
                case kickcat::State::PRE_OP: r.state = "PRE_OP"; break;
                case kickcat::State::BOOT: r.state = "BOOT"; break;
                case kickcat::State::SAFE_OP: r.state = "SAFE_OP"; break;
                case kickcat::State::OPERATIONAL: r.state = "OP"; break;
                default: r.state = "INVALID"; break;
            }
            std::ostringstream oss; oss << "0x" << std::hex << std::uppercase << s.al_status_code;
            r.al_code = oss.str();
            rows.push_back(std::move(r));
        }
        model_->setSlaves(std::move(rows));
        model_->setOperational(true);
        model_->setStatus("op ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("op err: ") + e.what());
    }
}

void MasterController::run_()
{
    // Removed automatic initial sequence - wait for user to press 's' key
    // This allows slaves to be ready before scanning
    model_->setStatus("ready - press 's' to scan");

    auto period = std::chrono::microseconds(cycle_us_);
    while (!stop_.load()) {
        try {
            ensureBus_();
            bus_->sendNop([](auto const&){});
            bus_->finalizeDatagrams();
            bus_->processAwaitingFrames();
            // periodic refresh of states (lightweight)
            std::vector<SlaveRow> rows;
            rows.reserve(bus_->slaves().size());
            for (auto const& s : bus_->slaves()) {
                SlaveRow r;
                r.address = s.address;
            switch (static_cast<kickcat::State>(s.al_status)) {
                case kickcat::State::INIT: r.state = "INIT"; break;
                case kickcat::State::PRE_OP: r.state = "PRE_OP"; break;
                case kickcat::State::BOOT: r.state = "BOOT"; break;
                case kickcat::State::SAFE_OP: r.state = "SAFE_OP"; break;
                case kickcat::State::OPERATIONAL: r.state = "OP"; break;
                default: r.state = "INVALID"; break;
            }
                std::ostringstream oss; oss << "0x" << std::hex << std::uppercase << s.al_status_code;
                r.al_code = oss.str();
                rows.push_back(std::move(r));
            }
            model_->setSlaves(std::move(rows));
        } catch (...) { /* ignore */ }
        std::this_thread::sleep_for(period);
    }
}

bool MasterController::sdoUpload(int slave_index, uint16_t index, uint8_t subindex, uint32_t& value)
{
    try {
        ensureBus_();
        auto& vec = bus_->slaves();
        if (slave_index < 0 || slave_index >= static_cast<int>(vec.size())) {
            model_->setSdoStatus("invalid slave index");
            return false;
        }
        uint32_t size = sizeof(uint32_t);
        bus_->readSDO(vec[slave_index], index, subindex, kickcat::Bus::Access::COMPLETE, &value, &size, std::chrono::milliseconds(500));
        char buf[16];
        std::snprintf(buf, sizeof(buf), "0x%08X", value);
        model_->setSdoValueHex(buf);
        model_->setSdoStatus("read ok");
        return true;
    } catch (std::exception const& e) {
        model_->setSdoStatus(std::string("read err: ") + e.what());
        return false;
    }
}

bool MasterController::sdoDownload(int slave_index, uint16_t index, uint8_t subindex, uint32_t value)
{
    try {
        ensureBus_();
        auto& vec = bus_->slaves();
        if (slave_index < 0 || slave_index >= static_cast<int>(vec.size())) {
            model_->setSdoStatus("invalid slave index");
            return false;
        }
        bus_->writeSDO(vec[slave_index], index, subindex, false, &value, sizeof(uint32_t), std::chrono::milliseconds(500));
        model_->setSdoStatus("write ok");
        return true;
    } catch (std::exception const& e) {
        model_->setSdoStatus(std::string("write err: ") + e.what());
        return false;
    }
}

} // namespace ethercat_sim::app::master
