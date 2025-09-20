#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ethercat_sim::communication {

class EndpointParser {
  public:
    // Parse TCP endpoint format: tcp://host:port
    static bool parseTcpEndpoint(const std::string& ep, std::string& host, uint16_t& port);

    // Parse UDS endpoint format: uds:///path/to/socket or uds://@abstract
    static bool parseUdsEndpoint(const std::string& ep, std::string& path);

    // Validate endpoint format
    static bool isValidEndpoint(const std::string& ep);

    // Get endpoint type (tcp, uds, unknown)
    static std::string getEndpointType(const std::string& ep);

  private:
    static constexpr std::string_view TCP_PREFIX = "tcp://";
    static constexpr std::string_view UDS_PREFIX = "uds://";
};

} // namespace ethercat_sim::communication