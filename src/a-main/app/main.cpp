#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include "bus/main_socket.h"
#include "kickcat/Frame.h"
#include "kickcat/protocol.h"
#include "mvc/controller/main_controller.h"
#include "mvc/model/main_model.h"
#if HAVE_FTXUI
#include "mvc/view/main_tui.h"
#endif

using ethercat_sim::bus::MainSocket;

static void usage(const char* argv0)
{
    std::cerr << "Usage: " << argv0 << " [--uds PATH | --tcp HOST:PORT] [--cycle us]\n";
}

int main(int argc, char** argv)
{
    std::string endpoint = "uds:///tmp/ethercat_bus.sock";
    int cycle_us = 1000;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--uds" && i+1 < argc) { endpoint = std::string("uds://") + argv[++i]; }
        else if (a == "--tcp" && i+1 < argc) { endpoint = std::string("tcp://") + argv[++i]; }
        else if (a == "--cycle" && i+1 < argc) { cycle_us = std::stoi(argv[++i]); }
        else if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
    }

    try {
        static std::atomic_bool stop{false};
        // Signals: Ctrl+C (SIGINT), SIGTERM, Ctrl+Z (SIGTSTP)
        auto on_signal = [](int){ stop.store(true); };
        struct sigaction sa{}; sa.sa_handler = on_signal; sigemptyset(&sa.sa_mask); sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGTSTP, &sa, nullptr);
        std::signal(SIGPIPE, SIG_IGN);

        auto controller = std::make_shared<ethercat_sim::app::main::MainController>(endpoint, cycle_us);
        controller->start();

        // Setup non-blocking keyboard read for ESC
        bool tty = ::isatty(STDIN_FILENO);
        struct termios oldt{}; bool term_active = false;
        if (tty) {
            struct termios raw{};
            if (tcgetattr(STDIN_FILENO, &oldt) == 0) {
                raw = oldt; cfmakeraw(&raw);
                raw.c_lflag &= ~(ICANON | ECHO);
                raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0;
                if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) term_active = true;
            }
        }

        // Idle loop until signal or ESC pressed
        // If FTXUI is available and not running in smoke mode, show TUI
        bool smoke = std::getenv("TUI_SMOKE_TEST") != nullptr;
#if HAVE_FTXUI
        if (!smoke && ::isatty(STDIN_FILENO)) {
            ethercat_sim::app::main::run_main_tui(controller, controller->model(), false);
            stop.store(true);
        } else
#endif
        {
            // Headless loop with ESC support
            while (!stop.load()) {
                if (tty) {
                    struct pollfd pfd{STDIN_FILENO, POLLIN, 0};
                    int pr = ::poll(&pfd, 1, 200);
                    if (pr > 0 && (pfd.revents & POLLIN)) {
                        unsigned char ch = 0; ssize_t n = ::read(STDIN_FILENO, &ch, 1);
                        if (n == 1 && ch == 27) { stop.store(true); break; }
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
            }
        }

        controller->stop();

        if (term_active) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        std::cout << "\n[a-master] Graceful shutdown" << std::endl;
    }
    catch (std::exception const& e) {
        std::cerr << "[a-master] error: " << e.what() << "\n";
        return 1;
    }
}
