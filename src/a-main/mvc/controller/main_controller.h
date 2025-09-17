#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#include "mvc/model/main_model.h"

namespace kickcat { class Bus; class Link; }
namespace ethercat_sim { namespace bus { class MainSocket; } }

namespace ethercat_sim::app::main {

class MainController {
public:
    MainController(std::string endpoint, int cycle_us);
    ~MainController();

    void start();
    void stop();

    void scan();
    void initPreop();
    void requestOperational();
    // SDO operations (expedited)
    bool sdoUpload(int slave_index, uint16_t index, uint8_t subindex, uint32_t& value);
    bool sdoDownload(int slave_index, uint16_t index, uint8_t subindex, uint32_t value);

    std::shared_ptr<MainModel> model() { return model_; }

private:
    void run_();
    void ensureBus_();
    int32_t detectSlavesWithRetries_(int attempts, std::chrono::milliseconds delay);

    std::string endpoint_;
    int cycle_us_ {1000};
    std::shared_ptr<MainModel> model_ {std::make_shared<MainModel>()};
    std::shared_ptr<bus::MainSocket> sock_;
    std::shared_ptr<kickcat::Link> link_;
    std::unique_ptr<kickcat::Bus> bus_;

    std::thread th_;
    std::atomic_bool stop_{false};
    std::mutex bus_mutex_;
};

} // namespace ethercat_sim::app::main
