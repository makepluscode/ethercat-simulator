#pragma once

#include <cstddef>
#include <cstdint>

namespace ethercat_sim::simulation::slaves::el1258
{
inline constexpr std::size_t CHANNEL_COUNT = 8;
inline constexpr uint8_t CHANNEL_COUNT_U8  = static_cast<uint8_t>(CHANNEL_COUNT);
inline constexpr int CHANNEL_MAX           = static_cast<int>(CHANNEL_COUNT);

inline constexpr uint32_t VENDOR_ID                 = 0x00000002u;
inline constexpr uint32_t PRODUCT_CODE              = 0x04EA3052u;
inline constexpr uint32_t REVISION                  = 0x00110000u;
inline constexpr uint32_t DEVICE_TYPE_CODE          = 0x00000000u;
inline constexpr uint32_t HARDWARE_VERSION_VALUE    = 0x00000001u;
inline constexpr uint32_t SOFTWARE_VERSION_VALUE    = 0x00000001u;
inline constexpr uint32_t SERIAL_NUMBER_PLACEHOLDER = 0x00000000u;
inline constexpr uint8_t IDENTITY_ENTRY_COUNT       = 4u;

inline constexpr uint16_t OBJ_DEVICE_TYPE         = 0x1000u;
inline constexpr uint16_t OBJ_DEVICE_NAME         = 0x1008u;
inline constexpr uint16_t OBJ_HARDWARE_VERSION    = 0x1009u;
inline constexpr uint16_t OBJ_SOFTWARE_VERSION    = 0x100Au;
inline constexpr uint16_t OBJ_IDENTITY            = 0x1018u;
inline constexpr uint16_t OBJ_DIGITAL_INPUT       = 0x6000u;
inline constexpr uint16_t OBJ_DIGITAL_STATUS      = 0x6001u;
inline constexpr uint16_t OBJ_DIGITAL_AGGREGATE   = 0x6002u;
inline constexpr uint16_t OBJ_INVERT_MASK         = 0x8000u;
inline constexpr uint16_t OBJ_DEBOUNCE_TIME       = 0x8001u;
inline constexpr uint16_t OBJ_ERROR_REGISTER      = 0x8002u;
inline constexpr uint16_t OBJ_MANUFACTURER_STATUS = 0x8003u;
inline constexpr uint16_t OBJ_ERROR_HISTORY       = 0x8004u;

inline constexpr uint16_t PDO_MAPPING_BASE    = 0x1A00u;
inline constexpr uint16_t PDO_MAPPING_STRIDE  = 0x0004u;
inline constexpr uint8_t PDO_ENTRY_BIT_LENGTH = 1u;
inline constexpr uint16_t PDO_ASSIGN_TX       = 0x1C13u;

inline constexpr uint32_t CHANNEL_MASK     = 0xFFu;
inline constexpr uint8_t STATUS_INPUT_HIGH = 0x01u;
inline constexpr int DEBOUNCE_FILTER_MS    = 3;
inline constexpr uint32_t BYTE_MASK        = 0xFFu;
inline constexpr uint32_t WORD_MASK        = 0xFFFFu;

inline constexpr uint32_t makeChannelMapping(uint8_t subindex) noexcept
{
    return (static_cast<uint32_t>(OBJ_DIGITAL_INPUT) << 16) |
           (static_cast<uint32_t>(subindex) << 8) | static_cast<uint32_t>(PDO_ENTRY_BIT_LENGTH);
}

inline constexpr int channelFromMapping(uint16_t mapping_index) noexcept
{
    return static_cast<int>((mapping_index - PDO_MAPPING_BASE) / PDO_MAPPING_STRIDE) + 1;
}

} // namespace ethercat_sim::simulation::slaves::el1258
