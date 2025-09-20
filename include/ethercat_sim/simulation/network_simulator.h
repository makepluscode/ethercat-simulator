#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ethercat_sim/communication/ethercat_frame.h"
#include "ethercat_sim/simulation/virtual_slave.h"

namespace ethercat_sim::simulation
{

class NetworkSimulator
{
  public:
    NetworkSimulator() = default;
    void initialize(const std::string& config = "") noexcept;
    int runOnce() noexcept; // returns 0 on success

    // KickCAT-like concept: simple frame I/O to a simulated link
    void setLinkUp(bool up) noexcept;
    void setLatencyMs(std::uint32_t ms) noexcept; // delivery delay simulation (not enforced yet)
    bool isLinkUp() const noexcept
    {
        return linkUp_;
    }

    void setVirtualSlaveCount(std::size_t n) noexcept;
    std::size_t virtualSlaveCount() const noexcept
    {
        return virtualSlaveCount_;
    }
    std::size_t slaveCount() const noexcept
    {
        return virtualSlaveCount_;
    }
    std::size_t onlineSlaveCount() const noexcept;

    // Virtual slave registry (initial skeleton)
    void addVirtualSlave(std::shared_ptr<VirtualSlave> slave) noexcept;
    void clearSlaves() noexcept;

    // Non-blocking send/receive stubs
    bool sendFrame(const communication::EtherCATFrame& frame) noexcept;
    bool receiveFrame(communication::EtherCATFrame& out) noexcept;

    // Addressed register access helpers (for adapter integration/tests)
    bool writeToSlave(std::uint16_t station_address, std::uint16_t reg, const std::uint8_t* data,
                      std::size_t len) noexcept;
    bool readFromSlave(std::uint16_t station_address, std::uint16_t reg, std::uint8_t* out,
                       std::size_t len) const noexcept;

    // Auto-increment physical addressing by slave index (0-based)
    bool writeToSlaveByIndex(std::size_t index, std::uint16_t reg, const std::uint8_t* data,
                             std::size_t len) noexcept;
    bool readFromSlaveByIndex(std::size_t index, std::uint16_t reg, std::uint8_t* out,
                              std::size_t len) const noexcept;

    // Logical memory (LRD/LWR/LRW minimal emulation)
    bool writeLogical(std::uint32_t logical_address, const std::uint8_t* data,
                      std::size_t len) noexcept;
    bool readLogical(std::uint32_t logical_address, std::uint8_t* out,
                     std::size_t len) const noexcept;

    // Minimal PDO mapping helpers for input bitfields (e.g., EL1258):
    // Map a slave's digital input bitfield into logical memory at logical_address.
    // width_bytes is the number of bytes to write (default 1 for 8 DI channels).
    void mapDigitalInputs(const std::shared_ptr<VirtualSlave>& slave, std::uint32_t logical_address,
                          std::size_t width_bytes = 1) noexcept;
    void clearInputMappings() noexcept;

  private:
    struct FrameItem
    {
        communication::EtherCATFrame frame;
        std::chrono::steady_clock::time_point ready_at{};
    };

    bool linkUp_{true};
    std::uint32_t latencyMs_{0};
    std::size_t virtualSlaveCount_{0};
    mutable std::mutex mutex_;
    std::deque<FrameItem> queue_;
    std::vector<std::shared_ptr<VirtualSlave>> slaves_;
    std::vector<std::uint8_t> logical_ = std::vector<std::uint8_t>(16384, 0);

    struct InputMap
    {
        std::weak_ptr<VirtualSlave> slave;
        std::uint32_t logical_address{0};
        std::size_t width_bytes{1};
    };
    std::vector<InputMap> input_maps_;

    // Internal helpers (no locking) to centralize slave lookup
    std::shared_ptr<VirtualSlave> getSlaveByStationAddressNoLock(std::uint16_t addr) const noexcept;
    std::shared_ptr<VirtualSlave> getSlaveByIndexNoLock(std::size_t index) const noexcept;
};

} // namespace ethercat_sim::simulation
