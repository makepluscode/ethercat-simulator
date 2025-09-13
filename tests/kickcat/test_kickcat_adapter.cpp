#include <gtest/gtest.h>

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

#include "kickcat/DebugHelpers.h"
#include "kickcat/Link.h"
#include "kickcat/protocol.h"
#include "kickcat/SocketNull.h"

using namespace ethercat_sim;

TEST(KickcatAdapter, ReadWrite_StationAddress)
{
    auto sim = std::make_shared<simulation::NetworkSimulator>();
    sim->initialize();
    sim->clearSlaves();
    sim->setVirtualSlaveCount(0);

    // Add one virtual slave with address 1
    auto s1 = std::make_shared<simulation::VirtualSlave>(1, 0x9A, 0x1111, "S1");
    sim->addVirtualSlave(s1);

    auto nominal = std::make_shared<kickcat_adapter::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, []{});

    uint16_t station = 0;
    ::kickcat::sendGetRegister(*link, /*slave*/1, ::kickcat::reg::STATION_ADDR, station);
    EXPECT_EQ(station, 1u);

    // Write a new station address and read it back at the new address
    ::kickcat::sendWriteRegister<uint16_t>(*link, /*slave*/1, ::kickcat::reg::STATION_ADDR, 2u);
    station = 0;
    ::kickcat::sendGetRegister(*link, /*slave*/2, ::kickcat::reg::STATION_ADDR, station);
    EXPECT_EQ(station, 2u);
}
