#include <gtest/gtest.h>

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

#include "kickcat/Frame.h"
#include "kickcat/Link.h"
#include "kickcat/protocol.h"
#include "kickcat/DebugHelpers.h"
#include "kickcat/SocketNull.h"

using ethercat_sim::simulation::NetworkSimulator;
using ethercat_sim::simulation::VirtualSlave;

TEST(KickcatAdapter, APRW_AssignsAddresses_WkcIsOne)
{
    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->clearSlaves();
    sim->addVirtualSlave(std::make_shared<VirtualSlave>(0, 0, 0, "S1"));
    sim->addVirtualSlave(std::make_shared<VirtualSlave>(0, 0, 0, "S2"));

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, []{});

    // Build a frame with two APWR datagrams to set station addresses
    ::kickcat::Frame frame;
    uint16_t addr1 = 1001;
    uint16_t addr2 = 1002;
    frame.addDatagram(0, ::kickcat::Command::APWR, ::kickcat::createAddress(static_cast<uint16_t>(0 - 0), ::kickcat::reg::STATION_ADDR), &addr1, sizeof(addr1));
    frame.addDatagram(1, ::kickcat::Command::APWR, ::kickcat::createAddress(static_cast<uint16_t>(0 - 1), ::kickcat::reg::STATION_ADDR), &addr2, sizeof(addr2));

    link->writeThenRead(frame);

    auto [h1, _, wkc1] = frame.nextDatagram();
    ASSERT_NE(h1, nullptr);
    EXPECT_EQ(wkc1, 1);
    auto [h2, __, wkc2] = frame.nextDatagram();
    ASSERT_NE(h2, nullptr);
    EXPECT_EQ(wkc2, 1);

    // Verify by reading back via station address
    uint16_t station = 0;
    ::kickcat::sendGetRegister<uint16_t>(*link, 1001, ::kickcat::reg::STATION_ADDR, station);
    EXPECT_EQ(station, 1001);
    ::kickcat::sendGetRegister<uint16_t>(*link, 1002, ::kickcat::reg::STATION_ADDR, station);
    EXPECT_EQ(station, 1002);
}

TEST(KickcatAdapter, LRW_LogicalMemory_ReadWrite)
{
    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->setVirtualSlaveCount(1);

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, []{});

    // Write logical memory
    uint8_t wbuf[4] = {1,2,3,4};
    auto process_ok = [](::kickcat::DatagramHeader const*, uint8_t const*, uint16_t wkc){ return (wkc >= 1) ? ::kickcat::DatagramState::OK : ::kickcat::DatagramState::INVALID_WKC; };
    auto error_throw = [](::kickcat::DatagramState const&){ throw std::runtime_error("LR* failed"); };

    link->addDatagram(::kickcat::Command::LWR, /*logical*/0x10, wbuf, static_cast<uint16_t>(sizeof(wbuf)), process_ok, error_throw);
    link->processDatagrams();

    // Read back
    uint8_t rbuf[4] = {0};
    link->addDatagram(::kickcat::Command::LRD, /*logical*/0x10, nullptr, static_cast<uint16_t>(sizeof(rbuf)),
        [&rbuf](::kickcat::DatagramHeader const*, uint8_t const* data, uint16_t wkc){
            if (wkc < 1) return ::kickcat::DatagramState::INVALID_WKC;
            std::memcpy(rbuf, data, sizeof(rbuf));
            return ::kickcat::DatagramState::OK;
        }, error_throw);
    link->processDatagrams();

    EXPECT_EQ(0, std::memcmp(wbuf, rbuf, sizeof(wbuf)));
}
