#include "mvc/controller/slave_controller.h"

#include <iostream>

namespace ethercat_sim::app::slaves {

void SlaveController::start()
{
    if (th_.joinable()) return;
    stop_.store(false);
    th_ = std::thread(&SlaveController::run_, this);
}

void SlaveController::stop()
{
    stop_.store(true);
    if (th_.joinable()) th_.join();
}

void SlaveController::run_()
{
    model_->setEndpoint(endpoint_);
    model_->setCount(count_);
    model_->setStatus("starting");

    ethercat_sim::bus::SlaveEndpoint ep(endpoint_);
    std::atomic_bool stop_flag{false};
    ep.setStopFlag(&stop_flag);
    ep.setSlaveCount(static_cast<std::size_t>(count_));
    ep.setConnectionCallback([this](bool connected){ this->model_->setConnected(connected); });

    model_->setListening(true);
    // Light integration: the endpoint returns only when stopping; we simulate connected status changes in endpoint in future.
    // Here we just run it until stop_ is set, then signal stop to endpoint.
    std::thread server([&]{ ep.run(); });

    model_->setStatus("listening");
    while (!stop_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    stop_flag.store(true);
    if (server.joinable()) server.join();
    model_->setListening(false);
    model_->setStatus("stopped");
}

} // namespace ethercat_sim::app::slaves
