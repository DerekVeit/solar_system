#include "app/context.hpp"
#include "sim/solar_system.hpp"

namespace solar::app {

struct AppObjects {
    Window window;
    sim::SolarSystem simulation;
    AppContext context;

    void run_loop();
    void log_shutdown_report() const;
};

AppObjects make_app();

} // namespace solar::app
