#include "ethercat_sim/communication/socket_factory.h"

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <unordered_map>

#include "ethercat_sim/communication/endpoint_parser.h"

namespace ethercat_sim::communication {

std::unordered_map<std::string, SocketFactory::SocketCreator> SocketFactory::creators_;

void SocketFactory::registerCreator(const std::string& type, SocketCreator creator) {
    creators_[type] = std::move(creator);
}

int SocketFactory::createSocket(const std::string& endpoint) {
    std::string type = EndpointParser::getEndpointType(endpoint);

    auto it = creators_.find(type);
    if (it == creators_.end()) {
        throw std::runtime_error("Unsupported endpoint type: " + type);
    }

    return it->second(endpoint);
}

bool SocketFactory::isSupported(const std::string& endpoint) {
    std::string type = EndpointParser::getEndpointType(endpoint);
    return creators_.find(type) != creators_.end();
}

void SocketFactory::registerDefaultCreators() {
    // TCP socket creator
    registerCreator("tcp", [](const std::string& endpoint) -> int {
        std::string host;
        uint16_t    port;
        if (!EndpointParser::parseTcpEndpoint(endpoint, host, port)) {
            throw std::runtime_error("Invalid TCP endpoint: " + endpoint);
        }

        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Failed to create TCP socket: " +
                                     std::string(strerror(errno)));
        }

        struct sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);

        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            // Try to resolve hostname
            struct hostent* he = gethostbyname(host.c_str());
            if (he == nullptr) {
                ::close(sock);
                throw std::runtime_error("Failed to resolve host: " + host);
            }
            memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
        }

        if (::connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(sock);
            throw std::runtime_error("Failed to connect to TCP endpoint: " +
                                     std::string(strerror(errno)));
        }

        return sock;
    });

    // UDS socket creator
    registerCreator("uds", [](const std::string& endpoint) -> int {
        std::string path;
        if (!EndpointParser::parseUdsEndpoint(endpoint, path)) {
            throw std::runtime_error("Invalid UDS endpoint: " + endpoint);
        }

        int sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Failed to create UDS socket: " +
                                     std::string(strerror(errno)));
        }

        struct sockaddr_un addr {};
        addr.sun_family = AF_UNIX;

        if (path.length() >= sizeof(addr.sun_path)) {
            ::close(sock);
            throw std::runtime_error("UDS path too long: " + path);
        }

        if (path[0] == '@') {
            // Abstract socket
            strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
            addr.sun_path[0] = '\0'; // Abstract socket starts with null byte
        } else {
            // Regular filesystem socket
            strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
        }

        if (::connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(sock);
            throw std::runtime_error("Failed to connect to UDS endpoint: " +
                                     std::string(strerror(errno)));
        }

        return sock;
    });
}

} // namespace ethercat_sim::communication