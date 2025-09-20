#include "bus/slaves_endpoint.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <system_error>
#include <thread>
#include <unistd.h>

#include "framework/logger/logger.h"

#include "kickcat/Frame.h"
#include "kickcat/protocol.h"

#include "ethercat_sim/communication/endpoint_parser.h"
#include "sim/el1258_subs.h"

namespace ethercat_sim::bus
{

bool SlavesEndpoint::bindUDS_(const std::string& path)
{
    listen_fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd_ < 0)
    {
        ethercat_sim::framework::logger::Logger::error("Failed to create socket: %s", strerror(errno));
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family       = AF_UNIX;
    std::string disk_path = path;
    socklen_t len         = 0;
    if (!path.empty() && path[0] == '@')
    {
        addr.sun_path[0] = '\0';
        std::strncpy(addr.sun_path + 1, path.c_str() + 1, sizeof(addr.sun_path) - 2);
        uds_is_abstract_ = true;
        size_t n         = std::min(std::strlen(path.c_str() + 1), sizeof(addr.sun_path) - 2);
        len              = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + 1 + n);
    }
    else
    {
        // unlink existing path to avoid EADDRINUSE
        ::unlink(disk_path.c_str());
        std::strncpy(addr.sun_path, disk_path.c_str(), sizeof(addr.sun_path) - 1);
        bound_disk_path_ = disk_path;
        len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + std::strlen(addr.sun_path));
    }
    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), len) < 0)
    {
        ethercat_sim::framework::logger::Logger::error("Failed to bind socket: %s", strerror(errno));
        ::close(listen_fd_);
        return false;
    }
    if (::listen(listen_fd_, 1) < 0)
    {
        ethercat_sim::framework::logger::Logger::error("Failed to listen on socket: %s", strerror(errno));
        ::close(listen_fd_);
        return false;
    }
    ethercat_sim::framework::logger::Logger::info("Socket bound and listening, fd=%d", listen_fd_);
    return true;
}

bool SlavesEndpoint::bindTCP_(const std::string& host, uint16_t port)
{
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0)
        return false;
    int yes = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &sa.sin_addr) != 1)
        return false;
    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0)
        return false;
    if (::listen(listen_fd_, 1) < 0)
        return false;
    return true;
}

static bool read_exact_nb(int fd, void* buf, size_t len, std::atomic_bool* stop)
{
    uint8_t* p  = static_cast<uint8_t*>(buf);
    size_t left = len;
    while (left > 0)
    {
        struct pollfd pfd
        {
            fd, POLLIN, 0
        };
        int pr = ::poll(&pfd, 1, 200);
        if (pr < 0)
            return false;
        if (pr == 0)
        {
            if (stop && stop->load())
                return false;
            continue;
        }
        if (!(pfd.revents & POLLIN))
        {
            if (stop && stop->load())
                return false;
            continue;
        }
        ssize_t n = ::recv(fd, p, left, 0);
        if (n <= 0)
            return false;
        p += n;
        left -= static_cast<size_t>(n);
    }
    return true;
}

static bool write_exact_nb(int fd, const void* buf, size_t len, std::atomic_bool* stop)
{
    const uint8_t* p = static_cast<const uint8_t*>(buf);
    size_t left      = len;
    while (left > 0)
    {
        struct pollfd pfd
        {
            fd, POLLOUT, 0
        };
        int pr = ::poll(&pfd, 1, 200);
        if (pr < 0)
            return false;
        if (pr == 0)
        {
            if (stop && stop->load())
                return false;
            continue;
        }
        if (!(pfd.revents & POLLOUT))
        {
            if (stop && stop->load())
                return false;
            continue;
        }
        ssize_t n = ::send(fd, p, left, 0);
        if (n <= 0)
            return false;
        p += n;
        left -= static_cast<size_t>(n);
    }
    return true;
}

bool SlavesEndpoint::handleClient_(int fd)
{
    // Prepare simulator with N slaves
    sim_->initialize("");
    sim_->clearSlaves();
    // Create N EL1258-like virtual slaves with station addresses starting at 1
    for (std::size_t i = 0; i < slaves_count_; ++i)
    {
        auto s = std::make_shared<ethercat_sim::subs::El1258Slave>(static_cast<uint16_t>(1 + i),
                                                                   0x00000000, 0x00001258);
        sim_->addVirtualSlave(std::move(s));
    }
    sim_->setLinkUp(true);

    // blocking loop: read len(uint16), read frame, process, write back
    while (true)
    {
        if (stop_ && stop_->load())
            return true;
        uint16_t be_len = 0;
        if (!read_exact_nb(fd, &be_len, sizeof(be_len), stop_))
        {
            if (stop_ && stop_->load())
                return true; // graceful stop
            return false;
        }
        uint16_t len = ntohs(be_len);
        if (len == 0 || len > 1500)
        {
            return false; // invalid
        }
        std::vector<uint8_t> buf(len);
        if (!read_exact_nb(fd, buf.data(), len, stop_))
        {
            if (stop_ && stop_->load())
                return true;
            else
                return false;
        }
        processFrame_(buf.data(), static_cast<int32_t>(len));
        uint16_t out_len = htons(static_cast<uint16_t>(buf.size()));
        if (!write_exact_nb(fd, &out_len, sizeof(out_len), stop_))
            return stop_ && stop_->load();
        if (!write_exact_nb(fd, buf.data(), buf.size(), stop_))
            return stop_ && stop_->load();
    }
}

