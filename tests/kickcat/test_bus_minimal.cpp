#include <gtest/gtest.h>

#include "kickcat/Bus.h"
#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

using ethercat_sim::simulation::NetworkSimulator;

TEST(KickcatBus, DetectSlaves_ReturnsOnlineCount)
{
    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->setVirtualSlaveCount(2);
    sim->setLinkUp(true);

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, [] {});
    ::kickcat::Bus bus(link);

    int32_t n = bus.detectSlaves();
    EXPECT_EQ(n, 2);
}
