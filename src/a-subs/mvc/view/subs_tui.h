#pragma once

#include <memory>

namespace ethercat_sim::app::subs {

class SubsController;
class SubsModel;

void run_subs_tui(std::shared_ptr<SubsController> controller,
                   std::shared_ptr<SubsModel> model,
                   bool smoke_test = false);

} // namespace ethercat_sim::app::subs

