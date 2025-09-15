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
        // Initialize default mailbox offsets/sizes (standard mailbox)
        mb_recv_offset_ = 0x1000;
        mb_recv_size_   = 512;
        mb_send_offset_ = 0x1200;
        mb_send_size_   = 512;
        mb_out_.assign(mb_send_size_, 0);
        mb_in_.assign(mb_recv_size_, 0);

        syncCoreRegisters_();
        syncSMRegisters_();
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
        // Handle EEPROM data read (4 bytes per request)
        if (reg == ::kickcat::reg::EEPROM_DATA && len >= 4) {
            uint32_t val = 0;
            uint16_t lo = 0, hi = 0;
            if (eeprom_addr_ + 1 < eeprom_.size()) {
                lo = eeprom_[eeprom_addr_];
                hi = eeprom_[eeprom_addr_ + 1];
            }
            val = static_cast<uint32_t>(lo) | (static_cast<uint32_t>(hi) << 16);
            std::memcpy(dst, &val, sizeof(uint32_t));
            return true;
        }

        // ESC register space
        if ((static_cast<std::size_t>(reg) + len) <= regs_.size()) {
            std::copy(regs_.begin() + reg, regs_.begin() + reg + len, dst);
            return true;
        }

        // Mailbox send area (slave -> master)
        if ((reg >= mb_send_offset_) && ((static_cast<std::size_t>(reg) + len) <= (mb_send_offset_ + mb_send_size_))) {
            std::size_t off = static_cast<std::size_t>(reg - mb_send_offset_);
            std::size_t n = std::min(len, mb_out_.size() - off);
            std::copy(mb_out_.begin() + off, mb_out_.begin() + off + n, dst);
            // Reading the whole message: clear can_read flag
            mb_have_reply_ = false;
            syncSMStatus_();
            return true;
        }

        return false;
    }

    bool write(std::uint16_t reg, std::uint8_t const* src, std::size_t len) noexcept
    {
        // Handle EEPROM control write (request)
        if (reg == ::kickcat::reg::EEPROM_CONTROL && len >= 6) {
            // struct { uint16_t command; uint16_t addressLow; uint16_t addressHigh; }
            uint16_t addressLow = static_cast<uint16_t>(src[2] | (static_cast<uint16_t>(src[3]) << 8));
            eeprom_addr_ = addressLow; // words
            return true;
        }

        // ESC register space
        if ((static_cast<std::size_t>(reg) + len) <= regs_.size()) {
            std::copy(src, src + len, regs_.begin() + reg);
        }
        else if ((reg >= mb_recv_offset_) && ((static_cast<std::size_t>(reg) + len) <= (mb_recv_offset_ + mb_recv_size_))) {
            // Mailbox recv area (master -> slave)
            std::size_t off = static_cast<std::size_t>(reg - mb_recv_offset_);
            std::size_t n = std::min(len, mb_in_.size() - off);
            std::copy(src, src + n, mb_in_.begin() + off);
            handleMailboxWrite_(off, n);
            return true;
        }
        else if ((reg >= ::kickcat::reg::SYNC_MANAGER) && (reg < (::kickcat::reg::SYNC_MANAGER + 16))) {
            // Update mailbox SM config if written via FPWR
            // We accept up to 2 SM entries (8 bytes each)
            std::size_t remaining = len;
            std::size_t pos = 0;
            while (remaining >= 8) {
                auto start = static_cast<uint16_t>(src[pos] | (static_cast<uint16_t>(src[pos+1]) << 8));
                auto length= static_cast<uint16_t>(src[pos+2] | (static_cast<uint16_t>(src[pos+3]) << 8));
                int sm_index = static_cast<int>((reg + pos - ::kickcat::reg::SYNC_MANAGER) / 8);
                if (sm_index == 0) { mb_recv_offset_ = start; mb_recv_size_ = length; if (mb_in_.size() != mb_recv_size_) mb_in_.assign(mb_recv_size_, 0); }
                if (sm_index == 1) { mb_send_offset_ = start; mb_send_size_ = length; if (mb_out_.size() != mb_send_size_) mb_out_.assign(mb_send_size_, 0); }
                pos += 8; remaining -= 8;
            }
            syncSMRegisters_();
            return true;
        }
        else {
            return false;
        }

        // If STATION_ADDR is written, keep internal address in sync
        if (reg == 0x0010 && len >= 2) {
            address_ = static_cast<std::uint16_t>(regs_[0x0010] | (static_cast<uint16_t>(regs_[0x0011]) << 8));
        }
        // If AL_CONTROL written, update AL_STATUS accordingly (minimal behavior)
        if (reg == ::kickcat::reg::AL_CONTROL && len >= 1) {
            uint8_t req = regs_[::kickcat::reg::AL_CONTROL];
            req &= static_cast<uint8_t>(~::kickcat::State::ACK);
            auto target = static_cast<::kickcat::State>(req);
            bool allow = false;
            switch (target) {
                case ::kickcat::State::INIT:
                    allow = true; break;
                case ::kickcat::State::PRE_OP:
                    allow = true; break; // 기본 허용
                case ::kickcat::State::SAFE_OP:
                    // 최소 조건: 입력 PDO 매핑이 준비되어야 함(입력 전용 슬레이브 가정)
                    allow = input_pdo_mapped_;
                    break;
                case ::kickcat::State::OPERATIONAL:
                    // 최소 조건: SAFE_OP 조건 충족 + 입력 PDO 매핑 유지
                    allow = input_pdo_mapped_;
                    break;
                default:
                    allow = true; // 기타 상태는 제한 없음
                    break;
            }
            if (allow) {
                al_state_ = target;
                al_status_code_ = 0; // OK
                syncCoreRegisters_();
            } else {
                // Set specific error code for SAFE_OP/OPERATIONAL gating
                if (target == ::kickcat::State::SAFE_OP || target == ::kickcat::State::OPERATIONAL) {
                    al_status_code_ = 0x0011; // Inputs not mapped
                } else {
                    al_status_code_ = 0x0001; // Unspecified error
                }
                // Do not change al_state_ (remains INIT)
                syncCoreRegisters_();
            }
        }
        return true;
    }

    // Optional: return digital inputs bitfield for PDO mapping (LSB=channel0)
    virtual bool readDigitalInputsBitfield(uint32_t& /*bits_out*/) const noexcept { return false; }

    // Make these public for access from NetworkSimulator
    void setInputPDOMapped(bool v) noexcept { input_pdo_mapped_ = v; }
    bool inputPDOMapped() const noexcept { return input_pdo_mapped_; }

