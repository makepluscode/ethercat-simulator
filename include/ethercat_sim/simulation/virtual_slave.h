#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "kickcat/protocol.h"

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

        // Initialize AL status/state to INIT
        al_state_ = ::kickcat::State::INIT;
        syncCoreRegisters_();
    }

    std::uint16_t address() const noexcept { return address_; }
    std::uint32_t vendorId() const noexcept { return vendor_id_; }
    std::uint32_t productCode() const noexcept { return product_code_; }
    const std::string& name() const noexcept { return name_; }

    bool online() const noexcept { return online_; }
    void setOnline(bool on) noexcept { online_ = on; syncCoreRegisters_(); }

    ::kickcat::State alState() const noexcept { return al_state_; }
    void setALState(::kickcat::State s) noexcept { al_state_ = s; syncCoreRegisters_(); }

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
        // If STATION_ADDR is written, keep internal address in sync
        if (reg == 0x0010 && len >= 2) {
            address_ = static_cast<std::uint16_t>(regs_[0x0010] | (static_cast<uint16_t>(regs_[0x0011]) << 8));
        }
        // If AL_CONTROL written, update AL_STATUS accordingly (minimal behavior)
        if (reg == ::kickcat::reg::AL_CONTROL && len >= 1) {
            al_state_ = static_cast<::kickcat::State>(regs_[::kickcat::reg::AL_CONTROL]);
            // No error: AL_STATUS_CODE = 0
            al_status_code_ = 0;
            syncCoreRegisters_();
        }
        return true;
    }

private:
    void syncCoreRegisters_() noexcept
    {
        // DL_STATUS (0x110): set basic communication flags when online
        uint16_t dl = 0;
        if (online_) {
            // PDI_op (bit0) and COM_port0(bit9), PL_port0(bit4)
            dl |= (1u << 0); // PDI_op
            dl |= (1u << 4); // PL_port0
            dl |= (1u << 9); // COM_port0
        }
        regs_.at(::kickcat::reg::ESC_DL_STATUS + 0) = static_cast<uint8_t>(dl & 0xFF);
        regs_.at(::kickcat::reg::ESC_DL_STATUS + 1) = static_cast<uint8_t>((dl >> 8) & 0xFF);

        // AL_STATUS (0x130): current state in low byte
        regs_.at(::kickcat::reg::AL_STATUS + 0) = static_cast<uint8_t>(al_state_);
        regs_.at(::kickcat::reg::AL_STATUS + 1) = 0u;

        // AL_STATUS_CODE (0x134): 0 => no error
        regs_.at(::kickcat::reg::AL_STATUS_CODE + 0) = static_cast<uint8_t>(al_status_code_ & 0xFF);
        regs_.at(::kickcat::reg::AL_STATUS_CODE + 1) = static_cast<uint8_t>((al_status_code_ >> 8) & 0xFF);
    }

    std::uint16_t address_ {};
    std::uint32_t vendor_id_ {};
    std::uint32_t product_code_ {};
    std::string name_;
    bool online_ {true};
    ::kickcat::State al_state_ {::kickcat::State::INIT};
    uint16_t al_status_code_ {0};
    std::vector<std::uint8_t> regs_ = std::vector<std::uint8_t>(2048, 0);
};

} // namespace ethercat_sim::simulation
