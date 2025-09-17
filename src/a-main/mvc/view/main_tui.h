#pragma once

#include <memory>

namespace ftxui { class ComponentBase; }

namespace ethercat_sim::app::main {

class MainController;
class MainModel;

void run_main_tui(std::shared_ptr<MainController> controller,
                    std::shared_ptr<MainModel> model,
                    bool smoke_test = false);

} // namespace ethercat_sim::app::main
