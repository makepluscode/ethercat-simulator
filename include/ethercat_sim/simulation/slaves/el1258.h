#pragma once

#include "ethercat_sim/simulation/slaves/el1258_constants.h"
#include "ethercat_sim/simulation/virtual_slave.h"

namespace ethercat_sim::simulation::slaves
{

// Beckhoff EL1258-compatible digital input terminal (8x DI).
// Notes:
// - Vendor/Product codes are placeholders; adjust to match your environment if needed.
// - 8 채널 중 DI0=POWER_BUTTON, DI1=AC_ON(전원 상태 반영), 나머지 기본 false.
class EL1258Slave final : public VirtualSlave
{
  public:
    explicit EL1258Slave(std::uint16_t address, std::uint32_t vendor_id = el1258::VENDOR_ID,
                         std::uint32_t product_code = el1258::PRODUCT_CODE,
                         std::string name           = "EL1258")
        : VirtualSlave(address, vendor_id, product_code, std::move(name))
    {
        updateDerivedInputs_();
    }

    // Apply a sensible default TxPDO mapping for 8 DI channels:
    // - 0x1A00:01 = 0x6002:00 (8 bits aggregate)
    // - 0x1C13:01 = 0x1A00 assignment
    // This marks inputs as PDO-mapped for AL state gating purposes.
    void applyDefaultTxPdoMapping() noexcept
    {
        txpdo_count_      = 1;
        txpdo_entries_[0] = (static_cast<uint32_t>(el1258::OBJ_DIGITAL_AGGREGATE) << 16) |
                            (0x00u << 8) | static_cast<uint32_t>(el1258::CHANNEL_COUNT_U8);
        assign_count_      = 1;
        assign_entries_[0] = el1258::PDO_MAPPING_BASE;
        setInputPDOMapped(true);
    }

    // External controls
    void setPower(bool on) noexcept
    {
        power_ = on;
        updateDerivedInputs_();
    }
    bool power() const noexcept
    {
        return power_;
    }

    void setPowerButton(bool pressed) noexcept
    {
        power_button_ = pressed;
        setRawInput_(0, pressed);
    }
    bool powerButton() const noexcept
    {
        return power_button_;
    }

    void setInput(int channel, bool value) noexcept
    {
        if (channel >= 0 && channel < static_cast<int>(el1258::CHANNEL_COUNT))
        {
            setRawInput_(channel, value);
            if (channel == 1)
                power_ = value; // AC_ON과 전원 동기화 허용(명시적 설정 시)
        }
    }
    bool input(int channel) const noexcept
    {
        return (channel >= 0 && channel < static_cast<int>(el1258::CHANNEL_COUNT))
                   ? effectiveBit_(channel)
                   : false;
    }

  protected:
    // CoE SDO Upload hook
    bool onSdoUpload(uint16_t index, uint8_t subindex, uint32_t& value) const noexcept override
    {
        // Basic device information (minimal, 4-byte expedited only)
        if (index == el1258::OBJ_DEVICE_TYPE && subindex == 0x00)
        {
            value = device_type_code_;
            return true;
        }
        if (index == el1258::OBJ_DEVICE_NAME && subindex == 0x00)
        {
            value = pack4_(device_name_);
            return true;
        }
        if (index == el1258::OBJ_HARDWARE_VERSION && subindex == 0x00)
        {
            value = pack4_(hardware_version_);
            return true;
        }
        if (index == el1258::OBJ_SOFTWARE_VERSION && subindex == 0x00)
        {
            value = pack4_(software_version_);
            return true;
        }
        // 0x6000: Digital Inputs, subindex 1..8 -> boolean
        if (index == el1258::OBJ_DIGITAL_INPUT && subindex >= 1 &&
            subindex <= el1258::CHANNEL_COUNT_U8)
        {
            value = effectiveBit_(subindex - 1) ? 1u : 0u;
            return true;
        }
        // 0x6002: Optionally return aggregated bitfield (bit0..bit7)
        if (index == el1258::OBJ_DIGITAL_AGGREGATE && subindex == 0x00)
        {
            value = aggregateBits_();
            return true;
        }
        // 0x8000: Invert mask (bit0..bit7). subindex 0
        if (index == el1258::OBJ_INVERT_MASK && subindex == 0x00)
        {
            value = invert_mask_;
            return true;
        }
        // 0x8001: Debounce time (ms) global. subindex 0
        if (index == el1258::OBJ_DEBOUNCE_TIME && subindex == 0x00)
        {
            value = debounce_ms_;
            return true;
        }
        // 0x1A00: TxPDO mapping (readback)
        if (index == el1258::PDO_MAPPING_BASE)
        {
            if (subindex == 0)
            {
                value = txpdo_count_;
                return true;
            }
            uint8_t si = subindex - 1;
            if (subindex >= 1 && si < el1258::CHANNEL_COUNT)
            {
                value = txpdo_entries_[si];
                return true;
            }
        }
        // 0x1C13: SyncManager TxPDO assignment (readback)
        if (index == el1258::PDO_ASSIGN_TX)
        {
            if (subindex == 0)
            {
                value = assign_count_;
                return true;
            }
            uint8_t si = subindex - 1;
            if (subindex >= 1 && si < 4)
            {
                value = assign_entries_[si];
                return true;
            }
        }
        return false;
    }

