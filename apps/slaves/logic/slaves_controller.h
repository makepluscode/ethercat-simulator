#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "bus/slaves_endpoint.h"
#include "slaves_model.h"

namespace ethercat_sim::app::slaves
{

class SlavesController
{
  public:
    SlavesController(std::string endpoint, int count)
        : endpoint_(std::move(endpoint)), count_(count)
    {
    }
    ~SlavesController()
    {
        stop();
    }

    void start();
    void stop();

    std::shared_ptr<SlavesModel> model()
    {
        return model_;
    }

  private:
    void run_();

    std::string endpoint_;
    int count_{1};
    std::shared_ptr<SlavesModel> model_{std::make_shared<SlavesModel>()};
    std::atomic_bool stop_{false};
    std::thread th_;
};

} // namespace ethercat_sim::app::slaves