void SlavesEndpoint::processFrame_(uint8_t* frame, int32_t frame_size)
{
    ::kickcat::Frame f(frame, frame_size);
    static bool dbg = []
    {
        const char* e = std::getenv("EC_DEBUG");
        return e && *e;
    }();

    // Ensure the ethernet header has the correct EtherCAT type
    if (f.ethernet()->type != ::kickcat::ETH_ETHERCAT_TYPE)
    {
        f.ethernet()->type = ::kickcat::ETH_ETHERCAT_TYPE;
    }

    while (true)
    {
        auto [hdr, data, wkc] = f.peekDatagram();
        if (hdr == nullptr)
            break;

        uint16_t ack = 0;
        switch (hdr->command)
        {
        case ::kickcat::Command::NOP:
        {
            ack = 0;
            if (dbg)
            {
                ethercat_sim::framework::logger::Logger::debug("NOP");
            }
            break;
        }
        case ::kickcat::Command::BRD:
        {
            auto n = static_cast<uint16_t>(sim_->onlineSlaveCount());
            ack    = (n == 0) ? 1 : n; // ensure >0 to help initial detection
            if (dbg)
            {
                ethercat_sim::framework::logger::Logger::debug("BRD len=%d wkc->%d online=%d", hdr->len, ack, sim_->onlineSlaveCount());
            }
            break;
        }
        case ::kickcat::Command::BWR:
        case ::kickcat::Command::BRW:
        {
            auto [pos, ado] = ::kickcat::extractAddress(hdr->address);
            (void) pos;
            uint16_t okc       = 0;
            std::size_t online = sim_->onlineSlaveCount();
            for (std::size_t idx = 0; idx < online; ++idx)
            {
                if (sim_->writeToSlaveByIndex(idx, ado, data, hdr->len))
                {
                    ++okc;
                }
            }
            ack = okc;
            if (dbg)
            {
                uint16_t value = 0;
                if (hdr->len >= 1)
                {
                    value = data[0];
                    if (hdr->len >= 2)
                    {
                        value |= static_cast<uint16_t>(data[1]) << 8;
                    }
                }
                std::cerr << "[a-slaves][DBG] BWR/BRW ado=0x" << std::hex << ado << std::dec
                          << " len=" << hdr->len << " value=0x" << std::hex << value << std::dec
                          << " wkc=" << ack << std::endl;
            }
            break;
        }
        case ::kickcat::Command::FPRD:
        case ::kickcat::Command::FPWR:
        case ::kickcat::Command::FPRW:
        {
            auto [adp, ado] = ::kickcat::extractAddress(hdr->address);
            bool ok         = false;
            bool is_read    = (hdr->command == ::kickcat::Command::FPRD);
            if (is_read)
                ok = sim_->readFromSlave(adp, ado, data, hdr->len);
            else
                ok = sim_->writeToSlave(adp, ado, data, hdr->len);
            ack = ok ? 1 : 0;
            if (dbg)
            {
                std::cerr << "[a-slaves][DBG] "
                          << (hdr->command == ::kickcat::Command::FPRD
                                  ? "FPRD"
                                  : (hdr->command == ::kickcat::Command::FPWR ? "FPWR" : "FPRW"))
                          << " adp=" << adp << " ado=0x" << std::hex << ado << std::dec
                          << " ok=" << ok << " wkc=" << ack;
                if (ok && is_read && ado == ::kickcat::reg::AL_STATUS && hdr->len >= 1)
                {
                    std::cerr << " value=0x" << std::hex << static_cast<int>(data[0]) << std::dec;
                }
                std::cerr << std::endl;
            }
            break;
        }
        case ::kickcat::Command::APRD:
        case ::kickcat::Command::APWR:
        case ::kickcat::Command::APRW:
        {
            auto [pos, ado] = ::kickcat::extractAddress(hdr->address);
            std::size_t idx = (pos & 0x8000)
                                  ? static_cast<std::size_t>(static_cast<uint16_t>(0 - pos))
                                  : static_cast<std::size_t>(pos);
            bool ok         = false;
            if (hdr->command == ::kickcat::Command::APRD)
                ok = sim_->readFromSlaveByIndex(idx, ado, data, hdr->len);
            else
                ok = sim_->writeToSlaveByIndex(idx, ado, data, hdr->len);
            ack = ok ? 1 : 0;
            if (dbg)
            {
                std::cerr << "[a-slaves][DBG] APRD/APWR/APRW idx=" << idx << " ado=0x" << std::hex
                          << ado << std::dec << " ok=" << ok << " wkc=" << ack << std::endl;
            }
            break;
        }
        case ::kickcat::Command::LRD:
        case ::kickcat::Command::LWR:
        case ::kickcat::Command::LRW:
        {
            uint32_t logical = hdr->address;
            bool ok          = false;
            if (hdr->command == ::kickcat::Command::LRD)
                ok = sim_->readLogical(logical, data, hdr->len);
            else
                ok = sim_->writeLogical(logical, data, hdr->len);
            ack = ok ? 1 : 0;
            if (dbg)
            {
                std::cerr << "[a-slaves][DBG] LRD/LWR/LRW logical=0x" << std::hex << logical
                          << std::dec << " len=" << hdr->len << " ok=" << ok << " wkc=" << ack
                          << std::endl;
            }
            break;
        }
        case ::kickcat::Command::ARMW:
        case ::kickcat::Command::FRMW:
        {
            auto n = static_cast<uint16_t>(sim_->onlineSlaveCount());
            ack    = (n == 0) ? 1 : n;
            if (dbg)
            {
                std::cerr << "[a-slaves][DBG] ARMW/FRMW wkc->" << ack << std::endl;
            }
            break;
        }
        default:
        {
            ack = (sim_->onlineSlaveCount() > 0) ? 1 : 0;
            if (dbg)
            {
                std::cerr << "[a-slaves][DBG] OTHER cmd=" << (int) hdr->command << " wkc=" << ack
                          << std::endl;
            }
            break;
        }
        }
        if (wkc)
        {
            *wkc = ack;
        }
    }

    // Copy the mutated frame (including updated WKC) back to the output buffer
    std::memcpy(frame, f.data(), static_cast<std::size_t>(frame_size));
}

