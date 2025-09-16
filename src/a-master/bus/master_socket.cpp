#include "bus/master_socket.h"

#include <cstring>
#include <string_view>
#include <stdexcept>
#include <system_error>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace ethercat_sim::bus {

static bool parse_tcp_endpoint(std::string const& ep, std::string& host, uint16_t& port)
{
    // ep format: tcp://host:port
    std::string_view v{ep};
    constexpr std::string_view tcp = "tcp://";
    if (v.rfind(tcp, 0) != 0) return false;
    v.remove_prefix(tcp.size());
    auto colon = v.find(':');
    if (colon == std::string_view::npos) return false;
    host.assign(v.substr(0, colon));
    auto pstr = v.substr(colon + 1);
    int p = std::stoi(std::string(pstr));
    if (p < 1 || p > 65535) return false;
    port = static_cast<uint16_t>(p);
    return true;
}

static bool parse_uds_endpoint(std::string const& ep, std::string& path)
{
    // ep format: uds:///tmp/ethercat_bus.sock or uds://@abstract
    std::string_view v{ep};
    constexpr std::string_view uds = "uds://";
    if (v.rfind(uds, 0) != 0) return false;
    v.remove_prefix(uds.size());
    path.assign(v);
    return true;
}

void MasterSocket::open(std::string const& /*interface*/)
{
    if (fd_ != -1) return;
    std::string path, host; uint16_t port = 0;
    if (parse_uds_endpoint(endpoint_, path)) {
        if (!connectUDS_(path)) {
            throw std::runtime_error("MasterSocket: UDS connect failed");
        }
    }
    else if (parse_tcp_endpoint(endpoint_, host, port)) {
        if (!connectTCP_(host, port)) {
            throw std::runtime_error("MasterSocket: TCP connect failed");
        }
    }
    else {
        throw std::invalid_argument("MasterSocket: unsupported endpoint (use uds:// or tcp://)");
    }
}

void MasterSocket::setTimeout(std::chrono::nanoseconds timeout)
{
    timeout_ = timeout;
    if (fd_ == -1) return;
    timeval tv{};
    tv.tv_sec = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(timeout_).count());
    tv.tv_usec = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(timeout_ % std::chrono::seconds(1)).count());
    setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

void MasterSocket::close() noexcept
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

int32_t MasterSocket::write(uint8_t const* frame, int32_t frame_size)
{
    if (fd_ == -1) return -1;
    uint16_t be_len = htons(static_cast<uint16_t>(frame_size));
    if (!writeAll_(&be_len, sizeof(be_len))) return -1;
    if (!writeAll_(frame, static_cast<size_t>(frame_size))) return -1;
    return frame_size;
}

int32_t MasterSocket::read(uint8_t* frame, int32_t frame_size)
{
    if (fd_ == -1) return -1;
    uint16_t be_len = 0;
    if (!readAll_(&be_len, sizeof(be_len))) return 0; // timeout -> 0
    uint16_t len = ntohs(be_len);
    if (len > static_cast<uint16_t>(frame_size)) {
        // read and drop
        std::string tmp(len, '\0');
        readAll_(tmp.data(), len);
        return -1;
    }
    if (!readAll_(frame, len)) return 0;
    return static_cast<int32_t>(len);
}

bool MasterSocket::connectUDS_(const std::string& path)
{
    fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0) return false;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    socklen_t len = 0;
    if (!path.empty() && path[0] == '@') {
        // abstract namespace: leading NUL, compute length explicitly
        addr.sun_path[0] = '\0';
        std::strncpy(addr.sun_path + 1, path.c_str() + 1, sizeof(addr.sun_path) - 2);
        size_t n = std::min(std::strlen(path.c_str() + 1), sizeof(addr.sun_path) - 2);
        len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + 1 + n);
    } else {
        std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
        len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + std::strlen(addr.sun_path));
    }
    if (::connect(fd_, reinterpret_cast<sockaddr*>(&addr), len) < 0) {
        ::close(fd_); fd_ = -1; return false;
    }
    setTimeout(timeout_);
    return true;
}

bool MasterSocket::connectTCP_(const std::string& host, uint16_t port)
{
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) return false;
    sockaddr_in sa{}; sa.sin_family = AF_INET; sa.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &sa.sin_addr) != 1) {
        ::close(fd_); fd_ = -1; return false;
    }
    if (::connect(fd_, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0) {
        ::close(fd_); fd_ = -1; return false;
    }
    setTimeout(timeout_);
    return true;
}

bool MasterSocket::writeAll_(const void* data, size_t len)
{
    const uint8_t* p = static_cast<const uint8_t*>(data);
    size_t left = len;
    while (left > 0) {
        ssize_t n = ::send(fd_, p, left, 0);
        if (n <= 0) return false;
        p += n; left -= static_cast<size_t>(n);
    }
    return true;
}

bool MasterSocket::readAll_(void* data, size_t len)
{
    uint8_t* p = static_cast<uint8_t*>(data);
    size_t left = len;
    while (left > 0) {
        ssize_t n = ::recv(fd_, p, left, 0);
        if (n <= 0) return false;
        p += n; left -= static_cast<size_t>(n);
    }
    return true;
}

} // namespace ethercat_sim::bus
