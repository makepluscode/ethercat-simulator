#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

#include "ethercat_sim/app/cli_runtime.h"
#include "logic/master_controller.h"
#include "logic/master_model.h"
#if HAVE_FTXUI
#include "gui/master_tui.h"
#endif

static void usage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " [--uds PATH | --tcp HOST:PORT] [--cycle us] [--headless]"
              << " [--no-auto]\n";
}

namespace {

bool runAutoSequence(
    const std::shared_ptr<ethercat_sim::app::master::MasterController>& controller) {
    using namespace std::chrono_literals;

    auto model   = controller->model();
    auto attempt = [&](const char* label, auto&& action, auto&& predicate) {
        constexpr int  tries   = 5;
        constexpr auto delay   = 400ms;
        bool           success = false;
        for (int i = 0; i < tries; ++i) {
            action();
            if (predicate()) {
                success = true;
                break;
            }
            std::this_thread::sleep_for(delay);
        }
        if (success) {
            std::cout << "[a-master] Auto sequence step '" << label << "' ok" << std::endl;
        } else {
            std::cerr << "[a-master] Auto sequence step '" << label << "' failed" << std::endl;
        }
        return success;
    };

    auto scan_ok = attempt(
        "scan", [&] { controller->scan(); },
        [&] {
            auto snap = model->snapshot();
            return snap.detected_slaves > 0 && !snap.slaves.empty();
        });
    if (!scan_ok) {
        return false;
    }

    auto preop_ok =
        attempt("preop", [&] { controller->initPreop(); }, [&] { return model->snapshot().preop; });
    if (!preop_ok) {
        return false;
    }

    auto op_ok = attempt(
        "operational", [&] { controller->requestOperational(); },
        [&] { return model->snapshot().operational; });
    return op_ok;
}

} // namespace

int main(int argc, char** argv) {
    std::string endpoint       = "uds:///tmp/ethercat_bus.sock";
    int         cycle_us       = 1000;
    bool        force_headless = false;
    bool        auto_sequence  = true;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--uds" && i + 1 < argc) {
            endpoint = std::string("uds://") + argv[++i];
        } else if (a == "--tcp" && i + 1 < argc) {
            endpoint = std::string("tcp://") + argv[++i];
        } else if (a == "--cycle" && i + 1 < argc) {
            cycle_us = std::stoi(argv[++i]);
        } else if (a == "--headless") {
            force_headless = true;
        } else if (a == "--no-auto") {
            auto_sequence = false;
        } else if (a == "-h" || a == "--help") {
            usage(argv[0]);
            return 0;
        }
    }

    try {
        static std::atomic_bool stop{false};
        ethercat_sim::app::installSignalHandlers(stop);

        auto controller =
            std::make_shared<ethercat_sim::app::master::MasterController>(endpoint, cycle_us);
        controller->start();

        bool auto_ok = true;
        if (auto_sequence) {
            std::cout << "[a-master] Running automatic scan/init/op sequence" << std::endl;
            auto_ok = runAutoSequence(controller);
            if (!auto_ok) {
                std::cerr << "[a-master] Automatic sequence did not reach OP state" << std::endl;
            }
        }

        // Idle loop until signal or ESC pressed
        // If FTXUI is available and not running in smoke mode, show TUI
        bool smoke = std::getenv("TUI_SMOKE_TEST") != nullptr;
#if HAVE_FTXUI
        bool interactive = !force_headless && !smoke && ::isatty(STDIN_FILENO);
#else
        bool interactive = false;
#endif
#if HAVE_FTXUI
        if (interactive) {
            ethercat_sim::app::master::run_master_tui(controller, controller->model(), false);
            stop.store(true);
        } else
#endif
        {
            ethercat_sim::app::TerminalGuard terminal;
            using namespace std::chrono_literals;
            if (!interactive && !auto_sequence) {
                std::cout << "[a-master] Headless mode without auto sequence - waiting for user"
                          << std::endl;
            } else if (!interactive && auto_ok) {
                std::cout << "[a-master] Headless mode - OP state reached" << std::endl;
            }
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
        std::cout << "\n[a-master] Graceful shutdown" << std::endl;
    } catch (std::exception const& e) {
        std::cerr << "[a-master] error: " << e.what() << "\n";
        return 1;
    }
}
