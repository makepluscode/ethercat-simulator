#pragma once

#include <cstdint>
#include <vector>

namespace ethercat_sim::communication {

// Minimal EtherCAT frame placeholder inspired by KickCAT usage patterns
struct EtherCATFrame {
    std::vector<std::uint8_t> payload;
};

} // namespace ethercat_sim::communication

