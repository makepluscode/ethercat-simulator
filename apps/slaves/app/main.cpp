#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

#include "logic/slaves_controller.h"
#include "logic/slaves_model.h"
#if HAVE_FTXUI
#include "gui/slaves_tui.h"
#endif

#include "ethercat_sim/app/cli_runtime.h"

static void usage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " [--uds PATH | --tcp HOST:PORT] [--count N] [--headless]\n";
}

int main(int argc, char** argv) {
    std::string endpoint       = "uds:///tmp/ethercat_bus.sock";
    std::size_t count          = 1;
    bool        force_headless = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--uds" && i + 1 < argc) {
            endpoint = std::string("uds://") + argv[++i];
        } else if (a == "--tcp" && i + 1 < argc) {
            endpoint = std::string("tcp://") + argv[++i];
        } else if (a == "--count" && i + 1 < argc) {
            count = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (a == "--headless") {
            force_headless = true;
        } else if (a == "-h" || a == "--help") {
            usage(argv[0]);
            return 0;
        }
    }

    static std::atomic_bool stop{false};
    ethercat_sim::app::installSignalHandlers(stop);

    auto controller = std::make_shared<ethercat_sim::app::slaves::SlavesController>(
        endpoint, static_cast<int>(count));
    controller->start();
    bool smoke = std::getenv("TUI_SMOKE_TEST") != nullptr;
#if HAVE_FTXUI
    bool interactive = !force_headless && !smoke && ::isatty(STDIN_FILENO);
#else
    bool interactive = false;
#endif
#if HAVE_FTXUI
    if (interactive) {
        ethercat_sim::app::slaves::run_slaves_tui(controller, controller->model(), false);
        stop.store(true);
    } else
#endif
    {
        ethercat_sim::app::TerminalGuard terminal;
        using namespace std::chrono_literals;
        while (!stop.load()) {
            if (terminal.isActive()) {
                if (terminal.pollForEscape(200ms)) {
                    stop.store(true);
                    break;
                }
            } else {
                std::this_thread::sleep_for(200ms);
            }
        }
    }
    controller->stop();
    std::cout << "[a-slaves] Graceful shutdown\n";
    return 0;
}
