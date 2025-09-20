#include "ethercat_sim/communication/endpoint_parser.h"

#include <cstdint>
#include <stdexcept>

namespace ethercat_sim::communication {

bool EndpointParser::parseTcpEndpoint(const std::string& ep, std::string& host, uint16_t& port) {
    std::string_view v{ep};

    // Check if it starts with tcp://
    if (v.rfind(TCP_PREFIX, 0) != 0) {
        return false;
    }

    // Remove tcp:// prefix
    v.remove_prefix(TCP_PREFIX.size());

    // Find colon separator
    auto colon = v.find(':');
    if (colon == std::string_view::npos) {
        return false;
    }

    // Extract host and port
    host.assign(v.substr(0, colon));
    auto pstr = v.substr(colon + 1);

    try {
        int p = std::stoi(std::string(pstr));
        if (p < 1 || p > 65535) {
            return false;
        }
        port = static_cast<uint16_t>(p);
    } catch (const std::exception&) {
        return false;
    }

    return true;
}

bool EndpointParser::parseUdsEndpoint(const std::string& ep, std::string& path) {
    std::string_view v{ep};

    // Check if it starts with uds://
    if (v.rfind(UDS_PREFIX, 0) != 0) {
        return false;
    }

    // Remove uds:// prefix
    v.remove_prefix(UDS_PREFIX.size());

    // Extract path
    path.assign(v);
    return true;
}

bool EndpointParser::isValidEndpoint(const std::string& ep) {
    std::string host, path;
    uint16_t    port;

    return parseTcpEndpoint(ep, host, port) || parseUdsEndpoint(ep, path);
}

std::string EndpointParser::getEndpointType(const std::string& ep) {
    if (ep.rfind(TCP_PREFIX, 0) == 0) {
        return "tcp";
    } else if (ep.rfind(UDS_PREFIX, 0) == 0) {
        return "uds";
    } else {
        return "unknown";
    }
}

} // namespace ethercat_sim::communication