#include "mvc/controller/master_controller.h"

#include "bus/master_socket.h"
#include "kickcat/Bus.h"
#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"

#include <chrono>
#include <iostream>

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
    sock_->setTimeout(5ms);
    nullRed_ = std::make_shared<kickcat::SocketNull>();
    link_ = std::make_shared<kickcat::Link>(sock_, nullRed_, []{});
    link_->setTimeout(5ms);
    bus_ = std::make_unique<kickcat::Bus>(link_);
}

void MasterController::scan()
{
    try {
        ensureBus_();
        int32_t n = bus_->detectSlaves();
        model_->setDetectedSlaves(n);
        model_->setStatus("scan ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("scan err: ") + e.what());
    }
}

void MasterController::initPreop()
{
    try {
        ensureBus_();
        bus_->init();
        model_->setPreop(true);
        model_->setStatus("preop ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("preop err: ") + e.what());
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
        model_->setOperational(true);
        model_->setStatus("op ok");
    } catch (std::exception const& e) {
        model_->setStatus(std::string("op err: ") + e.what());
    }
}

void MasterController::run_()
{
    // Initial sequence: scan -> preop -> attempt op
    scan();
    initPreop();
    requestOperational();

    auto period = std::chrono::microseconds(cycle_us_);
    while (!stop_.load()) {
        try {
            ensureBus_();
            bus_->sendNop([](auto const&){});
            bus_->finalizeDatagrams();
            bus_->processAwaitingFrames();
        } catch (...) { /* ignore */ }
        std::this_thread::sleep_for(period);
    }
}

} // namespace ethercat_sim::app::master
