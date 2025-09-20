#include "slaves_tui.h"

#include <chrono>
#include <thread>

#include "logic/slaves_controller.h"
#include "logic/slaves_model.h"

#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;
using namespace std::chrono_literals;

namespace ethercat_sim::app::slaves {

void run_slaves_tui(std::shared_ptr<SlavesController> controller,
                    std::shared_ptr<SlavesModel> model,
                   bool smoke_test)
{
    (void)controller; // no interactions yet
    auto screen = ScreenInteractive::Fullscreen();
    auto renderer = Renderer([&] {
        auto snap = model->snapshot();
        return vbox({
            text("Virtual Slaves"),
            separator(),
            text("endpoint: " + snap.endpoint),
            text("count: " + std::to_string(snap.count)),
            text(std::string("listening: ") + (snap.listening ? "yes" : "no")),
            text(std::string("connected: ") + (snap.connected ? "yes" : "no")),
            separator(),
            text("keys: [q]/[ESC] quit"),
            separator(),
            text("status: " + snap.status),
        }) | border;
    });
    auto events = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape || e == Event::Character('q')) { screen.Exit(); return true; }
        return false;
    });
    if (smoke_test) {
        std::thread([&]{ std::this_thread::sleep_for(300ms); screen.PostEvent(Event::Custom); screen.Exit(); }).detach();
    }
    screen.Loop(events);
}

} // namespace ethercat_sim::app::slaves
