#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ethercat_sim::simulation {

class VirtualSlave {
public:
    VirtualSlave(std::uint16_t address = 0,
                 std::uint32_t vendor_id = 0,
                 std::uint32_t product_code = 0,
                 std::string name = {})
        : address_(address)
        , vendor_id_(vendor_id)
        , product_code_(product_code)
        , name_(std::move(name))
    {
        // initialize a minimal ESC register map
        // Set STATION_ADDR (0x0010) with the configured address (LE)
        regs_.at(0x0010) = static_cast<uint8_t>(address_ & 0xFF);
        regs_.at(0x0011) = static_cast<uint8_t>((address_ >> 8) & 0xFF);
    }

    std::uint16_t address() const noexcept { return address_; }
    std::uint32_t vendorId() const noexcept { return vendor_id_; }
    std::uint32_t productCode() const noexcept { return product_code_; }
    const std::string& name() const noexcept { return name_; }

    bool online() const noexcept { return online_; }
    void setOnline(bool on) noexcept { online_ = on; }

    // Minimal register map (byte-addressable)
    bool read(std::uint16_t reg, std::uint8_t* dst, std::size_t len) const noexcept
    {
        if ((static_cast<std::size_t>(reg) + len) > regs_.size()) {
            return false;
        }
        std::copy(regs_.begin() + reg, regs_.begin() + reg + len, dst);
        return true;
    }

    bool write(std::uint16_t reg, std::uint8_t const* src, std::size_t len) noexcept
    {
        if ((static_cast<std::size_t>(reg) + len) > regs_.size()) {
            return false;
        }
        std::copy(src, src + len, regs_.begin() + reg);
        return true;
    }

private:
    std::uint16_t address_ {};
    std::uint32_t vendor_id_ {};
    std::uint32_t product_code_ {};
    std::string name_;
    bool online_ {true};
    std::vector<std::uint8_t> regs_ = std::vector<std::uint8_t>(2048, 0);
};

} // namespace ethercat_sim::simulation
