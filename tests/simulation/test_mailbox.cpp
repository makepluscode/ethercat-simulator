#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

#include "kickcat/protocol.h"

#include "ethercat_sim/simulation/network_simulator.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::simulation::VirtualSlave;

TEST(Mailbox, SDO_Upload_IdentityVendorId_Direct) {
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();
    auto s1 = std::make_shared<VirtualSlave>(1, 0x12345678u, 0x00001111u, "S1");
    sim.addVirtualSlave(s1);

    // Build mailbox CoE SDO UPLOAD request for 0x1018:1
    uint8_t msg[64] = {0};
    auto*   mbx     = reinterpret_cast<::kickcat::mailbox::Header*>(msg);
    auto*   coe     = ::kickcat::pointData<::kickcat::CoE::Header>(mbx);
    auto*   sdo     = ::kickcat::pointData<::kickcat::CoE::ServiceData>(coe);
    mbx->len        = 10;
    mbx->type       = ::kickcat::mailbox::CoE;
    mbx->count      = 1;
    coe->service    = ::kickcat::CoE::SDO_REQUEST;
    sdo->command    = ::kickcat::CoE::SDO::request::UPLOAD;
    sdo->index      = 0x1018;
    sdo->subindex   = 1;

    ASSERT_TRUE(sim.writeToSlave(1, 0x1000, msg, sizeof(msg)));

    uint8_t rx[64] = {0};
    ASSERT_TRUE(sim.readFromSlave(1, 0x1200, rx, sizeof(rx)));

    auto*    rmbx   = reinterpret_cast<::kickcat::mailbox::Header*>(rx);
    auto*    rcoe   = ::kickcat::pointData<::kickcat::CoE::Header>(rmbx);
    auto*    rsdo   = ::kickcat::pointData<::kickcat::CoE::ServiceData>(rcoe);
    uint32_t vendor = 0;
    std::memcpy(&vendor, ::kickcat::pointData<uint8_t>(rsdo), sizeof(uint32_t));
    EXPECT_EQ(vendor, 0x12345678u);
}
