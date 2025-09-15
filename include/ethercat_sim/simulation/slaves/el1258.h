#pragma once

#include "ethercat_sim/simulation/virtual_slave.h"

namespace ethercat_sim::simulation::slaves {

// Beckhoff EL1258-compatible digital input terminal (8x DI).
// Notes:
// - Vendor/Product codes are placeholders; adjust to match your environment if needed.
// - 8 채널 중 DI0=POWER_BUTTON, DI1=AC_ON(전원 상태 반영), 나머지 기본 false.
class EL1258Slave final : public VirtualSlave {
public:
    explicit EL1258Slave(std::uint16_t address,
                         std::uint32_t vendor_id = 0x00000002u,      // Beckhoff (placeholder)
                         std::uint32_t product_code = 0x00001258u,   // EL1258 (placeholder)
                         std::string name = "EL1258")
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
        txpdo_count_ = 1;
        txpdo_entries_[0] = (0x6002u << 16) | (0x00u << 8) | 8u;
        assign_count_ = 1;
        assign_entries_[0] = 0x1A00;
        setInputPDOMapped(true);
    }

    // External controls
    void setPower(bool on) noexcept { power_ = on; updateDerivedInputs_(); }
    bool power() const noexcept { return power_; }

    void setPowerButton(bool pressed) noexcept { power_button_ = pressed; di_[0] = pressed; }
    bool powerButton() const noexcept { return power_button_; }

    void setInput(int channel, bool value) noexcept {
        if (channel >= 0 && channel < 8) {
            di_[channel] = value;
            if (channel == 1) power_ = value; // AC_ON과 전원 동기화 허용(명시적 설정 시)
        }
    }
    bool input(int channel) const noexcept { return (channel >= 0 && channel < 8) ? di_[channel] : false; }

protected:
    // CoE SDO Upload hook
    bool onSdoUpload(uint16_t index, uint8_t subindex, uint32_t& value) const noexcept override
    {
        // 0x6000: Digital Inputs, subindex 1..8 -> boolean
        if (index == 0x6000 && subindex >= 1 && subindex <= 8) {
            value = di_[subindex - 1] ? 1u : 0u;
            return true;
        }
        // 0x6002: Optionally return aggregated bitfield (bit0..bit7)
        if (index == 0x6002 && subindex == 0x00) {
            value = aggregateBits_();
            return true;
        }
        // 0x1A00: TxPDO mapping (readback)
        if (index == 0x1A00) {
            if (subindex == 0) { value = txpdo_count_; return true; }
            uint8_t si = subindex - 1;
            if (subindex >= 1 && si < 8) { value = txpdo_entries_[si]; return true; }
        }
        // 0x1C13: SyncManager TxPDO assignment (readback)
        if (index == 0x1C13) {
            if (subindex == 0) { value = assign_count_; return true; }
            uint8_t si = subindex - 1;
            if (subindex >= 1 && si < 4) { value = assign_entries_[si]; return true; }
        }
        return false;
    }

    bool onSdoDownload(uint16_t index, uint8_t subindex, uint32_t value, uint8_t /*nbytes*/) noexcept override
    {
        bool changed = false;
        if (index == 0x1A00) {
            if (subindex == 0) {
                txpdo_count_ = static_cast<uint8_t>(value & 0xFF);
                if (txpdo_count_ > 8) txpdo_count_ = 8;
                // Clear existing entries beyond count
                for (int i = txpdo_count_; i < 8; ++i) txpdo_entries_[i] = 0;
                changed = true;
            } else if (subindex >= 1 && subindex <= 8) {
                txpdo_entries_[subindex - 1] = value;
                changed = true;
            }
        }
        else if (index == 0x1C13) {
            if (subindex == 0) {
                assign_count_ = static_cast<uint8_t>(value & 0xFF);
                if (assign_count_ > 4) assign_count_ = 4;
                for (int i = assign_count_; i < 4; ++i) assign_entries_[i] = 0;
                changed = true;
            } else if (subindex >= 1 && subindex <= 4) {
                assign_entries_[subindex - 1] = static_cast<uint16_t>(value & 0xFFFF);
                changed = true;
            }
        }
        if (changed) {
            // Consider mapping valid when TxPDO assigned and at least one entry mapped
            bool assigned = false;
            for (int i = 0; i < assign_count_; ++i) {
                if (assign_entries_[i] == 0x1A00) { assigned = true; break; }
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
        di_[1] = power_;     // DI1 mirrors AC_ON
        di_[0] = power_button_; // DI0 = POWER_BUTTON
        // DI2..DI7 remain as-is (default false)
    }

    bool power_ {false};
    bool power_button_ {false};
    bool di_[8] {false, false, false, false, false, false, false, false};

    // Minimal CoE PDO mapping state
    uint8_t txpdo_count_ {0};
    uint32_t txpdo_entries_[8] {0,0,0,0,0,0,0,0};
    uint8_t assign_count_ {0};
    uint16_t assign_entries_[4] {0,0,0,0};

    uint32_t aggregateBits_() const noexcept
    {
        uint32_t bits = 0;
        for (int i = 0; i < 8; ++i) bits |= (di_[i] ? (1u << i) : 0u);
        return bits;
    }
};

} // namespace ethercat_sim::simulation::slaves
