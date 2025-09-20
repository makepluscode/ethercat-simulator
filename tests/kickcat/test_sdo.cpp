#include <gtest/gtest.h>

#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"
#include "kickcat/protocol.h"

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::simulation::VirtualSlave;

namespace {
constexpr uint16_t MBX_RECV = 0x1000;
constexpr uint16_t MBX_SEND = 0x1200;
} // namespace

TEST(KickcatAdapter, SDO_Upload_IdentityVendorId) {
    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->clearSlaves();
    auto s1 = std::make_shared<VirtualSlave>(1, 0x12345678u, 0x00001111u, "S1");
    sim->addVirtualSlave(s1);

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, [] {});

    // Build CoE SDO Upload request for 0x1018:1
    uint8_t msg[64] = {0};
    auto*   mbx     = reinterpret_cast<::kickcat::mailbox::Header*>(msg);
    auto*   coe     = ::kickcat::pointData<::kickcat::CoE::Header>(mbx);
    auto*   sdo     = ::kickcat::pointData<::kickcat::CoE::ServiceData>(coe);
    mbx->len        = 10; // CoE header + SDO service data
    mbx->type       = ::kickcat::mailbox::CoE;
    mbx->count      = 1;
    coe->service    = ::kickcat::CoE::SDO_REQUEST;
    sdo->command    = ::kickcat::CoE::SDO::request::UPLOAD;
    sdo->index      = 0x1018;
    sdo->subindex   = 1;

    // Write mailbox request
    auto process_ok = [](::kickcat::DatagramHeader const*, uint8_t const*, uint16_t wkc) {
        return (wkc == 1) ? ::kickcat::DatagramState::OK : ::kickcat::DatagramState::INVALID_WKC;
    };
    auto error_throw = [](::kickcat::DatagramState const&) {
        throw std::runtime_error("FPWR failed");
    };
    link->addDatagram(::kickcat::Command::FPWR, ::kickcat::createAddress(1, MBX_RECV), msg, 64,
                      process_ok, error_throw);
    link->processDatagrams();

    // Read mailbox response from SM1 directly via simulator (robust to adapter details)
    uint8_t rx[64] = {0};
    ASSERT_TRUE(sim->readFromSlave(1, MBX_SEND, rx, sizeof(rx)));

    auto*    rmbx   = reinterpret_cast<::kickcat::mailbox::Header*>(rx);
    auto*    rcoe   = ::kickcat::pointData<::kickcat::CoE::Header>(rmbx);
    auto*    rsdo   = ::kickcat::pointData<::kickcat::CoE::ServiceData>(rcoe);
    uint32_t vendor = 0;
    std::memcpy(&vendor, ::kickcat::pointData<uint8_t>(rsdo), sizeof(uint32_t));
    EXPECT_EQ(vendor, 0x12345678u);
}
