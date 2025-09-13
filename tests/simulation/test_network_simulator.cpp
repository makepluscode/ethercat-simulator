#include <gtest/gtest.h>
#include <cstring>

#include "ethercat_sim/simulation/network_simulator.h"
#include "ethercat_sim/communication/ethercat_frame.h"
#include "ethercat_sim/simulation/virtual_slave.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::communication::EtherCATFrame;

TEST(NetworkSimulator, Loopback_WhenLinkUp_SendsAndReceives)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.setLinkUp(true);

    EtherCATFrame tx; tx.payload = {0x01, 0x02, 0x03};
    ASSERT_TRUE(sim.sendFrame(tx));

    EtherCATFrame rx;
    ASSERT_TRUE(sim.receiveFrame(rx));
    EXPECT_EQ(rx.payload, tx.payload);
}

TEST(NetworkSimulator, SendReceive_WhenLinkDown_Fails)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.setLinkUp(false);

    EtherCATFrame tx; tx.payload = {0xAA};
    EXPECT_FALSE(sim.sendFrame(tx));

    EtherCATFrame rx;
    EXPECT_FALSE(sim.receiveFrame(rx));
}

TEST(NetworkSimulator, VirtualSlaveRegistry_AddAndCount)
{
    using ethercat_sim::simulation::VirtualSlave;

    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();
    EXPECT_EQ(sim.virtualSlaveCount(), 0u);

    // Set via count API
    sim.setVirtualSlaveCount(2);
    EXPECT_EQ(sim.virtualSlaveCount(), 2u);

    // Add an actual slave object
    auto s = std::make_shared<VirtualSlave>(/*address*/1, /*vendor*/0x9A, /*product*/0x1234, "stub");
    sim.addVirtualSlave(s);
    EXPECT_EQ(sim.virtualSlaveCount(), 3u);
}

TEST(NetworkSimulator, RegisterReadWrite_ByStationAddress)
{
    using ethercat_sim::simulation::VirtualSlave;

    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto s1 = std::make_shared<VirtualSlave>(1, 0x9A, 0x1111, "S1");
    sim.addVirtualSlave(s1);

    // Use a non-identity register to avoid changing station address
    constexpr std::uint16_t kReg = 0x0100; // ESC_DL_FWRD (arbitrary for R/W test)
    std::uint8_t wbuf[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    EXPECT_TRUE(sim.writeToSlave(1, kReg, wbuf, sizeof(wbuf)));

    std::uint8_t rbuf[4] = {0};
    EXPECT_TRUE(sim.readFromSlave(1, kReg, rbuf, sizeof(rbuf)));
    EXPECT_EQ(0, std::memcmp(wbuf, rbuf, sizeof(wbuf)));
}
