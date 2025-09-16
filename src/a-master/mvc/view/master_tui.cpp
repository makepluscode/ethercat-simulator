#include "mvc/view/master_tui.h"

#include <chrono>
#include <thread>
#include <string>

#include "mvc/controller/master_controller.h"
#include "mvc/model/master_model.h"

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;
using namespace std::chrono_literals;

namespace ethercat_sim::app::master {

void run_master_tui(std::shared_ptr<MasterController> controller,
                    std::shared_ptr<MasterModel> model,
                    bool smoke_test)
{
    auto screen = ScreenInteractive::Fullscreen();

    auto renderer = Renderer([&] {
        auto snap = model->snapshot();
        auto status = text("status: " + snap.status);
        auto info = vbox({
            text("EtherCAT Master"),
            separator(),
            text("slaves: " + std::to_string(snap.detected_slaves)),
            text(std::string("PREOP: ") + (snap.preop ? "yes" : "no")),
            text(std::string("OP: ") + (snap.operational ? "yes" : "no")),
            separator(),
            text("keys: [s]can  [i]nit  [o]p  [ESC]/[q]uit"),
            separator(),
            status,
        }) | border;
        return info;
    });

    auto events = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape || e == Event::Character('q')) { screen.Exit(); return true; }
        if (e == Event::Character('s')) { controller->scan(); return true; }
        if (e == Event::Character('i')) { controller->initPreop(); return true; }
        if (e == Event::Character('o')) { controller->requestOperational(); return true; }
        return false;
    });

    if (smoke_test) {
        std::thread([&] { std::this_thread::sleep_for(300ms); screen.PostEvent(Event::Custom); screen.Exit(); }).detach();
    }
    screen.Loop(events);
}

} // namespace ethercat_sim::app::master
