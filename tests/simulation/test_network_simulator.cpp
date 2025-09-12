#include <gtest/gtest.h>

#include "ethercat_sim/simulation/network_simulator.h"
#include "ethercat_sim/communication/ethercat_frame.h"

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