bool SlavesEndpoint::run()
{
    ethercat_sim::framework::logger::Logger::info("SlavesEndpoint::run() started with endpoint: %s", endpoint_.c_str());
    std::string path, host;
    uint16_t port = 0;
    if (communication::EndpointParser::parseUdsEndpoint(endpoint_, path))
    {
        ethercat_sim::framework::logger::Logger::info("Parsed UDS endpoint, path: %s", path.c_str());
        if (!bindUDS_(path))
        {
            ethercat_sim::framework::logger::Logger::error("Failed to bind UDS at %s", path.c_str());
            return false;
        }
        ethercat_sim::framework::logger::Logger::info("Successfully bound and listening UDS %s", path.c_str());
    }
    else if (communication::EndpointParser::parseTcpEndpoint(endpoint_, host, port))
    {
        ethercat_sim::framework::logger::Logger::info("Parsed TCP endpoint, host: %s, port: %d", host.c_str(), port);
        if (!bindTCP_(host, port))
        {
            ethercat_sim::framework::logger::Logger::error("Failed to bind TCP at %s:%d", host.c_str(), port);
            return false;
        }
        ethercat_sim::framework::logger::Logger::info("Successfully bound and listening TCP %s:%d", host.c_str(), port);
    }
    else
    {
        ethercat_sim::framework::logger::Logger::error("Unsupported endpoint: %s", endpoint_.c_str());
        return false;
    }

    // Accept loop with poll to allow graceful stop
    ethercat_sim::framework::logger::Logger::info("Entering accept loop, waiting for connections...");
    int fd = -1;
    while (true)
    {
        if (stop_ && stop_->load())
        {
            ethercat_sim::framework::logger::Logger::info("Stop requested, exiting accept loop");
            fd = -1;
            break;
        }
        struct pollfd pfd
        {
            listen_fd_, POLLIN, 0
        };
        int pr = ::poll(&pfd, 1, 200);
        if (pr < 0)
        {
            ethercat_sim::framework::logger::Logger::error("Poll error: %s", strerror(errno));
            fd = -1;
            break;
        }
        if (pr == 0)
            continue;
        if (pfd.revents & POLLIN)
        {
            sockaddr_storage peer{};
            socklen_t plen = sizeof(peer);
            fd             = ::accept(listen_fd_, reinterpret_cast<sockaddr*>(&peer), &plen);
            ethercat_sim::framework::logger::Logger::info("Accept returned fd=%d", fd);
            break;
        }
    }
    if (stop_ && stop_->load())
    {
        ethercat_sim::framework::logger::Logger::info("Stop requested before client connect");
        if (listen_fd_ != -1)
            ::close(listen_fd_);
        if (!uds_is_abstract_ && !bound_disk_path_.empty())
            ::unlink(bound_disk_path_.c_str());
        return true;
    }
    if (fd < 0)
    {
        if (listen_fd_ != -1)
            ::close(listen_fd_);
        if (!uds_is_abstract_ && !bound_disk_path_.empty())
            ::unlink(bound_disk_path_.c_str());
        return false;
    }
    ethercat_sim::framework::logger::Logger::info("Client connected");
    if (on_connection_)
        on_connection_(true);
    bool ok = handleClient_(fd);
    ::close(fd);
    ::close(listen_fd_);
    if (!uds_is_abstract_ && !bound_disk_path_.empty())
        ::unlink(bound_disk_path_.c_str());
    if (on_connection_)
        on_connection_(false);
    return ok;
}

} // namespace ethercat_sim::bus
