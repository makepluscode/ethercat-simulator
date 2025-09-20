#include "logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

int main()
{
    using namespace ethercat_sim::framework::logger;

    std::cout << "Testing EtherCAT Simulator Logger\n";
    std::cout << "================================\n\n";

    // Test basic logging
    std::cout << "1. Basic logging test:\n";
    Logger::setComponent("test");
    Logger::info("Logger initialized successfully");
    Logger::warn("This is a warning message");
    Logger::error("This is an error message");
    Logger::debug("This is a debug message (should not appear with default INFO level)");

    std::cout << "\n2. Setting DEBUG level:\n";
    Logger::setLevel(LogLevel::DEBUG);
    Logger::debug("Now debug messages should appear");

    std::cout << "\n3. Formatted logging test:\n";
    Logger::info("Connection established to %s on port %d", "localhost", 8080);
    Logger::warn("Retry attempt %d of %d failed", 3, 5);
    Logger::error("Socket error: %s (errno: %d)", "Connection refused", 111);

    std::cout << "\n4. Component switching test:\n";
    Logger::setComponent("master");
    Logger::info("Master application started");
    
    Logger::setComponent("slaves");
    Logger::info("Slaves application started");

    std::cout << "\n5. Thread safety test:\n";
    Logger::setComponent("thread-test");
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i)
    {
        threads.emplace_back([i]() {
            for (int j = 0; j < 3; ++j)
            {
                Logger::info("Thread %d, message %d", i, j);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    std::cout << "\n6. Timestamp test:\n";
    Logger::setTimestampEnabled(true);
    Logger::info("Message with timestamp");
    
    Logger::setTimestampEnabled(false);
    Logger::info("Message without timestamp");

    std::cout << "\n7. Output redirection test:\n";
    Logger::setOutput(std::cerr);
    Logger::info("This message goes to stderr");
    
    Logger::setOutput(std::cout);
    Logger::info("This message goes back to stdout");

    std::cout << "\nLogger test completed successfully!\n";
    return 0;
}