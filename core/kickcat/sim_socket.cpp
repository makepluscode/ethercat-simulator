#include "ethercat_sim/kickcat/sim_socket.h"

#include "ethercat_sim/communication/ethercat_frame.h"
#include "kickcat/Frame.h"
#include "kickcat/protocol.h"

#include <algorithm>

namespace ethercat_sim::kickcat_adapter {

void SimSocket::open(std::string const& interface)
{
    (void)interface; // no real interface in simulation
}

void SimSocket::setTimeout(std::chrono::nanoseconds timeout)
{
    timeout_ = timeout;
    (void)timeout_; // currently unused in stub
}

void SimSocket::close() noexcept
{
}

int32_t SimSocket::write(uint8_t const* frame, int32_t frame_size)
{
    if (!sim_) {
        return -1;
    }
    // Parse the outgoing frame and simulate WKC according to simple rules
    // Default behavior: if link is down, drop
    if (!sim_->isLinkUp()) {
        return -1;
    }

    // Build a KickCAT frame view to access datagrams and WKC pointers
    ::kickcat::Frame f(frame, frame_size);

    while (true) {
        auto [hdr, data, wkc] = f.peekDatagram();
        if (hdr == nullptr) {
            break; // no more datagrams
        }

        uint16_t ack = 0;
        switch (hdr->command) {
            case ::kickcat::Command::BRD:
                // Broadcast read: just report how many slaves would respond
                ack = static_cast<uint16_t>(sim_->onlineSlaveCount());
                break;
            case ::kickcat::Command::BWR:
            case ::kickcat::Command::BRW: {
                // Broadcast write: apply to all online slaves at given ADO
                auto [pos, ado] = ::kickcat::extractAddress(hdr->address);
                (void)pos;
                uint16_t okc = 0;
                std::size_t online = sim_->onlineSlaveCount();
                for (std::size_t idx = 0; idx < online; ++idx) {
                    if (sim_->writeToSlaveByIndex(idx, ado, data, hdr->len)) {
                        ++okc;
                    }
                }
                ack = okc;
                break;
            }
            case ::kickcat::Command::FPRD:
            case ::kickcat::Command::FPWR:
            case ::kickcat::Command::FPRW: {
                // Station-addressed ops: operate on addressed slave registers
                auto [adp, ado] = ::kickcat::extractAddress(hdr->address);
                bool ok = false;
                switch (hdr->command) {
                    case ::kickcat::Command::FPRD:
                        ok = sim_->readFromSlave(adp, ado, data, hdr->len);
                        break;
                    case ::kickcat::Command::FPWR:
                        ok = sim_->writeToSlave(adp, ado, data, hdr->len);
                        break;
                    case ::kickcat::Command::FPRW: {
                        // read then write back provided payload
                        // minimal behavior: perform write and report success
                        ok = sim_->writeToSlave(adp, ado, data, hdr->len);
                        break;
                    }
                    default: break;
                }
                ack = ok ? 1 : 0;
                break;
            }
            case ::kickcat::Command::APRD:
            case ::kickcat::Command::APWR:
            case ::kickcat::Command::APRW: {
                // Auto-increment physical addressing: map position to slave index (0-based)
                auto [pos, ado] = ::kickcat::extractAddress(hdr->address);
                std::size_t idx = 0;
                if (pos == 0) {
                    idx = 0;
                } else if (pos & 0x8000) {
                    idx = static_cast<std::size_t>(static_cast<uint16_t>(0 - pos));
                } else {
                    idx = pos;
                }
                bool ok = false;
                switch (hdr->command) {
                    case ::kickcat::Command::APRD:
                        ok = sim_->readFromSlaveByIndex(idx, ado, data, hdr->len);
                        break;
                    case ::kickcat::Command::APWR:
                        ok = sim_->writeToSlaveByIndex(idx, ado, data, hdr->len);
                        break;
                    case ::kickcat::Command::APRW:
                        ok = sim_->writeToSlaveByIndex(idx, ado, data, hdr->len);
                        break;
                    default: break;
                }
                ack = ok ? 1 : 0;
                break;
            }
            case ::kickcat::Command::LRD:
            case ::kickcat::Command::LWR:
            case ::kickcat::Command::LRW: {
                // Minimal logical memory emulation in simulator
                uint32_t logical_address = hdr->address; // full 32-bit
                bool ok = false;
                switch (hdr->command) {
                    case ::kickcat::Command::LRD:
                        ok = sim_->readLogical(logical_address, data, hdr->len);
                        break;
                    case ::kickcat::Command::LWR:
                        ok = sim_->writeLogical(logical_address, data, hdr->len);
                        break;
                    case ::kickcat::Command::LRW:
                        ok = sim_->writeLogical(logical_address, data, hdr->len);
                        break;
                    default: break;
                }
                ack = ok ? 1 : 0;
                break;
            }
            default:
                // Minimal emulation: acknowledge single-target operations if any online
                ack = (sim_->onlineSlaveCount() > 0) ? 1 : 0;
                break;
        }
        if (wkc) {
            *wkc = ack;
        }
    }

    // Enqueue processed frame to the simulator receive queue
    communication::EtherCATFrame tx;
    tx.payload.assign(f.data(), f.data() + frame_size);
    bool ok = sim_->sendFrame(tx);
    return ok ? frame_size : -1;
}

int32_t SimSocket::read(uint8_t* frame, int32_t frame_size)
{
    if (!sim_) {
        return -1;
    }
    communication::EtherCATFrame rx;
    bool ok = sim_->receiveFrame(rx);
    if (!ok) {
        return 0; // non-blocking stub: no data available
    }
    int32_t n = static_cast<int32_t>(std::min(rx.payload.size(), static_cast<size_t>(frame_size)));
    std::copy(rx.payload.begin(), rx.payload.begin() + n, frame);
    return n;
}

} // namespace ethercat_sim::kickcat_adapter
