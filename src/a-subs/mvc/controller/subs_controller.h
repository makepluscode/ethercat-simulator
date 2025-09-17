#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>

#include "mvc/model/subs_model.h"
#include "bus/subs_endpoint.h"

namespace ethercat_sim::app::subs {

class SubsController {
public:
    SubsController(std::string endpoint, int count)
        : endpoint_(std::move(endpoint)), count_(count) {}
    ~SubsController() { stop(); }

    void start();
    void stop();

    std::shared_ptr<SubsModel> model() { return model_; }

private:
    void run_();

    std::string endpoint_;
    int count_ {1};
    std::shared_ptr<SubsModel> model_ {std::make_shared<SubsModel>()};
    std::atomic_bool stop_{false};
    std::thread th_;
};

} // namespace ethercat_sim::app::subs
