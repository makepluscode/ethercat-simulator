#pragma once

#include <string>
#include <memory>
#include <functional>

namespace ethercat_sim::communication {

class SocketFactory {
public:
    // Socket creation callback type
    using SocketCreator = std::function<int(const std::string&)>;
    
    // Register socket creator for endpoint type
    static void registerCreator(const std::string& type, SocketCreator creator);
    
    // Create socket for given endpoint
    static int createSocket(const std::string& endpoint);
    
    // Check if endpoint type is supported
    static bool isSupported(const std::string& endpoint);
    
    // Get default creators for TCP and UDS
    static void registerDefaultCreators();

private:
    static std::unordered_map<std::string, SocketCreator> creators_;
};

} // namespace ethercat_sim::communication