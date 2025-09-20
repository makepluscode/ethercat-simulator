#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ethercat_sim/simulation/network_simulator.h"

namespace ethercat_sim::bus
{

class SlavesEndpoint
{
  public:
    explicit SlavesEndpoint(std::string endpoint) : endpoint_(std::move(endpoint)) {}

    void setSlavesCount(std::size_t n)
    {
        slaves_count_ = n;
    }
    void setStopFlag(std::atomic_bool* stop_flag)
    {
        stop_ = stop_flag;
    }
    bool run(); // blocking server loop (single connection for now)

  private:
    std::string endpoint_;
    std::size_t slaves_count_{1};
    std::atomic_bool* stop_{nullptr};

    std::shared_ptr<simulation::NetworkSimulator> sim_{
        std::make_shared<simulation::NetworkSimulator>()};

    int listen_fd_{-1};
    bool uds_is_abstract_{false};
    std::string bound_disk_path_;
    bool bindUDS_(const std::string& path);
    bool bindTCP_(const std::string& host, uint16_t port);
    bool handleClient_(int fd);
    void processFrame_(uint8_t* buf, int32_t len);
    std::function<void(bool)> on_connection_;

  public:
    void setConnectionCallback(std::function<void(bool)> cb)
    {
        on_connection_ = std::move(cb);
    }
};

} // namespace ethercat_sim::bus
