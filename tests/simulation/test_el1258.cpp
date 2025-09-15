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

bool sdo_download_u32(NetworkSimulator& sim, uint16_t addr, uint16_t index, uint8_t subidx, uint32_t value)
{
    uint8_t msg[64] = {0};
    auto* mbx = reinterpret_cast<::kickcat::mailbox::Header*>(msg);
    auto* coe = ::kickcat::pointData<::kickcat::CoE::Header>(mbx);
    auto* sdo = ::kickcat::pointData<::kickcat::CoE::ServiceData>(coe);
    mbx->len = 10;
    mbx->type = ::kickcat::mailbox::CoE;
    mbx->count = 1;
    coe->service = ::kickcat::CoE::SDO_REQUEST;
    sdo->command = ::kickcat::CoE::SDO::request::DOWNLOAD;
    sdo->index = index;
    sdo->subindex = subidx;
    // expedited write of 4 bytes
    uint8_t* payload = ::kickcat::pointData<uint8_t>(sdo);
    std::memcpy(payload, &value, sizeof(uint32_t));

    if (!sim.writeToSlave(addr, 0x1000, msg, sizeof(msg))) return false;

    // Read back ack (ignore content)
    uint8_t rx[64] = {0};
    if (!sim.readFromSlave(addr, 0x1200, rx, sizeof(rx))) return false;
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

TEST(EL1258, PDO_Mapping_To_LogicalMemory)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto el = std::make_shared<EL1258Slave>(1);
    el->setPower(false);
    el->setPowerButton(false);
    sim.addVirtualSlave(el);

    // Map DI bitfield to logical address 0x0000, 1 byte
    sim.mapDigitalInputs(el, /*logical*/0x0000, /*width_bytes*/1);

    // Initial: all zero
    sim.runOnce();
    uint8_t byte = 0xFF;
    ASSERT_TRUE(sim.readLogical(0x0000, &byte, 1));
    EXPECT_EQ(byte, 0x00);

    // Power on => DI1 set
    el->setPower(true);
    sim.runOnce();
    ASSERT_TRUE(sim.readLogical(0x0000, &byte, 1));
    EXPECT_EQ(byte & 0x02, 0x02);

    // Power button => DI0 set
    el->setPowerButton(true);
    sim.runOnce();
    ASSERT_TRUE(sim.readLogical(0x0000, &byte, 1));
    EXPECT_EQ(byte & 0x01, 0x01);
}

TEST(EL1258, AL_Transition_Gating_By_PDO_Mapping)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto el = std::make_shared<EL1258Slave>(1);
    sim.addVirtualSlave(el);

    // Initially INIT
    uint8_t al_status[2] = {0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_status, sizeof(al_status)));
    EXPECT_EQ(al_status[0], static_cast<uint8_t>(::kickcat::State::INIT));

    // Try SAFE_OP without PDO mapping -> denied
    uint8_t al_ctl_safe[2] = { static_cast<uint8_t>(::kickcat::State::SAFE_OP), 0x00 };
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl_safe, sizeof(al_ctl_safe)));
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_status, sizeof(al_status)));
    EXPECT_EQ(al_status[0], static_cast<uint8_t>(::kickcat::State::INIT));
    uint8_t al_code[2] = {0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS_CODE, al_code, sizeof(al_code)));
    EXPECT_NE(static_cast<uint16_t>(al_code[0] | (al_code[1] << 8)), 0u);

    // Map inputs and retry SAFE_OP -> allowed
    sim.mapDigitalInputs(el, 0x0000, 1);
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl_safe, sizeof(al_ctl_safe)));
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_status, sizeof(al_status)));
    EXPECT_EQ(al_status[0], static_cast<uint8_t>(::kickcat::State::SAFE_OP));

    // OP should also be allowed now
    uint8_t al_ctl_op[2] = { static_cast<uint8_t>(::kickcat::State::OPERATIONAL), 0x00 };
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl_op, sizeof(al_ctl_op)));
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_status, sizeof(al_status)));
    EXPECT_EQ(al_status[0], static_cast<uint8_t>(::kickcat::State::OPERATIONAL));
}

TEST(EL1258, CoE_TxPDO_Mapping_Enables_SafeOp)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto el = std::make_shared<EL1258Slave>(1);
    sim.addVirtualSlave(el);

    // Before mapping, SAFE_OP denied
    uint8_t al_ctl_safe[2] = { static_cast<uint8_t>(::kickcat::State::SAFE_OP), 0x00 };
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl_safe, sizeof(al_ctl_safe)));
    uint8_t al_st[2] = {0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_st, sizeof(al_st)));
    EXPECT_EQ(al_st[0], static_cast<uint8_t>(::kickcat::State::INIT));

    // Map 0x1A00 with a single entry (e.g., aggregate at 0x6002:00, 8 bits)
    // value format: 0xIIII SS BB => index(16) | sub(8) | size_bits(8)
    uint32_t entry = (0x6002u << 16) | (0x00u << 8) | 8u;
    ASSERT_TRUE(sdo_download_u32(sim, 1, 0x1A00, 0, 1));
    ASSERT_TRUE(sdo_download_u32(sim, 1, 0x1A00, 1, entry));
    // Assign 0x1A00 into 0x1C13
    ASSERT_TRUE(sdo_download_u32(sim, 1, 0x1C13, 0, 1));
    ASSERT_TRUE(sdo_download_u32(sim, 1, 0x1C13, 1, 0x1A00));

    // Retry SAFE_OP -> should pass
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl_safe, sizeof(al_ctl_safe)));
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_st, sizeof(al_st)));
    EXPECT_EQ(al_st[0], static_cast<uint8_t>(::kickcat::State::SAFE_OP));
}

TEST(EL1258, DefaultTxPdoMapping_Enables_SafeOp)
{
    NetworkSimulator sim;
    sim.initialize();
    sim.clearSlaves();

    auto el = std::make_shared<EL1258Slave>(1);
    // Apply built-in default mapping
    el->applyDefaultTxPdoMapping();
    sim.addVirtualSlave(el);

    // SAFE_OP should be allowed now
    uint8_t al_ctl_safe[2] = { static_cast<uint8_t>(::kickcat::State::SAFE_OP), 0x00 };
    ASSERT_TRUE(sim.writeToSlave(1, ::kickcat::reg::AL_CONTROL, al_ctl_safe, sizeof(al_ctl_safe)));
    uint8_t al_st[2] = {0};
    ASSERT_TRUE(sim.readFromSlave(1, ::kickcat::reg::AL_STATUS, al_st, sizeof(al_st)));
    EXPECT_EQ(al_st[0], static_cast<uint8_t>(::kickcat::State::SAFE_OP));
}
