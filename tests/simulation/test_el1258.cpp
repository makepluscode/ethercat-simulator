#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>

#include "ethercat_sim/simulation/network_simulator.h"
#include "ethercat_sim/simulation/slaves/el1258.h"
#include "kickcat/protocol.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::simulation::slaves::EL1258Slave;

namespace {

bool sdo_upload(NetworkSimulator& sim, uint16_t addr, uint16_t index, uint8_t subidx, uint32_t& out)
{
    uint8_t msg[64] = {0};
    auto* mbx = reinterpret_cast<::kickcat::mailbox::Header*>(msg);
    auto* coe = ::kickcat::pointData<::kickcat::CoE::Header>(mbx);
    auto* sdo = ::kickcat::pointData<::kickcat::CoE::ServiceData>(coe);
    mbx->len = 10;
    mbx->type = ::kickcat::mailbox::CoE;
    mbx->count = 1;
    coe->service = ::kickcat::CoE::SDO_REQUEST;
    sdo->command = ::kickcat::CoE::SDO::request::UPLOAD;
    sdo->index = index;
    sdo->subindex = subidx;

    if (!sim.writeToSlave(addr, 0x1000, msg, sizeof(msg))) return false;

    uint8_t rx[64] = {0};
    if (!sim.readFromSlave(addr, 0x1200, rx, sizeof(rx))) return false;

    auto* rmbx = reinterpret_cast<::kickcat::mailbox::Header*>(rx);
    auto* rcoe = ::kickcat::pointData<::kickcat::CoE::Header>(rmbx);
    auto* rsdo = ::kickcat::pointData<::kickcat::CoE::ServiceData>(rcoe);
    std::memcpy(&out, ::kickcat::pointData<uint8_t>(rsdo), sizeof(uint32_t));
    return true;
}

} // namespace

TEST(EL1258, DI_States_PowerAndButton)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto el = std::make_shared<EL1258Slave>(1);
    el->setPower(false);
    el->setPowerButton(false);
    sim.addVirtualSlave(el);

    uint32_t v = 0;

    // DI1 (AC_ON) follows power (off)
    ASSERT_TRUE(sdo_upload(sim, 1, 0x6000, 2, v));
    EXPECT_EQ(v, 0u);

    // Turn power on => DI1 == 1
    el->setPower(true);
    ASSERT_TRUE(sdo_upload(sim, 1, 0x6000, 2, v));
    EXPECT_EQ(v, 1u);

    // DI0 (POWER_BUTTON) reflects button press
    ASSERT_TRUE(sdo_upload(sim, 1, 0x6000, 1, v));
    EXPECT_EQ(v, 0u);
    el->setPowerButton(true);
    ASSERT_TRUE(sdo_upload(sim, 1, 0x6000, 1, v));
    EXPECT_EQ(v, 1u);

    // Unused inputs remain 0 (e.g., DI3)
    ASSERT_TRUE(sdo_upload(sim, 1, 0x6000, 3, v));
    EXPECT_EQ(v, 0u);
}

