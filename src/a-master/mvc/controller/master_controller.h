#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>

#include "mvc/model/master_model.h"

namespace kickcat { class Bus; class Link; class SocketNull; }
namespace ethercat_sim { namespace bus { class MasterSocket; } }

namespace ethercat_sim::app::master {

class MasterController {
public:
    MasterController(std::string endpoint, int cycle_us);
    ~MasterController();

    void start();
    void stop();

    void scan();
    void initPreop();
    void requestOperational();

    std::shared_ptr<MasterModel> model() { return model_; }

private:
    void run_();
    void ensureBus_();

    std::string endpoint_;
    int cycle_us_ {1000};
    std::shared_ptr<MasterModel> model_ {std::make_shared<MasterModel>()};
    std::shared_ptr<bus::MasterSocket> sock_;
    std::shared_ptr<kickcat::SocketNull> nullRed_;
    std::shared_ptr<kickcat::Link> link_;
    std::unique_ptr<kickcat::Bus> bus_;

    std::thread th_;
    std::atomic_bool stop_{false};
};

} // namespace ethercat_sim::app::master