    bool onSdoDownload(uint16_t index, uint8_t subindex, uint32_t value,
                       uint8_t /*nbytes*/) noexcept override
    {
        bool changed = false;
        if (index == el1258::OBJ_INVERT_MASK && subindex == 0x00)
        {
            invert_mask_ = static_cast<uint8_t>(value & 0xFF);
            changed      = true;
        }
        else if (index == el1258::OBJ_DEBOUNCE_TIME && subindex == 0x00)
        {
            debounce_ms_ = static_cast<uint16_t>(value & 0xFFFF);
            changed      = true;
        }
        else if (index == el1258::PDO_MAPPING_BASE)
        {
            if (subindex == 0)
            {
                txpdo_count_ = static_cast<uint8_t>(value & 0xFF);
                if (txpdo_count_ > el1258::CHANNEL_COUNT_U8)
                    txpdo_count_ = el1258::CHANNEL_COUNT_U8;
                // Clear existing entries beyond count
                for (int i = txpdo_count_; i < static_cast<int>(el1258::CHANNEL_COUNT); ++i)
                    txpdo_entries_[i] = 0;
                changed = true;
            }
            else if (subindex >= 1 && subindex <= el1258::CHANNEL_COUNT_U8)
            {
                txpdo_entries_[subindex - 1] = value;
                changed                      = true;
            }
        }
        else if (index == el1258::PDO_ASSIGN_TX)
        {
            if (subindex == 0)
            {
                assign_count_ = static_cast<uint8_t>(value & 0xFF);
                if (assign_count_ > 4)
                    assign_count_ = 4;
                for (int i = assign_count_; i < 4; ++i)
                    assign_entries_[i] = 0;
                changed = true;
            }
            else if (subindex >= 1 && subindex <= 4)
            {
                assign_entries_[subindex - 1] = static_cast<uint16_t>(value & 0xFFFF);
                changed                       = true;
            }
        }
        if (changed)
        {
            // Consider mapping valid when TxPDO assigned and at least one entry mapped
            bool assigned = false;
            for (int i = 0; i < assign_count_; ++i)
            {
                if (assign_entries_[i] == el1258::PDO_MAPPING_BASE)
                {
                    assigned = true;
                    break;
                }
            }
            bool has_entries = txpdo_count_ > 0;
            setInputPDOMapped(assigned && has_entries);
        }
        return changed;
    }

    bool readDigitalInputsBitfield(uint32_t& bits_out) const noexcept override
    {
        bits_out = aggregateBits_();
        return true;
    }

  private:
    void updateDerivedInputs_() noexcept
    {
        setRawInput_(1, power_);        // DI1 mirrors AC_ON
        setRawInput_(0, power_button_); // DI0 = POWER_BUTTON
        // DI2..DI7 remain as-is (default false)
    }

    bool power_{false};
    bool power_button_{false};
    bool raw_di_[el1258::CHANNEL_COUNT]{false, false, false, false, false, false, false, false};
    bool eff_di_[el1258::CHANNEL_COUNT]{false, false, false, false, false, false, false, false};
    uint8_t invert_mask_{0};
    uint16_t debounce_ms_{0};
    std::chrono::steady_clock::time_point last_change_[el1258::CHANNEL_COUNT]{
        std::chrono::steady_clock::now(), std::chrono::steady_clock::now(),
        std::chrono::steady_clock::now(), std::chrono::steady_clock::now(),
        std::chrono::steady_clock::now(), std::chrono::steady_clock::now(),
        std::chrono::steady_clock::now(), std::chrono::steady_clock::now()};

    // Minimal CoE PDO mapping state
    uint8_t txpdo_count_{0};
    uint32_t txpdo_entries_[el1258::CHANNEL_COUNT]{0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t assign_count_{0};
    uint16_t assign_entries_[4]{0, 0, 0, 0};

    void setRawInput_(int ch, bool v) noexcept
    {
        if (ch < 0 || ch >= static_cast<int>(el1258::CHANNEL_COUNT))
            return;
        if (raw_di_[ch] != v)
        {
            raw_di_[ch]      = v;
            last_change_[ch] = std::chrono::steady_clock::now();
        }
    }

    bool effectiveBit_(int ch) const noexcept
    {
        if (ch < 0 || ch >= static_cast<int>(el1258::CHANNEL_COUNT))
            return false;
        // Debounce: update eff only if stable long enough
        if (debounce_ms_ == 0)
        {
            return ((raw_di_[ch] ^ ((invert_mask_ >> ch) & 0x1)) != 0);
        }
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_change_[ch]).count();
        if (eff_di_[ch] != raw_di_[ch] && elapsed >= debounce_ms_)
        {
            const_cast<EL1258Slave*>(this)->eff_di_[ch] = raw_di_[ch];
        }
        bool val = eff_di_[ch];
        val      = val ^ (((invert_mask_ >> ch) & 0x1) != 0);
        return val;
    }

    uint32_t aggregateBits_() const noexcept
    {
        uint32_t bits = 0;
        for (int i = 0; i < static_cast<int>(el1258::CHANNEL_COUNT); ++i)
            bits |= (effectiveBit_(i) ? (1u << i) : 0u);
        return bits;
    }

    // Pack first 4 characters into a 32-bit value (expedited SDO simplification)
    uint32_t pack4_(const char* s) const noexcept
    {
        char buf[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4 && s && s[i]; ++i)
            buf[i] = s[i];
        uint32_t v = 0;
        std::memcpy(&v, buf, sizeof(v));
        return v;
    }

    // Minimal identification strings (truncated to 4 bytes in expedited SDO)
    static constexpr uint32_t device_type_code_    = 0x00000000u;
    static constexpr const char* device_name_      = "EL1258";
    static constexpr const char* hardware_version_ = "HW10";
    static constexpr const char* software_version_ = "SW10";
};

} // namespace ethercat_sim::simulation::slaves
