#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>

#include "mvc/model/slave_model.h"
#include "bus/slave_endpoint.h"

namespace ethercat_sim::app::slaves {

class SlaveController {
public:
    SlaveController(std::string endpoint, int count)
        : endpoint_(std::move(endpoint)), count_(count) {}
    ~SlaveController() { stop(); }

    void start();
    void stop();

    std::shared_ptr<SlaveModel> model() { return model_; }

private:
    void run_();

    std::string endpoint_;
    int count_ {1};
    std::shared_ptr<SlaveModel> model_ {std::make_shared<SlaveModel>()};
    std::atomic_bool stop_{false};
    std::thread th_;
};

} // namespace ethercat_sim::app::slaves
