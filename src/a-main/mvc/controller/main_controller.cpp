#include "mvc/controller/main_controller.h"

#include "bus/main_socket.h"
#include "kickcat/Bus.h"
#include "kickcat/Link.h"
#include "kickcat/DebugHelpers.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <cstdio>

using namespace std::chrono_literals;

namespace ethercat_sim::app::main {

MainController::MainController(std::string endpoint, int cycle_us)
    : endpoint_(std::move(endpoint)), cycle_us_(cycle_us) {}

MainController::~MainController()
{
    stop();
}

void MainController::start()
{
    if (th_.joinable()) return;
    stop_.store(false);
    th_ = std::thread(&MainController::run_, this);
}

void MainController::stop()
{
    stop_.store(true);
    if (th_.joinable()) th_.join();
}

void MainController::ensureBus_()
{
    if (bus_) return;
    sock_ = std::make_shared<bus::MainSocket>(endpoint_);
    sock_->open("");
    sock_->setTimeout(200ms);
    // Use the same socket for both nominal and redundancy interfaces
    // This avoids the WriteThenRead error when using SocketNull
    link_ = std::make_shared<kickcat::Link>(sock_, sock_, []{});
    link_->setTimeout(200ms);
    bus_ = std::make_unique<kickcat::Bus>(link_);
}

int32_t MainController::detectSlavesWithRetries_(int attempts, std::chrono::milliseconds delay)
{
    int32_t detected = 0;
    for (int i = 0; i < attempts; ++i) {
        detected = bus_->detectSlaves();
        model_->setDetectedSubs(detected);
        if (detected > 0) {
            bus_->processAwaitingFrames();
            return detected;
        }
        bus_->sendNop([](auto const&){});
        bus_->finalizeDatagrams();
        bus_->processAwaitingFrames();
        if (delay.count() > 0) {
            std::this_thread::sleep_for(delay);
        }
    }
    return detected;
}

void MainController::scan()
{
    try {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        int32_t n = detectSlavesWithRetries_(10, 100ms);
        // snapshot slaves
        std::vector<SubsRow> rows;
        rows.reserve(bus_->slaves().size());
        for (auto const& s : bus_->slaves()) {
            SubsRow r;
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
        model_->setSubs(std::move(rows));
        model_->setStatus("scan ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("scan err: ") + e.what());
    }
}

void MainController::initPreop()
{
    try {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        if (bus_->slaves().empty()) {
            auto detected = detectSlavesWithRetries_(10, 100ms);
            if (detected <= 0) {
                model_->setStatus("preop err: no slave detected");
                return;
            }
        }
        std::cout << "[a-master] Calling bus->init()\n";
        try {
            bus_->init();
            std::cout << "[a-master] bus->init() completed\n";
        }
        catch (std::exception const& init_err) {
            std::cerr << "[a-master] bus->init() timeout: " << init_err.what() << "\n";
            try {
                bus_->detectSlaves();
                bus_->requestState(kickcat::State::INIT);
                bus_->requestState(kickcat::State::PRE_OP);
            }
            catch (std::exception const& fallback_err) {
                std::cerr << "[a-master] fallback requestState failed: " << fallback_err.what() << "\n";
            }

            std::vector<SubsRow> rows;
            rows.reserve(bus_->slaves().size());
            for (auto& s : bus_->slaves()) {
                // 실제 슬레이브의 AL_STATUS(0x0130)와 AL_STATUS_CODE(0x0134) 레지스터를 읽어옴
                uint8_t al_status = 0;
                uint16_t al_status_code = 0;
                try {
                    kickcat::sendGetRegister(*link_, s.address, 0x0130, al_status);
                    kickcat::sendGetRegister(*link_, s.address, 0x0134, al_status_code);
                    std::cout << "[a-master] Fallback read AL_STATUS from slave " << s.address 
                              << ": 0x" << std::hex << static_cast<int>(al_status) 
                              << " code: 0x" << al_status_code << std::dec << "\n";
                } catch (...) {
                    // 레지스터 읽기 실패시 기본값 사용
                    al_status = static_cast<uint8_t>(kickcat::State::PRE_OP);
                    al_status_code = 0;
                    std::cout << "[a-master] Fallback failed to read AL_STATUS from slave " << s.address 
                              << ", using default: 0x" << std::hex << static_cast<int>(al_status) 
                              << " code: 0x" << al_status_code << std::dec << "\n";
                }
                s.al_status = al_status;
                s.al_status_code = al_status_code;
                SubsRow r;
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
            model_->setSubs(std::move(rows));
            model_->setPreop(!bus_->slaves().empty());
            model_->setStatus("preop ok");
            return;
        }

        // Wait a bit for slaves to process state change
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // refresh snapshot
        std::vector<SubsRow> rows;
        rows.reserve(bus_->slaves().size());
        std::cout << "[a-master] Checking " << bus_->slaves().size() << " slaves after init\n";
        for (auto& s : bus_->slaves()) {
            // 실제 슬레이브의 AL_STATUS(0x0130)와 AL_STATUS_CODE(0x0134) 레지스터를 읽어옴
            uint8_t al_status = 0;
            uint16_t al_status_code = 0;
            try {
                kickcat::sendGetRegister(*link_, s.address, 0x0130, al_status);
                kickcat::sendGetRegister(*link_, s.address, 0x0134, al_status_code);
                s.al_status = al_status;
                s.al_status_code = al_status_code;
                std::cout << "[a-master] Read AL_STATUS from slave " << s.address 
                          << ": 0x" << std::hex << static_cast<int>(al_status) 
                          << " code: 0x" << al_status_code << std::dec << "\n";
            } catch (...) {
                // 레지스터 읽기 실패시 기존 값 유지
                al_status = s.al_status;
                al_status_code = s.al_status_code;
                std::cout << "[a-master] Failed to read AL_STATUS from slave " << s.address 
                          << ", using cached: 0x" << std::hex << static_cast<int>(al_status) 
                          << " code: 0x" << al_status_code << std::dec << "\n";
            }
            
            SubsRow r;
            r.address = s.address;
            std::cout << "[a-master] Slave " << s.address << " AL status: 0x"
                      << std::hex << static_cast<int>(al_status)
                      << " code: 0x" << al_status_code << std::dec << "\n";
            switch (static_cast<kickcat::State>(al_status)) {
                case kickcat::State::INIT: r.state = "INIT"; break;
                case kickcat::State::PRE_OP: r.state = "PRE_OP"; break;
                case kickcat::State::BOOT: r.state = "BOOT"; break;
                case kickcat::State::SAFE_OP: r.state = "SAFE_OP"; break;
                case kickcat::State::OPERATIONAL: r.state = "OP"; break;
                default: r.state = "INVALID"; break;
            }
            std::ostringstream oss; oss << "0x" << std::hex << std::uppercase << al_status_code;
            r.al_code = oss.str();
            rows.push_back(std::move(r));
        }
        model_->setSubs(std::move(rows));
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

void MainController::requestOperational()
{
    try {
        std::lock_guard<std::mutex> guard(bus_mutex_);
        ensureBus_();
        if (bus_->slaves().empty()) {
            auto detected = detectSlavesWithRetries_(10, 100ms);
            if (detected <= 0) {
                model_->setStatus("op err: no slave detected");
                return;
            }
        }
        std::vector<uint8_t> iomap(4096, 0);
        bus_->createMapping(iomap.data());
        bus_->requestState(kickcat::State::OPERATIONAL);
        bus_->waitForState(kickcat::State::OPERATIONAL, 500ms, [&]{ bus_->processDataReadWrite([](auto const&){}); });
        // refresh snapshot
        std::vector<SubsRow> rows;
        rows.reserve(bus_->slaves().size());
        for (auto const& s : bus_->slaves()) {
            SubsRow r;
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
        model_->setSubs(std::move(rows));
        model_->setOperational(true);
        model_->setStatus("op ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("op err: ") + e.what());
    }
}

void MainController::run_()
{
    // Removed automatic initial sequence - wait for user to press 's' key
    // This allows slaves to be ready before scanning
    model_->setStatus("ready - press 's' to scan");

    auto period = std::chrono::microseconds(cycle_us_);
    while (!stop_.load()) {
        try {
            std::vector<SubsRow> rows;
            {
                std::lock_guard<std::mutex> guard(bus_mutex_);
                ensureBus_();
                bus_->sendNop([](auto const&){});
                bus_->finalizeDatagrams();
                bus_->processAwaitingFrames();
                // periodic refresh of states (lightweight)
                rows.reserve(bus_->slaves().size());
                for (auto const& s : bus_->slaves()) {
                    SubsRow r;
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
            }
            model_->setSubs(std::move(rows));
        } catch (...) { /* ignore */ }
        std::this_thread::sleep_for(period);
    }
}

bool MainController::sdoUpload(int slave_index, uint16_t index, uint8_t subindex, uint32_t& value)
{
    try {
        std::lock_guard<std::mutex> guard(bus_mutex_);
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

bool MainController::sdoDownload(int slave_index, uint16_t index, uint8_t subindex, uint32_t value)
{
    try {
        std::lock_guard<std::mutex> guard(bus_mutex_);
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
