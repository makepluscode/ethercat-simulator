#pragma once

#include <array>
#include <chrono>
#include <cstdint>

#include "ethercat_sim/simulation/slaves/el1258_constants.h"
#include "ethercat_sim/simulation/virtual_slave.h"
#include "framework/logger/logger.h"

namespace ethercat_sim::subs
{
namespace el1258 = ethercat_sim::simulation::slaves::el1258;

class El1258Slave : public ethercat_sim::simulation::VirtualSlave
{
  public:
    explicit El1258Slave(uint16_t station_addr, uint32_t vendor_id = el1258::VENDOR_ID,
                         uint32_t product_code = el1258::PRODUCT_CODE)
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
        LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                  "] applying multi-timestamping TxPDO mapping");

        // Set up 8 individual channel PDO mappings (0x1A00-0x1A1C)
        map_count_ = el1258::CHANNEL_COUNT_U8;
        for (std::size_t i = 0; i < el1258::CHANNEL_COUNT; ++i)
        {
            // Each channel gets its own PDO mapping
            // Format: (Object Index << 16) | (Subindex << 8) | (Bit Length)
            mappings_[i] = el1258::makeChannelMapping(static_cast<uint8_t>(i + 1));
        }

        assign_count_ = 1;
        assigned_     = true;
        setInputPDOMapped(true);

        LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                  "] Multi-timestamping TxPDO mapping applied: map_count=" +
                  std::to_string(map_count_) + ", assigned=" + (assigned_ ? "true" : "false") +
                  ", input_pdo_mapped=" + (isInputPDOMapped() ? "true" : "false"));
    }

    bool readDigitalInputsBitfield(uint32_t& bits_out) const noexcept override
    {
        bits_out = currentAggregate_();
        return true;
    }

    // Set individual channel state (for testing/simulation)
    void setChannelState(int channel, bool state) noexcept
    {
        if (channel >= 1 && channel <= el1258::CHANNEL_MAX)
        {
            // Apply 3ms input filter simulation
            auto now     = std::chrono::steady_clock::now();
            auto& filter = channel_filters_[channel - 1];

            // If state changed, start debounce timer
            if (filter.raw_state != state)
            {
                filter.raw_state       = state;
                filter.debounce_start  = now;
                filter.debounce_active = true;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] Channel " +
                          std::to_string(channel) + " raw state changed to " +
                          (state ? "HIGH" : "LOW") + ", starting " +
                          std::to_string(el1258::DEBOUNCE_FILTER_MS) + "ms debounce");
            }

            // Check if debounce period has elapsed
            if (filter.debounce_active)
            {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - filter.debounce_start);
                if (elapsed.count() >= el1258::DEBOUNCE_FILTER_MS)
                {
                    filter.debounce_active = false;
                    channels_[channel - 1] = filter.raw_state;
                    LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] Channel " +
                              std::to_string(channel) + " filtered state set to " +
                              (filter.raw_state ? "HIGH" : "LOW"));
                }
            }
        }
    }

    // Get individual channel state
    bool getChannelState(int channel) const noexcept
    {
        if (channel >= 1 && channel <= el1258::CHANNEL_MAX)
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
        case el1258::OBJ_DEVICE_TYPE:
            value = el1258::DEVICE_TYPE_CODE;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x1000: device type = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_DEVICE_NAME:
            // Note: This would typically return a string, but for simplicity we return 0
            value = 0;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x1008: device name = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_HARDWARE_VERSION:
            value = el1258::HARDWARE_VERSION_VALUE;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x1009: HW version = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_SOFTWARE_VERSION:
            value = el1258::SOFTWARE_VERSION_VALUE;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x100A: SW version = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_IDENTITY:
            if (subindex == 0)
            {
                value = el1258::IDENTITY_ENTRY_COUNT;
                LOG_DEBUG(
                    "El1258Slave[" + std::to_string(address()) +
                    "] SDO Upload 0x1018:0: number of identity entries = " + std::to_string(value));
                return true;
            }
            else if (subindex == 1)
            {
                value = el1258::VENDOR_ID;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x1018:1: vendor ID = 0x" + std::to_string(value));
                return true;
            }
            else if (subindex == 2)
            {
                value = el1258::PRODUCT_CODE;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x1018:2: product code = 0x" + std::to_string(value));
                return true;
            }
            else if (subindex == 3)
            {
                value = el1258::REVISION;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x1018:3: revision = 0x" + std::to_string(value));
                return true;
            }
            else if (subindex == 4)
            {
                value = el1258::SERIAL_NUMBER_PLACEHOLDER;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x1018:4: serial number = 0x" + std::to_string(value));
                return true;
            }
            break;
        case el1258::OBJ_DIGITAL_INPUT:
            if (subindex == 0)
            {
                value = static_cast<uint32_t>(el1258::CHANNEL_COUNT);
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x6000:0: number of channels = " + std::to_string(value));
                return true;
            }
            if (subindex >= 1 && subindex <= el1258::CHANNEL_COUNT_U8)
            {
                value = channels_[subindex - 1] ? 1 : 0;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x6000:" + std::to_string(subindex) + ": channel " +
                          std::to_string(subindex) + " = " + std::to_string(value));
                return true;
            }
            break;
        case el1258::OBJ_DIGITAL_STATUS:
            if (subindex == 0)
            {
                value = static_cast<uint32_t>(el1258::CHANNEL_COUNT);
                LOG_DEBUG(
                    "El1258Slave[" + std::to_string(address()) +
                    "] SDO Upload 0x6001:0: number of status entries = " + std::to_string(value));
                return true;
            }
            if (subindex >= 1 && subindex <= el1258::CHANNEL_COUNT_U8)
            {
                // Status: bit 0 = input state, bit 1 = error, bit 2 = overrange, bit 3 = underrange
                value = channels_[subindex - 1] ? el1258::STATUS_INPUT_HIGH : 0u;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x6001:" + std::to_string(subindex) + ": channel " +
                          std::to_string(subindex) + " status = 0x" + std::to_string(value));
                return true;
            }
            break;
        case el1258::OBJ_DIGITAL_AGGREGATE:
            value = currentAggregate_();
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x6002: aggregate inputs = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_INVERT_MASK:
            value = invert_mask_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x8000: invert mask = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_DEBOUNCE_TIME:
            value = debounce_ms_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x8001: debounce ms = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_ERROR_REGISTER:
            value = error_register_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x8002: error register = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_MANUFACTURER_STATUS:
            value = manufacturer_status_;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Upload 0x8003: manufacturer status = 0x" + std::to_string(value));
            return true;
        case el1258::OBJ_ERROR_HISTORY:
            if (subindex == 0)
            {
                value = error_count_;
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Upload 0x8004:0: error count = " + std::to_string(value));
                return true;
            }
            break;
        default:
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Upload 0x" +
                      std::to_string(index) + ":" + std::to_string(subindex) + ": unknown index");
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
        case el1258::OBJ_INVERT_MASK:
            invert_mask_ = (value & el1258::BYTE_MASK);
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Download 0x8000: invert mask = 0x" + std::to_string(invert_mask_));
            return true;
        case el1258::OBJ_DEBOUNCE_TIME:
            debounce_ms_ = value;
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                      "] SDO Download 0x8001: debounce ms = " + std::to_string(debounce_ms_));
            return true;
        case el1258::PDO_MAPPING_BASE + 0 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 1 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 2 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 3 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 4 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 5 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 6 * el1258::PDO_MAPPING_STRIDE:
        case el1258::PDO_MAPPING_BASE + 7 * el1258::PDO_MAPPING_STRIDE:
            if (subindex == 0)
            {
                // Calculate channel number from index
                int channel = el1258::channelFromMapping(index);
                if (channel >= 1 && channel <= el1258::CHANNEL_MAX)
                {
                    map_count_ = static_cast<uint8_t>(value & el1258::BYTE_MASK);
                    LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x" +
                              std::to_string(index) + ":0: channel " + std::to_string(channel) +
                              " map count = " + std::to_string(map_count_));
                }
                return true;
            }
            if (subindex >= 1 && subindex <= el1258::CHANNEL_COUNT_U8)
            {
                // Calculate channel number from index
                int channel = el1258::channelFromMapping(index);
                if (channel >= 1 && channel <= el1258::CHANNEL_MAX)
                {
                    mappings_[channel - 1] = value;
                    LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x" +
                              std::to_string(index) + ":" + std::to_string(subindex) +
                              ": channel " + std::to_string(channel) + " mapping = 0x" +
                              std::to_string(value));
                }
                return true;
            }
            break;
        case el1258::PDO_ASSIGN_TX:
            if (subindex == 0)
            {
                assign_count_ = static_cast<uint8_t>(value & el1258::BYTE_MASK);
                LOG_DEBUG(
                    "El1258Slave[" + std::to_string(address()) +
                    "] SDO Download 0x1C13:0: PDO assign count = " + std::to_string(assign_count_));
                return true;
            }
            if (subindex == 1)
            {
                // PDO assignment: 0x1A00 means TxPDO 1 is assigned
                assigned_ = (value & el1258::WORD_MASK) == el1258::PDO_MAPPING_BASE;
                // Mark input PDO mapped if both mapping count and assign are set
                bool mapped = assigned_ && (map_count_ > 0);
                setInputPDOMapped(mapped);
                LOG_DEBUG("El1258Slave[" + std::to_string(address()) +
                          "] SDO Download 0x1C13:1: PDO assigned=" +
                          (assigned_ ? "true" : "false") + " (value=0x" + std::to_string(value) +
                          "), mapped=" + (mapped ? "true" : "false") +
                          ", input_pdo_mapped=" + (isInputPDOMapped() ? "true" : "false"));
                return true;
            }
            break;
        default:
            LOG_DEBUG("El1258Slave[" + std::to_string(address()) + "] SDO Download 0x" +
                      std::to_string(index) + ":" + std::to_string(subindex) + ": unknown index");
            break;
        }
        return false;
    }

  private:
    // Input filter structure for 3ms debouncing
    struct ChannelFilter
    {
        bool raw_state{false};                                // Raw input state
        bool debounce_active{false};                          // Debounce timer active
        std::chrono::steady_clock::time_point debounce_start; // Debounce start time
    };

    uint32_t currentAggregate_() const noexcept
    {
        uint32_t raw = 0;
        for (std::size_t i = 0; i < el1258::CHANNEL_COUNT; ++i)
            raw |= (channels_[i] ? (1u << i) : 0u);
        return (raw ^ invert_mask_) & el1258::CHANNEL_MASK;
    }

    std::array<bool, el1258::CHANNEL_COUNT> channels_{}; // Filtered channel states
    std::array<ChannelFilter, el1258::CHANNEL_COUNT>
        channel_filters_{}; // Input filters for each channel
    uint32_t invert_mask_{0};
    uint32_t debounce_ms_{0};

    // Status and error handling
    uint32_t error_register_{0};      // Error register (0x8002)
    uint32_t manufacturer_status_{0}; // Manufacturer status register (0x8003)
    uint32_t error_count_{0};         // Error count (0x8004:0)

    uint8_t map_count_{0};
    std::array<uint32_t, el1258::CHANNEL_COUNT> mappings_{};
    uint8_t assign_count_{0};
    bool assigned_{false};
};

} // namespace ethercat_sim::subs
