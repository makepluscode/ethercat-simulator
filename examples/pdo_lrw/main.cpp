#include <cstring>
#include <iostream>

#include "kickcat/Link.h"
#include "kickcat/SocketNull.h"
#include "kickcat/protocol.h"

#include "ethercat_sim/kickcat/sim_socket.h"
#include "ethercat_sim/simulation/network_simulator.h"

int main()
{
    using ethercat_sim::simulation::NetworkSimulator;

    auto sim = std::make_shared<NetworkSimulator>();
    sim->initialize();
    sim->setLinkUp(true);
    sim->setVirtualSlaveCount(1); // 하나의 가상 슬레이브

    auto nominal = std::make_shared<ethercat_sim::kickcat::SimSocket>(sim);
    auto redun   = std::make_shared<::kickcat::SocketNull>();
    auto link    = std::make_shared<::kickcat::Link>(nominal, redun, [] {});

    // 논리 메모리 0x20 주소에 패턴을 써보고, 읽어서 검증
    uint8_t wbuf[8] = {0x10, 0x20, 0x30, 0x40, 0xAA, 0xBB, 0xCC, 0xDD};
    auto process_ok = [](::kickcat::DatagramHeader const*, uint8_t const*, uint16_t wkc)
    { return (wkc >= 1) ? ::kickcat::DatagramState::OK : ::kickcat::DatagramState::INVALID_WKC; };
    auto on_error = [](::kickcat::DatagramState const&) { throw std::runtime_error("LR* failed"); };

    // LWR: 논리 메모리에 쓰기
    link->addDatagram(::kickcat::Command::LWR, /*logical*/ 0x20, wbuf,
                      static_cast<uint16_t>(sizeof(wbuf)), process_ok, on_error);
    link->processDatagrams();

    // LRD: 논리 메모리에서 읽기
    uint8_t rbuf[8] = {0};
    link->addDatagram(
        ::kickcat::Command::LRD, /*logical*/ 0x20, nullptr, static_cast<uint16_t>(sizeof(rbuf)),
        [&rbuf](::kickcat::DatagramHeader const*, uint8_t const* data, uint16_t wkc)
        {
            if (wkc < 1)
                return ::kickcat::DatagramState::INVALID_WKC;
            std::memcpy(rbuf, data, sizeof(rbuf));
            return ::kickcat::DatagramState::OK;
        },
        on_error);
    link->processDatagrams();

    if (std::memcmp(wbuf, rbuf, sizeof(wbuf)) != 0)
    {
        std::cerr << "PDO LRW example failed" << std::endl;
        return 1;
    }

    std::cout << "PDO LRW example OK" << std::endl;
    return 0;
}
