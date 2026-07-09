#include "app/context.hpp"
#include "sim/solar_system.hpp"

namespace solar::app {

void run_loop(AppContext& ctx);

void log_shutdown_report(const sim::SolarSystem& simulation);

} // namespace solar::app
