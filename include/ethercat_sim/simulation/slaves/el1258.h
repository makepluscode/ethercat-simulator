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
            uint32_t bits = 0;
            for (int i = 0; i < 8; ++i) bits |= (di_[i] ? (1u << i) : 0u);
            value = bits;
            return true;
        }
        return false;
    }

    bool readDigitalInputsBitfield(uint32_t& bits_out) const noexcept override
    {
        uint32_t bits = 0;
        for (int i = 0; i < 8; ++i) bits |= (di_[i] ? (1u << i) : 0u);
        bits_out = bits;
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
};

} // namespace ethercat_sim::simulation::slaves
