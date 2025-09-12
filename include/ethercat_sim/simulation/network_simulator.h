#pragma once

#include <string>
#include <cstdint>

#include "ethercat_sim/communication/ethercat_frame.h"

namespace ethercat_sim::simulation {

class NetworkSimulator {
public:
    NetworkSimulator() = default;
    void initialize(const std::string& config = "") noexcept;
    int runOnce() noexcept; // returns 0 on success

    // KickCAT-like concept: simple frame I/O to a simulated link
    void setLinkUp(bool up) noexcept;
    void setLatencyMs(std::uint32_t ms) noexcept; // delivery delay simulation (not enforced yet)
    bool isLinkUp() const noexcept { return linkUp_; }

    void setVirtualSlaveCount(std::size_t n) noexcept { virtualSlaveCount_ = n; }
    std::size_t virtualSlaveCount() const noexcept { return virtualSlaveCount_; }

    // Non-blocking send/receive stubs
    bool sendFrame(const communication::EtherCATFrame& frame) noexcept;
    bool receiveFrame(communication::EtherCATFrame& out) noexcept;

private:
    bool linkUp_ {true};
    std::uint32_t latencyMs_ {0};
    std::size_t virtualSlaveCount_ {0};
};

} // namespace ethercat_sim::simulation
