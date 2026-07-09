#include "app/context.hpp"
#include "app/files.hpp"
#include "app/input.hpp"
#include "app/logging.hpp"
#include "app/run.hpp"
#include "app/window.hpp"
#include "core/constants.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"
#include "sim/clock.hpp"
#include "sim/solar_system.hpp"

#include <fmt/core.h>

#include <string>

namespace {

using solar::app::log;

} // namespace

int main() {
    log("----------------------------------------");
    try {
        const auto bodies = solar::core::load_bodies(solar::app::asset_path("data/bodies.json"));
        auto ephemeris = std::make_unique<solar::core::KeplerEphemeris>(bodies);

        solar::sim::SimulationClock clock{solar::core::Epoch::at_now()};
        clock.set_time_scale(solar::sim::TimeScale::real_time);
        clock.set_acceleration(solar::core::kSecondsPerDay);

        solar::sim::SolarSystem simulation{std::move(ephemeris), clock};

        solar::app::Window window{{.title = "Solar System", .fullscreen = true}};
        solar::app::AppContext ctx{&window, &simulation};
        solar::app::register_key_handlers(ctx);

        solar::app::run_loop(ctx);
        solar::app::log_shutdown_report(simulation);

        return 0;
    } catch (const std::exception& error) {
        fmt::print(stderr, "fatal error: {}\n", error.what());
        return 1;
    }
}