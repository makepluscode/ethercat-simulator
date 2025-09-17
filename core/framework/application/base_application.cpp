#include "ethercat_sim/framework/application/base_application.h"

#include <iostream>
#include <cstring>

namespace ethercat_sim::framework {

std::atomic_bool BaseApplication::stop_flag_{false};

int BaseApplication::run(int argc, char** argv)
{
    try {
        // Setup signal handlers
        setupSignalHandlers();
        
        // Setup terminal
        setupTerminal();
        
        // Initialize application
        if (!initialize()) {
            std::cerr << "[" << getName() << "] Initialization failed" << std::endl;
            cleanupTerminal();
            return 1;
        }
        
        // Run main loop
        runMainLoop();
        
        // Cleanup
        cleanup();
        cleanupTerminal();
        
        std::cout << "\n[" << getName() << "] Graceful shutdown" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[" << getName() << "] Error: " << e.what() << std::endl;
        cleanupTerminal();
        return 1;
    }
}

void BaseApplication::setupSignalHandlers()
{
    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, nullptr);   // Ctrl+C
    sigaction(SIGTERM, &sa, nullptr);  // Termination request
    sigaction(SIGTSTP, &sa, nullptr);  // Ctrl+Z
    
    // Ignore SIGPIPE
    std::signal(SIGPIPE, SIG_IGN);
}

void BaseApplication::setupTerminal()
{
    is_tty_ = ::isatty(STDIN_FILENO);
    
    if (!is_tty_) {
        return;
    }
    
    if (tcgetattr(STDIN_FILENO, &old_termios_) == 0) {
        struct termios raw = old_termios_;
        cfmakeraw(&raw);
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
            terminal_active_ = true;
        }
    }
}

void BaseApplication::cleanupTerminal()
{
    if (terminal_active_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios_);
        terminal_active_ = false;
    }
}

bool BaseApplication::checkEscapeKey()
{
    if (!is_tty_) {
        return false;
    }
    
    struct pollfd pfd{STDIN_FILENO, POLLIN, 0};
    int result = ::poll(&pfd, 1, 0);
    
    if (result > 0 && (pfd.revents & POLLIN)) {
        unsigned char ch = 0;
        ssize_t n = ::read(STDIN_FILENO, &ch, 1);
        if (n == 1 && ch == 27) { // ESC key
            return true;
        }
    }
    
    return false;
}

void BaseApplication::sleepFor(std::chrono::milliseconds duration)
{
    auto start = std::chrono::steady_clock::now();
    while (!stop_flag_.load()) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed >= duration) {
            break;
        }
        
        // Check for ESC key during sleep
        if (checkEscapeKey()) {
            stop_flag_.store(true);
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void BaseApplication::signalHandler(int signal)
{
    (void)signal; // Suppress unused parameter warning
    stop_flag_.store(true);
}

} // namespace ethercat_sim::framework