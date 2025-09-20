#pragma once

#include <memory>

namespace ethercat_sim::app::slaves
{

class SlavesController;
class SlavesModel;

void run_slaves_tui(std::shared_ptr<SlavesController> controller,
                    std::shared_ptr<SlavesModel> model, bool smoke_test = false);

} // namespace ethercat_sim::app::slaves
