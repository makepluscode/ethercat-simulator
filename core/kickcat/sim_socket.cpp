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

        uint16_t value = 0;
        switch (hdr->command) {
            case ::kickcat::Command::BRD:
            case ::kickcat::Command::BWR:
            case ::kickcat::Command::BRW:
                value = static_cast<uint16_t>(sim_->virtualSlaveCount());
                break;
            default:
                // Minimal emulation: acknowledge single-target operations
                value = (sim_->virtualSlaveCount() > 0) ? 1 : 0;
                break;
        }
        if (wkc) {
            *wkc = value;
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
