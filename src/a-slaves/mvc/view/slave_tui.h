#pragma once

#include <memory>

namespace ethercat_sim::app::slaves {

class SlaveController;
class SlaveModel;

void run_slave_tui(std::shared_ptr<SlaveController> controller,
                   std::shared_ptr<SlaveModel> model,
                   bool smoke_test = false);

} // namespace ethercat_sim::app::slaves

