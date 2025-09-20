#include <cstdint>
#include <gtest/gtest.h>

#include "kickcat/protocol.h"

#include "ethercat_sim/simulation/network_simulator.h"
#include "ethercat_sim/simulation/virtual_slave.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::simulation::VirtualSlave;

namespace
{

void read16(NetworkSimulator& sim, uint16_t station, uint16_t reg, uint16_t& out)
{
    std::uint8_t buf[2] = {0, 0};
    ASSERT_TRUE(sim.readFromSlave(station, reg, buf, sizeof(buf)));
    out = static_cast<uint16_t>(buf[0] | (static_cast<uint16_t>(buf[1]) << 8));
}

void write8(NetworkSimulator& sim, uint16_t station, uint16_t reg, uint8_t val)
{
    std::uint8_t buf[2] = {val, 0x00};
    ASSERT_TRUE(sim.writeToSlave(station, reg, buf, sizeof(buf)));
}

} // namespace

TEST(VirtualSlaveAL, TransitionGating_StatusCodes)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto s1 = std::make_shared<VirtualSlave>(1, 0x9A, 0x1111, "S1");
    sim.addVirtualSlave(s1);

    // Start at INIT
    uint16_t al_status = 0;
    read16(sim, 1, ::kickcat::reg::AL_STATUS, al_status);
    EXPECT_EQ(al_status & 0x000F, static_cast<uint8_t>(::kickcat::State::INIT));

    // With no input PDO mapping, SAFE_OP must fail with 0x0011 and remain in PRE_OP
    s1->setInputPDOMapped(false);
    write8(sim, 1, ::kickcat::reg::AL_CONTROL, static_cast<uint8_t>(::kickcat::State::SAFE_OP));
    read16(sim, 1, ::kickcat::reg::AL_STATUS, al_status);
    EXPECT_EQ(al_status & 0x000F, static_cast<uint8_t>(::kickcat::State::PRE_OP));
    EXPECT_NE(al_status & static_cast<uint16_t>(::kickcat::State::ACK), 0u);
    uint16_t al_code = 0;
    read16(sim, 1, ::kickcat::reg::AL_STATUS_CODE, al_code);
    EXPECT_EQ(al_code, 0x0011);

    // Acknowledge the error to clear ACK flag and status code
    write8(sim, 1, ::kickcat::reg::AL_CONTROL,
           static_cast<uint8_t>(::kickcat::State::INIT) |
               static_cast<uint8_t>(::kickcat::State::ACK));
    read16(sim, 1, ::kickcat::reg::AL_STATUS, al_status);
    EXPECT_EQ(al_status & 0x000F, static_cast<uint8_t>(::kickcat::State::PRE_OP));
    EXPECT_EQ(al_status & static_cast<uint16_t>(::kickcat::State::ACK), 0u);
    read16(sim, 1, ::kickcat::reg::AL_STATUS_CODE, al_code);
    EXPECT_EQ(al_code, 0x0000);

    // OPERATIONAL must also fail with 0x0011 and remain in PRE_OP
    write8(sim, 1, ::kickcat::reg::AL_CONTROL, static_cast<uint8_t>(::kickcat::State::OPERATIONAL));
    read16(sim, 1, ::kickcat::reg::AL_STATUS, al_status);
    EXPECT_EQ(al_status & 0x000F, static_cast<uint8_t>(::kickcat::State::PRE_OP));
    EXPECT_NE(al_status & static_cast<uint16_t>(::kickcat::State::ACK), 0u);
    read16(sim, 1, ::kickcat::reg::AL_STATUS_CODE, al_code);
    EXPECT_EQ(al_code, 0x0011);

    // Clear ACK again before enabling PDO mapping
    write8(sim, 1, ::kickcat::reg::AL_CONTROL,
           static_cast<uint8_t>(::kickcat::State::INIT) |
               static_cast<uint8_t>(::kickcat::State::ACK));
    read16(sim, 1, ::kickcat::reg::AL_STATUS, al_status);
    EXPECT_EQ(al_status & 0x000F, static_cast<uint8_t>(::kickcat::State::PRE_OP));
    EXPECT_EQ(al_status & static_cast<uint16_t>(::kickcat::State::ACK), 0u);
    read16(sim, 1, ::kickcat::reg::AL_STATUS_CODE, al_code);
    EXPECT_EQ(al_code, 0x0000);

    // When mapping is ready, SAFE_OP succeeds and clears status code
    s1->setInputPDOMapped(true);
    write8(sim, 1, ::kickcat::reg::AL_CONTROL, static_cast<uint8_t>(::kickcat::State::SAFE_OP));
    read16(sim, 1, ::kickcat::reg::AL_STATUS, al_status);
    EXPECT_EQ(al_status & 0x000F, static_cast<uint8_t>(::kickcat::State::SAFE_OP));
    EXPECT_EQ(al_status & static_cast<uint16_t>(::kickcat::State::ACK), 0u);
    read16(sim, 1, ::kickcat::reg::AL_STATUS_CODE, al_code);
    EXPECT_EQ(al_code, 0x0000);
}
