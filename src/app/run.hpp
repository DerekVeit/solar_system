#include "app/context.hpp"
#include "sim/solar_system.hpp"

namespace solar::app {

struct AppObjects {
    Window window;
    sim::SolarSystem simulation;
    AppContext context;
};

AppObjects make_app();

void run_loop(AppContext& ctx);

void log_shutdown_report(const sim::SolarSystem& simulation);

} // namespace solar::app
