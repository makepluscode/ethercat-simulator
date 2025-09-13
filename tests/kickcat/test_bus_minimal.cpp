#include <gtest/gtest.h>

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

#include "kickcat/Link.h"
#include "kickcat/Bus.h"
#include "kickcat/SocketNull.h"

using namespace ethercat_sim;

TEST(KickcatBus, DetectSlaves_ReturnsOnlineCount)
{
    auto sim = std::make_shared<simulation::NetworkSimulator>();
    sim->initialize();
    sim->setVirtualSlaveCount(2);
    sim->setLinkUp(true);

    auto nominal = std::make_shared<kickcat_adapter::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, []{});
    ::kickcat::Bus bus(link);

    int32_t n = bus.detectSlaves();
    EXPECT_EQ(n, 2);
}
