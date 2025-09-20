#include "master_tui.h"

#include <chrono>
#include <string>
#include <thread>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "logic/master_controller.h"
#include "logic/master_model.h"

using namespace ftxui;
using namespace std::chrono_literals;

namespace ethercat_sim::app::master {

void run_master_tui(std::shared_ptr<MasterController> controller,
                    std::shared_ptr<MasterModel> model, bool smoke_test) {
    auto screen = ScreenInteractive::Fullscreen();

    int         selected = 0;
    std::string idx_str  = "0x6002";
    std::string sub_str  = "0";
    std::string val_str  = "0x0";

    auto idx_input = Input(&idx_str, "index(hex)");
    auto sub_input = Input(&sub_str, "sub");
    auto val_input = Input(&val_str, "value(hex)");
    auto do_read   = Button("Read", [&] {
        unsigned int idx   = 0;
        unsigned int sub   = 0;
        uint32_t     value = 0;
        try {
            idx = std::stoul(idx_str, nullptr, 0);
        } catch (...) {
            idx = 0;
        }
        try {
            sub = std::stoul(sub_str, nullptr, 0);
        } catch (...) {
            sub = 0;
        }
        controller->sdoUpload(selected, static_cast<uint16_t>(idx), static_cast<uint8_t>(sub),
                                value);
    });
    auto do_write  = Button("Write", [&] {
        unsigned int idx = 0;
        unsigned int sub = 0;
        unsigned int v   = 0;
        try {
            idx = std::stoul(idx_str, nullptr, 0);
        } catch (...) {
            idx = 0;
        }
        try {
            sub = std::stoul(sub_str, nullptr, 0);
        } catch (...) {
            sub = 0;
        }
        try {
            v = std::stoul(val_str, nullptr, 0);
        } catch (...) {
            v = 0;
        }
        controller->sdoDownload(selected, static_cast<uint16_t>(idx), static_cast<uint8_t>(sub),
                                 static_cast<uint32_t>(v));
    });

    auto inputs = Container::Vertical({idx_input, sub_input, val_input, do_read, do_write});

    auto renderer = Renderer(inputs, [&] {
        auto snap   = model->snapshot();
        auto status = text("status: " + snap.status);
        if (selected >= static_cast<int>(snap.slaves.size()))
            selected = static_cast<int>(snap.slaves.size()) - 1;
        if (selected < 0)
            selected = 0;
        std::vector<Element> rows;
        rows.push_back(
            hbox({text("Addr") | bold | underlined, text("  "), text("State") | bold | underlined,
                  text("  "), text("AL") | bold | underlined}));
        for (size_t i = 0; i < snap.slaves.size(); ++i) {
            auto const& r    = snap.slaves[i];
            auto        line = hbox({text(std::to_string(r.address)), text("  "), text(r.state),
                                     text("  "), text(r.al_code)});
            if (static_cast<int>(i) == selected)
                line = line | inverted;
            rows.push_back(line);
        }
        auto sdo_panel = vbox({
                             text("SDO"),
                             hbox({text("index:"), separator(), idx_input->Render()}),
                             hbox({text("sub:"), separator(), sub_input->Render()}),
                             hbox({text("value:"), separator(), val_input->Render()}),
                             hbox({do_read->Render(), text("  "), do_write->Render()}),
                             text("last:" + snap.sdo_value_hex),
                             text("result:" + snap.sdo_status),
                         }) |
                         border;
        auto info = vbox({
                        text("EtherCAT Master"),
                        separator(),
                        text("slaves: " + std::to_string(snap.detected_slaves)),
                        text(std::string("PREOP: ") + (snap.preop ? "yes" : "no")),
                        text(std::string("OP: ") + (snap.operational ? "yes" : "no")),
                        separator(),
                        vbox(std::move(rows)) | frame,
                        separator(),
                        text("keys: [s]can  [i]nit  [o]p  [Up/Down] select  [q]/[ESC] quit"),
                        separator(),
                        hbox({sdo_panel, filler()}),
                        separator(),
                        status,
                    }) |
                    border;
        return info;
    });

    auto events = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Escape || e == Event::Character('q')) {
            screen.Exit();
            return true;
        }
        if (e == Event::Character('s')) {
            controller->scan();
            return true;
        }
        if (e == Event::Character('i')) {
            controller->initPreop();
            return true;
        }
        if (e == Event::Character('o')) {
            controller->requestOperational();
            return true;
        }
        if (e == Event::ArrowDown) {
            selected++;
            return true;
        }
        if (e == Event::ArrowUp) {
            selected--;
            if (selected < 0)
                selected = 0;
            return true;
        }
        return false;
    });

    if (smoke_test) {
        std::thread([&] {
            std::this_thread::sleep_for(300ms);
            screen.PostEvent(Event::Custom);
            screen.Exit();
        }).detach();
    }
    screen.Loop(events);
}

} // namespace ethercat_sim::app::master
