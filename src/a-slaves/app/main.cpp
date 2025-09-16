#include <iostream>
#include <string>

#include "bus/slave_endpoint.h"
#include <atomic>
#include <thread>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <csignal>
#include "mvc/controller/slave_controller.h"
#include "mvc/model/slave_model.h"
#if HAVE_FTXUI
#include "mvc/view/slave_tui.h"
#endif

using ethercat_sim::bus::SlaveEndpoint;

static void usage(const char* argv0)
{
    std::cerr << "Usage: " << argv0 << " [--uds PATH | --tcp HOST:PORT] [--count N]\n";
}

int main(int argc, char** argv)
{
    std::string endpoint = "uds:///tmp/ethercat_bus.sock";
    std::size_t count = 1;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--uds" && i+1 < argc) { endpoint = std::string("uds://") + argv[++i]; }
        else if (a == "--tcp" && i+1 < argc) { endpoint = std::string("tcp://") + argv[++i]; }
        else if (a == "--count" && i+1 < argc) { count = static_cast<std::size_t>(std::stoul(argv[++i])); }
        else if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
    }

    static std::atomic_bool stop{false};
    auto on_signal = [](int){ stop.store(true); };
    struct sigaction sa{}; sa.sa_handler = on_signal; sigemptyset(&sa.sa_mask); sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGTSTP, &sa, nullptr);
    std::signal(SIGPIPE, SIG_IGN);

    auto controller = std::make_shared<ethercat_sim::app::slaves::SlaveController>(endpoint, static_cast<int>(count));
    controller->start();
    bool smoke = std::getenv("TUI_SMOKE_TEST") != nullptr;
#if HAVE_FTXUI
    if (!smoke && ::isatty(STDIN_FILENO)) {
        ethercat_sim::app::slaves::run_slave_tui(controller, controller->model(), false);
        stop.store(true);
    } else
#endif
    {
        // Headless loop with ESC
        bool tty = ::isatty(STDIN_FILENO);
        struct termios oldt{}; bool term_active = false;
        if (tty && tcgetattr(STDIN_FILENO, &oldt) == 0) {
            struct termios raw = oldt; cfmakeraw(&raw);
            raw.c_lflag &= ~(ICANON | ECHO); raw.c_cc[VMIN]=0; raw.c_cc[VTIME]=0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw); term_active = true;
        }
        while (!stop.load()) {
            if (tty) {
                struct pollfd pfd{STDIN_FILENO, POLLIN, 0};
                int pr = ::poll(&pfd, 1, 200);
                if (pr > 0 && (pfd.revents & POLLIN)) {
                    unsigned char ch = 0; if (::read(STDIN_FILENO, &ch, 1) == 1 && ch == 27) { stop.store(true); break; }
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        if (term_active) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
    controller->stop();
    std::cout << "[a-slaves] Graceful shutdown\n";
    return 0;
}
