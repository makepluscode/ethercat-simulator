#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "kickcat/AbstractSocket.h"

#include "ethercat_sim/simulation/network_simulator.h"

namespace ethercat_sim::kickcat
{

class SimSocket : public ::kickcat::AbstractSocket
{
  public:
    explicit SimSocket(std::shared_ptr<simulation::NetworkSimulator> sim) : sim_(std::move(sim)) {}

    void open(std::string const& interface) override;
    void setTimeout(std::chrono::nanoseconds timeout) override;
    void close() noexcept override;
    int32_t read(uint8_t* frame, int32_t frame_size) override;
    int32_t write(uint8_t const* frame, int32_t frame_size) override;

  private:
    std::shared_ptr<simulation::NetworkSimulator> sim_;
    std::chrono::nanoseconds timeout_{std::chrono::milliseconds(2)};
};

} // namespace ethercat_sim::kickcat

// Backward-compatibility alias: ethercat_sim::kickcat_adapter -> ethercat_sim::kickcat
namespace ethercat_sim
{
namespace kickcat_adapter = kickcat;
}
