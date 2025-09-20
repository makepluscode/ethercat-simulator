#include <iostream>
#include <memory>

#include "kickcat/Bus.h"
#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

int main() {
    using ethercat_sim::simulation::NetworkSimulator;

    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->setLinkUp(true);
    sim->setVirtualSlaveCount(2); // simulate two slaves on the bus

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();

    // Minimal link and bus setup
    auto link = std::make_shared<::kickcat::Link>(nominal, redun, [] {});
    link->setTimeout(std::chrono::milliseconds(2));

    ::kickcat::Bus bus(link);

    // Try a simple detect; our simulator is a loopback stub, so expect 0
    int32_t count = 0;
    try {
        count = bus.detectSlaves();
    } catch (...) {
        // Incomplete simulation may throw; treat as 0 detected
        count = 0;
    }

    std::cout << "KickCAT linked. Detected slaves: " << count << std::endl;
    return 0;
}
