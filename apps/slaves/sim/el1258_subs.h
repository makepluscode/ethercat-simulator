#pragma once

#include <array>
#include <cstdint>
#include <chrono>

#include "ethercat_sim/simulation/virtual_slave.h"
#include "framework/logger/logger.h"

namespace ethercat_sim::subs
{

class El1258Slave : public ethercat_sim::simulation::VirtualSlave
{
  public:
    explicit El1258Slave(uint16_t station_addr, uint32_t vendor_id = 0x00000002,
                         uint32_t product_code = 0x04ea3052)
        : VirtualSlave(station_addr, vendor_id, product_code, "EL1258")
    {
        LOG_DEBUG("El1258Slave[" + std::to_string(station_addr) + "] constructor: vendor=0x" + 
                  std::to_string(vendor_id) + ", product=0x" + std::to_string(product_code));
        // Apply default PDO mapping to enable SAFE_OP and OPERATIONAL states
        applyDefaultTxPdoMapping();
        LOG_DEBUG("El1258Slave[" + std::to_string(station_addr) + "] constructor complete");
    }

    // Apply multi-timestamping TxPDO mapping for 8 DI channels:
    // - 0x1A00-0x1A1C: Individual channel timestamping PDOs
    // - 0x1C13: PDO assignment
    // This implements EL1258's multi-timestamping capability.
    void applyDefaultTxPdoMapping() noexcept
    {
        LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] applying multi-timestamping TxPDO mapping");
        
        // Set up 8 individual channel PDO mappings (0x1A00-0x1A1C)
        map_count_ = 8;
        for (int i = 0; i < 8; ++i)
        {
            // Each channel gets its own PDO mapping
            // Format: (Object Index << 16) | (Subindex << 8) | (Bit Length)
            mappings_[i] = (0x6000u << 16) | ((i + 1) << 8) | 1u; // 1 bit per channel
        }
        
        assign_count_ = 1;
        assigned_     = true;
        setInputPDOMapped(true);
        
        LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] Multi-timestamping TxPDO mapping applied: map_count=" + 
                  std::to_string(map_count_) + ", assigned=" + (assigned_ ? "true" : "false") + 
                  ", input_pdo_mapped=" + (isInputPDOMapped() ? "true" : "false"));
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

    // Set individual channel state (for testing/simulation)
    void setChannelState(int channel, bool state) noexcept
    {
        if (channel >= 1 && channel <= 8)
        {
            // Apply 3ms input filter simulation
            auto now = std::chrono::steady_clock::now();
            auto& filter = channel_filters_[channel - 1];
            
            // If state changed, start debounce timer
            if (filter.raw_state != state)
            {
                filter.raw_state = state;
                filter.debounce_start = now;
                filter.debounce_active = true;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] Channel " + std::to_string(channel) + " raw state changed to " + (state ? "HIGH" : "LOW") + ", starting 3ms debounce");
            }
            
            // Check if debounce period has elapsed
            if (filter.debounce_active)
            {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - filter.debounce_start);
                if (elapsed.count() >= 3) // 3ms filter
                {
                    filter.debounce_active = false;
                    channels_[channel - 1] = filter.raw_state;
                    LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] Channel " + std::to_string(channel) + " filtered state set to " + (filter.raw_state ? "HIGH" : "LOW"));
                }
            }
        }
    }

    // Get individual channel state
    bool getChannelState(int channel) const noexcept
    {
        if (channel >= 1 && channel <= 8)
        {
            return channels_[channel - 1];
        }
        return false;
    }

  protected:
    bool onSdoUpload(uint16_t index, uint8_t subindex, uint32_t& value) const noexcept override
    {
        LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload: 0x" + 
                  std::to_string(index) + ":" + std::to_string(subindex));
        switch (index)
        {
        case 0x1000: // Device type
            value = 0x00000000; // Digital input terminal type
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1000: device type = 0x" + std::to_string(value));
            return true;
        case 0x1008: // Device name
            // Note: This would typically return a string, but for simplicity we return 0
            value = 0;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1008: device name = 0x" + std::to_string(value));
            return true;
        case 0x1009: // Hardware version
            value = 0x00000001; // Hardware version 1
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1009: HW version = 0x" + std::to_string(value));
            return true;
        case 0x100A: // Software version
            value = 0x00000001; // Software version 1
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x100A: SW version = 0x" + std::to_string(value));
            return true;
        case 0x1018: // Identity object
            if (subindex == 0)
            {
                value = 4; // Number of identity entries
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1018:0: number of identity entries = " + std::to_string(value));
                return true;
            }
            else if (subindex == 1)
            {
                value = 0x00000002; // Beckhoff vendor ID
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1018:1: vendor ID = 0x" + std::to_string(value));
                return true;
            }
            else if (subindex == 2)
            {
                value = 0x04ea3052; // EL1258 product code
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1018:2: product code = 0x" + std::to_string(value));
                return true;
            }
            else if (subindex == 3)
            {
                value = 0x00110000; // Revision number
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1018:3: revision = 0x" + std::to_string(value));
                return true;
            }
            else if (subindex == 4)
            {
                value = 0x00000000; // Serial number (placeholder)
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x1018:4: serial number = 0x" + std::to_string(value));
                return true;
            }
            break;
        case 0x6000: // Digital inputs (per-channel)
            if (subindex == 0)
            {
                value = 8; // Number of channels
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x6000:0: number of channels = " + std::to_string(value));
                return true;
            }
            if (subindex >= 1 && subindex <= 8)
            {
                value = channels_[subindex - 1] ? 1 : 0;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x6000:" + std::to_string(subindex) + ": channel " + std::to_string(subindex) + " = " + std::to_string(value));
                return true;
            }
            break;
        case 0x6001: // Digital inputs status
            if (subindex == 0)
            {
                value = 8; // Number of status entries
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x6001:0: number of status entries = " + std::to_string(value));
                return true;
            }
            if (subindex >= 1 && subindex <= 8)
            {
                // Status: bit 0 = input state, bit 1 = error, bit 2 = overrange, bit 3 = underrange
                value = (channels_[subindex - 1] ? 0x01 : 0x00);
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x6001:" + std::to_string(subindex) + ": channel " + std::to_string(subindex) + " status = 0x" + std::to_string(value));
                return true;
            }
            break;
        case 0x6002: // Digital inputs (aggregate)
            value = currentAggregate_();
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x6002: aggregate inputs = 0x" + std::to_string(value));
            return true;
        case 0x8000: // invert mask
            value = invert_mask_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x8000: invert mask = 0x" + std::to_string(value));
            return true;
        case 0x8001: // debounce ms
            value = debounce_ms_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x8001: debounce ms = 0x" + std::to_string(value));
            return true;
        case 0x8002: // Error register
            value = error_register_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x8002: error register = 0x" + std::to_string(value));
            return true;
        case 0x8003: // Manufacturer status register
            value = manufacturer_status_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x8003: manufacturer status = 0x" + std::to_string(value));
            return true;
        case 0x8004: // Pre-defined error field
            if (subindex == 0)
            {
                value = error_count_;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x8004:0: error count = " + std::to_string(value));
                return true;
            }
            break;
        default:
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x" + std::to_string(index) + ":" + std::to_string(subindex) + ": unknown index");
            break;
        }
        return false;
    }

    bool onSdoDownload(uint16_t index, uint8_t subindex, uint32_t value,
                       uint8_t nbytes) noexcept override
    {
        (void) nbytes;
        LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download: 0x" + 
                  std::to_string(index) + ":" + std::to_string(subindex) + " = 0x" + 
                  std::to_string(value) + " (" + std::to_string(nbytes) + " bytes)");
        switch (index)
        {
        case 0x8000: // invert mask
            invert_mask_ = (value & 0xFFu);
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x8000: invert mask = 0x" + std::to_string(invert_mask_));
            return true;
        case 0x8001: // debounce
            debounce_ms_ = value;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x8001: debounce ms = " + std::to_string(debounce_ms_));
            return true;
        case 0x1A00: // TxPDO mapping for channel 1 (Sub0=count, others=mappings)
        case 0x1A04: // TxPDO mapping for channel 2
        case 0x1A08: // TxPDO mapping for channel 3
        case 0x1A0C: // TxPDO mapping for channel 4
        case 0x1A10: // TxPDO mapping for channel 5
        case 0x1A14: // TxPDO mapping for channel 6
        case 0x1A18: // TxPDO mapping for channel 7
        case 0x1A1C: // TxPDO mapping for channel 8
            if (subindex == 0)
            {
                // Calculate channel number from index
                int channel = ((index - 0x1A00) / 4) + 1;
                if (channel >= 1 && channel <= 8)
                {
                    map_count_ = static_cast<uint8_t>(value & 0xFF);
                    LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x" + std::to_string(index) + ":0: channel " + std::to_string(channel) + " map count = " + std::to_string(map_count_));
                }
                return true;
            }
            if (subindex >= 1 && subindex <= 8)
            {
                // Calculate channel number from index
                int channel = ((index - 0x1A00) / 4) + 1;
                if (channel >= 1 && channel <= 8)
                {
                    mappings_[channel - 1] = value;
                    LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x" + std::to_string(index) + ":" + std::to_string(subindex) + ": channel " + std::to_string(channel) + " mapping = 0x" + std::to_string(value));
                }
                return true;
            }
            break;
        case 0x1C13: // PDO assignment (Sub0=count, Sub1=0x1A00)
            if (subindex == 0)
            {
                assign_count_ = static_cast<uint8_t>(value & 0xFF);
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x1C13:0: PDO assign count = " + std::to_string(assign_count_));
                return true;
            }
            if (subindex == 1)
            {
                // PDO assignment: 0x1A00 means TxPDO 1 is assigned
                assigned_ = (value & 0xFFFFu) == 0x1A00u;
                // Mark input PDO mapped if both mapping count and assign are set
                bool mapped = assigned_ && (map_count_ > 0);
                setInputPDOMapped(mapped);
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x1C13:1: PDO assigned=" + (assigned_ ? "true" : "false") + 
                          " (value=0x" + std::to_string(value) + "), mapped=" + (mapped ? "true" : "false") + 
                          ", input_pdo_mapped=" + (isInputPDOMapped() ? "true" : "false"));
                return true;
            }
            break;
        default:
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x" + std::to_string(index) + ":" + std::to_string(subindex) + ": unknown index");
            break;
        }
        return false;
    }

  private:
    // Input filter structure for 3ms debouncing
    struct ChannelFilter
    {
        bool raw_state{false};           // Raw input state
        bool debounce_active{false};     // Debounce timer active
        std::chrono::steady_clock::time_point debounce_start; // Debounce start time
    };

    uint32_t currentAggregate_() const noexcept
    {
        uint32_t raw = 0;
        for (int i = 0; i < 8; ++i)
            raw |= (channels_[i] ? (1u << i) : 0u);
        return (raw ^ invert_mask_) & 0xFFu;
    }

    std::array<bool, 8> channels_{}; // Filtered channel states
    std::array<ChannelFilter, 8> channel_filters_{}; // Input filters for each channel
    uint32_t invert_mask_{0};
    uint32_t debounce_ms_{0};

    // Status and error handling
    uint32_t error_register_{0};        // Error register (0x8002)
    uint32_t manufacturer_status_{0};   // Manufacturer status register (0x8003)
    uint32_t error_count_{0};           // Error count (0x8004:0)

    uint8_t map_count_{0};
    std::array<uint32_t, 8> mappings_{};
    uint8_t assign_count_{0};
    bool assigned_{false};
};

} // namespace ethercat_sim::subs
