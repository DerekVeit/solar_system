#include "app/color.hpp"
#include "app/context.hpp"
#include "app/files.hpp"
#include "app/input.hpp"
#include "app/logging.hpp"
#include "app/window.hpp"
#include "core/constants.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"
#include "sim/clock.hpp"
#include "sim/solar_system.hpp"

#include <fmt/core.h>

#include <chrono>
#include <string>

namespace {

using solar::app::log;

} // namespace

int main() {
    log("----------------------------------------");
    try {
        const auto bodies = solar::core::load_bodies(solar::app::asset_path("data/bodies.json"));
        auto ephemeris = std::make_unique<solar::core::KeplerEphemeris>(bodies);

        solar::sim::SimulationClock clock{solar::core::Epoch{}.at_now()};
        clock.set_time_scale(solar::sim::TimeScale::real_time);
        clock.set_acceleration(solar::core::kSecondsPerDay);

        solar::sim::SolarSystem simulation{std::move(ephemeris), clock};

        solar::app::Window window{{.title = "Solar System", .fullscreen = true}};
        solar::app::AppContext app_context{&window, &simulation};
        solar::app::register_key_handlers(app_context);

        auto previous_time = std::chrono::steady_clock::now();

        const double log_interval = 1.0;
        double log_timer = log_interval; // initially due for logging

        while (!window.should_close()) {
            const auto current_time = std::chrono::steady_clock::now();
            const double delta_seconds =
                std::chrono::duration<double>(current_time - previous_time).count();

            simulation.clock().advance(delta_seconds);

            window.clear_frame();

            const double earth_angle = simulation.state("Earth").position.polar_xy().angle;

            solar::app::Color clear_color = solar::app::color_from_angle(earth_angle);

            bool paused = simulation.clock().time_scale() == solar::sim::TimeScale::paused;
            if (!paused) {
                log_timer += delta_seconds;
                if (log_timer >= log_interval) {
                    log("RGB: ({}) for {:.01f}° at {}", clear_color.to_string(),
                        earth_angle * solar::core::kRadToDeg,
                        simulation.clock().epoch().to_string());
                    log_timer -= log_interval;
                }
            }

            window.set_clear_color(clear_color);
            window.swap_buffers();
            window.poll_events();
            previous_time = current_time;
        }

        log("Earth position at shutdown: {}", simulation.state("Earth").to_string());
        log("Venus position at shutdown: {}", simulation.state("Venus").to_string());
        log("Mars position at shutdown: {}", simulation.state("Mars").to_string());
        log("Jupiter position at shutdown: {}", simulation.state("Jupiter").to_string());

        log("Simulation epoch JD: {:.4f}", simulation.clock().epoch().jd);
        log("Simulation epoch: {}", simulation.clock().epoch().to_string());

        return 0;
    } catch (const std::exception& error) {
        fmt::print(stderr, "fatal error: {}\n", error.what());
        return 1;
    }
}