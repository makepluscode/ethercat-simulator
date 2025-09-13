#include "ethercat_sim/simulation/network_simulator.h"

#include <iostream>
#include <chrono>

namespace ethercat_sim::simulation {

void NetworkSimulator::initialize(const std::string& config) noexcept
{
    (void)config; // placeholder until real configuration is used
    std::cout << "[ethercat_sim] NetworkSimulator initialized" << std::endl;
}

int NetworkSimulator::runOnce() noexcept
{
    // Placeholder for a single simulation tick
    std::cout << "[ethercat_sim] NetworkSimulator tick" << std::endl;
    return 0;
}

void NetworkSimulator::setLinkUp(bool up) noexcept
{
    linkUp_ = up;
}

void NetworkSimulator::setLatencyMs(std::uint32_t ms) noexcept
{
    latencyMs_ = ms;
    (void)latencyMs_; // not used yet
}

void NetworkSimulator::setVirtualSlaveCount(std::size_t n) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    // Grow or shrink the registry to match n
    while (slaves_.size() < n) {
        std::uint16_t next_addr = slaves_.empty() ? 1u : static_cast<std::uint16_t>(slaves_.back()->address() + 1u);
        slaves_.push_back(std::make_shared<VirtualSlave>(next_addr, 0, 0, "stub"));
    }
    while (slaves_.size() > n) {
        slaves_.pop_back();
    }
    virtualSlaveCount_ = slaves_.size();
}

void NetworkSimulator::addVirtualSlave(std::shared_ptr<VirtualSlave> slave) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    slaves_.push_back(std::move(slave));
    virtualSlaveCount_ = slaves_.size();
}

void NetworkSimulator::clearSlaves() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    slaves_.clear();
    virtualSlaveCount_ = 0;
}

std::size_t NetworkSimulator::onlineSlaveCount() const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::size_t n = 0;
    for (auto const& s : slaves_) {
        if (s && s->online()) {
            ++n;
        }
    }
    // include any extra count set via setVirtualSlaveCount that is not represented in registry
    if (virtualSlaveCount_ > slaves_.size()) {
        n += (virtualSlaveCount_ - slaves_.size());
    }
    return n;
}

bool NetworkSimulator::sendFrame(const communication::EtherCATFrame& frame) noexcept
{
    if (!linkUp_) {
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    FrameItem item;
    item.frame = frame;
    item.ready_at = now + std::chrono::milliseconds(latencyMs_);
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(std::move(item));
    return true;
}

bool NetworkSimulator::receiveFrame(communication::EtherCATFrame& out) noexcept
{
    if (!linkUp_) {
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return false;
    }
    auto& item = queue_.front();
    if (now < item.ready_at) {
        return false;
    }
    out = item.frame;
    queue_.pop_front();
    return true;
}

bool NetworkSimulator::writeToSlave(std::uint16_t station_address, std::uint16_t reg, const std::uint8_t* data, std::size_t len) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& s : slaves_) {
        if (s && s->online() && s->address() == station_address) {
            return s->write(reg, data, len);
        }
    }
    // If registry does not contain explicit slaves but virtualSlaveCount_ suggests presence,
    // accept write as no-op success for minimal compatibility.
    if (slaves_.empty() && station_address >= 1 && station_address <= virtualSlaveCount_) {
        return true;
    }
    return false;
}

bool NetworkSimulator::readFromSlave(std::uint16_t station_address, std::uint16_t reg, std::uint8_t* out, std::size_t len) const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto const& s : slaves_) {
        if (s && s->online() && s->address() == station_address) {
            return s->read(reg, out, len);
        }
    }
    if (slaves_.empty() && station_address >= 1 && station_address <= virtualSlaveCount_) {
        // Fill zeros if not explicitly modeled
        std::fill(out, out + len, 0u);
        return true;
    }
    return false;
}

bool NetworkSimulator::writeToSlaveByIndex(std::size_t index, std::uint16_t reg, const std::uint8_t* data, std::size_t len) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < slaves_.size()) {
        auto& s = slaves_[index];
        if (s && s->online()) {
            return s->write(reg, data, len);
        }
    }
    return false;
}

bool NetworkSimulator::readFromSlaveByIndex(std::size_t index, std::uint16_t reg, std::uint8_t* out, std::size_t len) const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < slaves_.size()) {
        auto const& s = slaves_[index];
        if (s && s->online()) {
            return s->read(reg, out, len);
        }
    }
    return false;
}

bool NetworkSimulator::writeLogical(std::uint32_t logical_address, const std::uint8_t* data, std::size_t len) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((static_cast<std::size_t>(logical_address) + len) > logical_.size()) {
        return false;
    }
    std::copy(data, data + len, logical_.begin() + logical_address);
    return true;
}

bool NetworkSimulator::readLogical(std::uint32_t logical_address, std::uint8_t* out, std::size_t len) const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((static_cast<std::size_t>(logical_address) + len) > logical_.size()) {
        return false;
    }
    std::copy(logical_.begin() + logical_address, logical_.begin() + logical_address + len, out);
    return true;
}

} // namespace ethercat_sim::simulation
