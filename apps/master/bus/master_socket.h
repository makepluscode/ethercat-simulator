#pragma once

#include <string>
#include <chrono>
#include <cstdint>

#include "kickcat/AbstractSocket.h"

namespace ethercat_sim::bus {

// MasterSocket implements Kickcat's AbstractSocket and forwards EtherCAT frames
// over a stream transport (UDS or TCP). The EtherCAT frame bytes are sent as-is
// with a 16-bit big-endian length prefix to preserve message boundaries on streams.
class MasterSocket : public ::kickcat::AbstractSocket {
public:
    explicit MasterSocket(std::string endpoint)
        : endpoint_(std::move(endpoint)) {}

    void open(std::string const& interface) override;
    void setTimeout(std::chrono::nanoseconds timeout) override;
    void close() noexcept override;
    int32_t read(uint8_t* frame, int32_t frame_size) override;
    int32_t write(uint8_t const* frame, int32_t frame_size) override;

private:
    int fd_ {-1};
    std::string endpoint_;
    std::chrono::nanoseconds timeout_ {std::chrono::milliseconds(2)};

    bool connectUDS_(const std::string& path);
    bool connectTCP_(const std::string& host, uint16_t port);
    bool writeAll_(const void* data, size_t len);
    bool readAll_(void* data, size_t len);
};

} // namespace ethercat_sim::bus

