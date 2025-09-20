#pragma once

#include <memory>

namespace ftxui
{
class ComponentBase;
}

namespace ethercat_sim::app::master
{

class MasterController;
class MasterModel;

void run_master_tui(std::shared_ptr<MasterController> controller,
                    std::shared_ptr<MasterModel> model, bool smoke_test = false);

} // namespace ethercat_sim::app::master
