#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>

#include "ethercat_sim/simulation/network_simulator.h"
#include "ethercat_sim/simulation/virtual_slave.h"
#include "kickcat/protocol.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::simulation::VirtualSlave;

TEST(VirtualSlave, AL_DL_Defaults_And_AL_CONTROL_ChangesState)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto s1 = std::make_shared<VirtualSlave>(1, 0x9A, 0x1111, "S1");
    sim.addVirtualSlave(s1);

    // Read AL_STATUS (0x130), expect INIT
    std::uint8_t al_status[2] = {0, 0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_status, sizeof(al_status)));
    EXPECT_EQ(al_status[0], static_cast<uint8_t>(::kickcat::State::INIT));

    // Read DL_STATUS (0x110), expect PDI_op and COM_port0 set when online
    std::uint8_t dl_status[2] = {0, 0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::ESC_DL_STATUS, dl_status, sizeof(dl_status)));
    uint16_t dl = static_cast<uint16_t>(dl_status[0] | (static_cast<uint16_t>(dl_status[1]) << 8));
    EXPECT_TRUE((dl & (1u << 0)) != 0); // PDI_op
    EXPECT_TRUE((dl & (1u << 4)) != 0); // PL_port0
    EXPECT_TRUE((dl & (1u << 9)) != 0); // COM_port0

    // Write AL_CONTROL to request PRE_OP and observe AL_STATUS change
    std::uint8_t al_ctl[2] = { static_cast<uint8_t>(::kickcat::State::PRE_OP), 0x00 };
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl, sizeof(al_ctl)));

    std::uint8_t al_status2[2] = {0, 0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_status2, sizeof(al_status2)));
    EXPECT_EQ(al_status2[0], static_cast<uint8_t>(::kickcat::State::PRE_OP));
}

