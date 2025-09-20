#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "master_model.h"

namespace kickcat
{
class Bus;
class Link;
enum State : std::uint8_t;
} // namespace kickcat
namespace ethercat_sim
{
namespace bus
{
class MasterSocket;
}
} // namespace ethercat_sim

namespace ethercat_sim::app::master
{

class MasterController
{
  public:
    MasterController(std::string endpoint, int cycle_us);
    ~MasterController();

    void start();
    void stop();

    void scan();
    void initPreop();
    void requestOperational();
    // SDO operations (expedited)
    bool sdoUpload(int slave_index, uint16_t index, uint8_t subindex, uint32_t& value);
    bool sdoDownload(int slave_index, uint16_t index, uint8_t subindex, uint32_t value);

    std::shared_ptr<MasterModel> model()
    {
        return model_;
    }

  private:
    void run_();
    void ensureBus_();
    int32_t detectSlavesWithRetries_(int attempts, std::chrono::milliseconds delay);
    void refreshSlaveAlStatusUnlocked_();
    std::vector<SlavesRow> snapshotSlavesUnlocked_();
    bool allSlavesInStateUnlocked_(kickcat::State state) const;
    bool requestAndWaitStateUnlocked_(kickcat::State target, std::chrono::milliseconds timeout);

    std::string endpoint_;
    int cycle_us_{1000};
    std::shared_ptr<MasterModel> model_{std::make_shared<MasterModel>()};
    std::shared_ptr<bus::MasterSocket> sock_;
    std::shared_ptr<kickcat::Link> link_;
    std::unique_ptr<kickcat::Bus> bus_;

    std::thread th_;
    std::atomic_bool stop_{false};
    std::mutex bus_mutex_;
};

} // namespace ethercat_sim::app::master
