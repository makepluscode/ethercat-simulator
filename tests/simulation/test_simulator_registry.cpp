#include <cstring>
#include <gtest/gtest.h>

#include "ethercat_sim/simulation/network_simulator.h"
#include "ethercat_sim/simulation/virtual_slave.h"

using ethercat_sim::simulation::NetworkSimulator;

TEST(NetworkSimulatorRegistry, WriteRead_ByAddress_AffectsOnlyTarget)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();
    sim.setVirtualSlaveCount(3); // addresses 1..3

    constexpr uint16_t REG = 0x0100;
    uint8_t wbuf[4]        = {0x11, 0x22, 0x33, 0x44};
    ASSERT_TRUE(sim.writeToSlave(2, REG, wbuf, sizeof(wbuf)));

    uint8_t r1[4] = {0}, r2[4] = {0}, r3[4] = {0};
    ASSERT_TRUE(sim.readFromSlave(1, REG, r1, sizeof(r1)));
    ASSERT_TRUE(sim.readFromSlave(2, REG, r2, sizeof(r2)));
    ASSERT_TRUE(sim.readFromSlave(3, REG, r3, sizeof(r3)));

    EXPECT_NE(0, std::memcmp(r1, wbuf, sizeof(wbuf)));
    EXPECT_EQ(0, std::memcmp(r2, wbuf, sizeof(wbuf)));
    EXPECT_NE(0, std::memcmp(r3, wbuf, sizeof(wbuf)));
}

TEST(NetworkSimulatorRegistry, WriteRead_ByIndex_AliasesToStationAddress)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();
    sim.setVirtualSlaveCount(3); // index:0->addr1, 1->addr2, 2->addr3

    constexpr uint16_t REG = 0x0102;
    uint8_t wbuf[2]        = {0xAB, 0xCD};
    ASSERT_TRUE(sim.writeToSlaveByIndex(1, REG, wbuf, sizeof(wbuf))); // idx1 -> addr2

    uint8_t r2[2] = {0};
    ASSERT_TRUE(sim.readFromSlave(2, REG, r2, sizeof(r2)));
    EXPECT_EQ(0, std::memcmp(r2, wbuf, sizeof(wbuf)));
}