protected:
    // Allow derived classes (specific slaves) to answer SDO Upload values
    virtual bool onSdoUpload(uint16_t /*index*/, uint8_t /*subindex*/, uint32_t& /*value*/) const noexcept { return false; }

    // Allow derived classes to handle SDO Download writes (expedited only, up to 4 bytes)
    virtual bool onSdoDownload(uint16_t /*index*/, uint8_t /*subindex*/, uint32_t /*value*/, uint8_t /*nbytes*/) noexcept { return false; }

    // (legacy placeholder removed)

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

    void syncSMRegisters_() noexcept
    {
        // SM0: mailbox out (master -> slave)
        regs_.at(::kickcat::reg::SYNC_MANAGER_0 + 0) = static_cast<uint8_t>(mb_recv_offset_ & 0xFF);
        regs_.at(::kickcat::reg::SYNC_MANAGER_0 + 1) = static_cast<uint8_t>((mb_recv_offset_ >> 8) & 0xFF);
        regs_.at(::kickcat::reg::SYNC_MANAGER_0 + 2) = static_cast<uint8_t>(mb_recv_size_ & 0xFF);
        regs_.at(::kickcat::reg::SYNC_MANAGER_0 + 3) = static_cast<uint8_t>((mb_recv_size_ >> 8) & 0xFF);
        // control/status/activate/pdi_control left mostly for Bus writes; we maintain status bit
        // SM1: mailbox in (slave -> master)
        regs_.at(::kickcat::reg::SYNC_MANAGER_1 + 0) = static_cast<uint8_t>(mb_send_offset_ & 0xFF);
        regs_.at(::kickcat::reg::SYNC_MANAGER_1 + 1) = static_cast<uint8_t>((mb_send_offset_ >> 8) & 0xFF);
        regs_.at(::kickcat::reg::SYNC_MANAGER_1 + 2) = static_cast<uint8_t>(mb_send_size_ & 0xFF);
        regs_.at(::kickcat::reg::SYNC_MANAGER_1 + 3) = static_cast<uint8_t>((mb_send_size_ >> 8) & 0xFF);
        syncSMStatus_();
    }

    void syncSMStatus_() const noexcept
    {
        // Update only status byte for SM0 and SM1
        // SM0 status: we keep writable -> not full
        auto& reg0 = const_cast<std::vector<std::uint8_t>&>(regs_);
        reg0[::kickcat::reg::SYNC_MANAGER_0 + ::kickcat::reg::SM_STATS] = 0x00;
        // SM1 status: set MAILBOX_STATUS when we have a reply ready
        uint8_t st1 = mb_have_reply_ ? ::kickcat::MAILBOX_STATUS : 0x00;
        reg0[::kickcat::reg::SYNC_MANAGER_1 + ::kickcat::reg::SM_STATS] = st1;
    }

    void handleMailboxWrite_(std::size_t offset, std::size_t len) noexcept
    {
        (void)offset; (void)len;
        // For simplicity, assume a whole message is written at once starting at offset 0
        auto* header = reinterpret_cast<::kickcat::mailbox::Header*>(mb_in_.data());
        auto* coe = ::kickcat::pointData<::kickcat::CoE::Header>(header);
        auto* sdo = ::kickcat::pointData<::kickcat::CoE::ServiceData>(coe);
        // Minimal SDO handling: Upload/Download expedited only
        uint16_t req_index = sdo->index;
        uint8_t  req_sub   = sdo->subindex;

        // Prepare response header
        std::fill(mb_out_.begin(), mb_out_.end(), 0);
        auto* rh = reinterpret_cast<::kickcat::mailbox::Header*>(mb_out_.data());
        auto* rc = ::kickcat::pointData<::kickcat::CoE::Header>(rh);
        auto* rs = ::kickcat::pointData<::kickcat::CoE::ServiceData>(rc);
        rh->type = ::kickcat::mailbox::CoE;
        rh->count = header->count; // echo session handle
        rc->service = ::kickcat::CoE::SDO_RESPONSE;
        rc->number = 0;
        rs->index = req_index;
        rs->subindex = req_sub;
        rs->transfer_type = 1; // expedited
        rs->size_indicator = 1;
        rs->block_size = 0;

        if (coe->service == ::kickcat::CoE::SDO_REQUEST && sdo->command == ::kickcat::CoE::SDO::request::UPLOAD) {
            uint32_t value = 0;
            bool handled = false;
            if (req_index == 0x1018 && req_sub == 1) { value = vendor_id_; handled = true; }
            else if (req_index == 0x1018 && req_sub == 2) { value = product_code_; handled = true; }
            else if (req_index == 0x1018 && req_sub == 3) { value = 0; handled = true; }
            else if (req_index == 0x1018 && req_sub == 4) { value = 0; handled = true; }
            else {
                uint32_t v = 0;
                if (onSdoUpload(req_index, req_sub, v)) {
                    value = v;
                    handled = true;
                }
            }
            // Upload response
            rs->command = ::kickcat::CoE::SDO::response::UPLOAD;
            std::memcpy(::kickcat::pointData<uint8_t>(rs), &value, sizeof(uint32_t));
            rh->len = 10; // expedited payload
        }
        else if (coe->service == ::kickcat::CoE::SDO_REQUEST && sdo->command == ::kickcat::CoE::SDO::request::DOWNLOAD) {
            // Read up to 4 bytes of expedited data
            uint32_t v = 0;
            std::memcpy(&v, ::kickcat::pointData<uint8_t>(sdo), sizeof(uint32_t));
            // We don't parse nbytes precisely; assume 4 for simplicity
            (void)onSdoDownload(req_index, req_sub, v, 4);
            // Download ack response (no data)
            rs->command = ::kickcat::CoE::SDO::response::DOWNLOAD;
            rh->len = 10; // minimal header length kept consistent
        }
        else {
            // Unknown, still respond with UPLOAD and zero
            rs->command = ::kickcat::CoE::SDO::response::UPLOAD;
            uint32_t zero = 0;
            std::memcpy(::kickcat::pointData<uint8_t>(rs), &zero, sizeof(uint32_t));
            rh->len = 10;
        }

        mb_have_reply_ = true;
        syncSMStatus_();
    }

    std::uint16_t address_ {};
    std::uint32_t vendor_id_ {};
    std::uint32_t product_code_ {};
    std::string name_;
    bool online_ {true};
    ::kickcat::State al_state_ {::kickcat::State::INIT};
    uint16_t al_status_code_ {0};
    std::vector<std::uint8_t> regs_ = std::vector<std::uint8_t>(4096, 0);
    bool input_pdo_mapped_ {false};

    // Mailbox (standard) minimal simulation
    uint16_t mb_recv_offset_ {0};
    uint16_t mb_recv_size_ {0};
    uint16_t mb_send_offset_ {0};
    uint16_t mb_send_size_ {0};
    mutable bool mb_have_reply_ {false};
    mutable std::vector<std::uint8_t> mb_out_;
    std::vector<std::uint8_t> mb_in_;

    // EEPROM / SII minimal stub
    uint16_t eeprom_addr_ {0}; // word address for next read
    std::vector<uint16_t> eeprom_ = []{
        std::vector<uint16_t> v(128, 0);
        // Ensure End category (0xFFFF) appears after 32 double-words
        // so Bus::fetchEeprom() can stop
        if (v.size() > 66) {
            v[65] = 0xFFFF; // upper 16-bits at pos=64
        }
        return v;
    }();
};

} // namespace ethercat_sim::simulation
