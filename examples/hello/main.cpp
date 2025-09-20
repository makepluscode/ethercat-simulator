#include <iostream>

#include "ethercat_sim/communication/ethercat_frame.h"
#include "ethercat_sim/simulation/network_simulator.h"

int main() {
    std::cout << "Hello EtherCAT Simulator" << std::endl;
    ethercat_sim::simulation::NetworkSimulator sim;
    sim.initialize();

    // Create a minimal frame and loop it back
    ethercat_sim::communication::EtherCATFrame tx;
    tx.payload = {0x88, 0xA4, 0x11, 0x22}; // dummy bytes

    sim.setLinkUp(true);
    bool sent = sim.sendFrame(tx);

    ethercat_sim::communication::EtherCATFrame rx;
    bool                                       got = sim.receiveFrame(rx);

    if (!sent || !got || rx.payload != tx.payload) {
        std::cerr << "Frame loopback failed" << std::endl;
        return 1;
    }

    return sim.runOnce();
}
