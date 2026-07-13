#include "app/context.hpp"
#include "app/earth_renderer.hpp"
#include "app/window.hpp"
#include "sim/solar_system.hpp"

namespace solar::app {

struct AppObjects {
    Window window;
    sim::SolarSystem simulation;
    AppContext context;
    EarthRenderer earth_renderer;

    void run_loop();
    void log_shutdown_report() const;
};

AppObjects make_app();

} // namespace solar::app
