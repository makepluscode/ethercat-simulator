#include <gtest/gtest.h>

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

#include "kickcat/Bus.h"
#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"

using ethercat_sim::simulation::NetworkSimulator;

TEST(KickcatBus, Init_MinimalFlow)
{
    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->setLinkUp(true);
    sim->setVirtualSlaveCount(2);

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, []{});

    ::kickcat::Bus bus(link);

    // Full minimal init should not throw with our stubs
    EXPECT_NO_THROW({ bus.init(std::chrono::milliseconds(0)); });
}
