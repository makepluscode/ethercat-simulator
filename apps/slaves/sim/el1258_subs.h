#pragma once

#include <array>
#include <cstdint>

#include "ethercat_sim/simulation/virtual_slave.h"

namespace ethercat_sim::subs
{

class El1258Slave : public ethercat_sim::simulation::VirtualSlave
{
  public:
    explicit El1258Slave(uint16_t station_addr, uint32_t vendor_id = 0x00000000,
                         uint32_t product_code = 0x00000000)
        : VirtualSlave(station_addr, vendor_id, product_code, "EL1258")
    {
        // Apply default PDO mapping to enable SAFE_OP and OPERATIONAL states
        applyDefaultTxPdoMapping();
    }

    // Apply a sensible default TxPDO mapping for 8 DI channels:
    // - 0x1A00:01 = 0x6002:00 (8 bits aggregate)
    // - 0x1C13:01 = 0x1A00 assignment
    // This marks inputs as PDO-mapped for AL state gating purposes.
    void applyDefaultTxPdoMapping() noexcept
    {
        map_count_    = 1;
        mappings_[0]  = (0x6002u << 16) | (0x00u << 8) | 8u;
        assign_count_ = 1;
        assigned_     = true;
        setInputPDOMapped(true);
    }

    bool readDigitalInputsBitfield(uint32_t& bits_out) const noexcept override
    {
        uint32_t raw = 0;
        for (int i = 0; i < 8; ++i)
        {
            raw |= (channels_[i] ? (1u << i) : 0u);
        }
        uint32_t v = raw ^ invert_mask_;
        bits_out   = v;
        return true;
    }

  protected:
    bool onSdoUpload(uint16_t index, uint8_t subindex, uint32_t& value) const noexcept override
    {
        switch (index)
        {
        case 0x1000:
            value = 0x00000000;
            return true; // device type (placeholder)
        case 0x1008:
            value = 0;
            return true; // name (expedited 0)
        case 0x1009:
            value = 0;
            return true; // HW version
        case 0x100A:
            value = 0;
            return true; // SW version
        case 0x6002:     // aggregate inputs
            value = currentAggregate_();
            return true;
        case 0x6000: // per-channel
            if (subindex >= 1 && subindex <= 8)
            {
                value = (currentAggregate_() >> (subindex - 1)) & 0x1u;
                return true;
            }
            break;
        case 0x8000: // invert mask
            value = invert_mask_;
            return true;
        case 0x8001: // debounce ms
            value = debounce_ms_;
            return true;
        default:
            break;
        }
        return false;
    }

    bool onSdoDownload(uint16_t index, uint8_t subindex, uint32_t value,
                       uint8_t nbytes) noexcept override
    {
        (void) nbytes;
        switch (index)
        {
        case 0x8000: // invert mask
            invert_mask_ = (value & 0xFFu);
            return true;
        case 0x8001: // debounce
            debounce_ms_ = value;
            return true;
        case 0x1A00: // TxPDO mapping (Sub0=count, others=mappings)
            if (subindex == 0)
            {
                map_count_ = static_cast<uint8_t>(value & 0xFF);
                return true;
            }
            if (subindex >= 1 && subindex <= 8)
            {
                mappings_[subindex - 1] = value;
                return true;
            }
            break;
        case 0x1C13: // assign (Sub0=count, Sub1=0x1A00)
            if (subindex == 0)
            {
                assign_count_ = static_cast<uint8_t>(value & 0xFF);
                return true;
            }
            if (subindex == 1)
            {
                assigned_ = (value & 0xFFFFu) == 0x1A00u;
                // Mark input PDO mapped if both mapping count and assign are set
                bool mapped = assigned_ && (map_count_ > 0);
                setInputPDOMapped(mapped);
                return true;
            }
            break;
        default:
            break;
        }
        return false;
    }

  private:
    uint32_t currentAggregate_() const noexcept
    {
        uint32_t raw = 0;
        for (int i = 0; i < 8; ++i)
            raw |= (channels_[i] ? (1u << i) : 0u);
        return (raw ^ invert_mask_) & 0xFFu;
    }

    std::array<bool, 8> channels_{}; // future: TUI에서 토글
    uint32_t invert_mask_{0};
    uint32_t debounce_ms_{0};

    uint8_t map_count_{0};
    std::array<uint32_t, 8> mappings_{};
    uint8_t assign_count_{0};
    bool assigned_{false};
};

} // namespace ethercat_sim::subs
