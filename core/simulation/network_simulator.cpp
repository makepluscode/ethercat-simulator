#include "ethercat_sim/simulation/network_simulator.h"

#include <deque>
#include <iostream>

namespace {
// very small local queue for frames to simulate loopback
static std::deque<ethercat_sim::communication::EtherCATFrame> g_queue;
}

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

bool NetworkSimulator::sendFrame(const communication::EtherCATFrame& frame) noexcept
{
    if (!linkUp_) {
        return false;
    }
    g_queue.push_back(frame); // loopback enqueue
    return true;
}

bool NetworkSimulator::receiveFrame(communication::EtherCATFrame& out) noexcept
{
    if (!linkUp_ || g_queue.empty()) {
        return false;
    }
    out = g_queue.front();
    g_queue.pop_front();
    return true;
}

} // namespace ethercat_sim::simulation
