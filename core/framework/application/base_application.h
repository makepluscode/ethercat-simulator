#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <poll.h>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace ethercat_sim::framework {

// Base application class with common functionality
class BaseApplication {
  public:
    BaseApplication()          = default;
    virtual ~BaseApplication() = default;

    // Main entry point
    int run(int argc, char** argv);

    // Application-specific initialization
    virtual bool initialize() = 0;

    // Application-specific cleanup
    virtual void cleanup() = 0;

    // Application-specific main loop
    virtual void runMainLoop() = 0;

    // Get application name
    virtual std::string getName() const = 0;

    // Get usage information
    virtual std::string getUsage() const = 0;

  protected:
    // Setup signal handlers
    void setupSignalHandlers();

    // Setup terminal for non-blocking input
    void setupTerminal();

    // Restore terminal settings
    void cleanupTerminal();

    // Check if ESC key was pressed
    bool checkEscapeKey();

    // Check if application should stop
    bool shouldStop() const {
        return stop_flag_.load();
    }

    // Sleep for specified duration, respecting stop flag
    void sleepFor(std::chrono::milliseconds duration);

  private:
    // Signal handler
    static void signalHandler(int signal);

    // Static stop flag for signal handler
    static std::atomic_bool stop_flag_;

    // Terminal state
    struct termios old_termios_ {};
    bool           terminal_active_{false};
    bool           is_tty_{false};
};

} // namespace ethercat_sim::framework