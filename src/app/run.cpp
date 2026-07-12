#include "app/run.hpp"

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

#include <chrono>
#include <string>

namespace {

constexpr double log_interval = 1.0;

} // namespace

namespace solar::app {

AppObjects make_app() {
    const auto bodies = core::load_bodies(asset_path("data/bodies.json"));
    auto ephemeris = std::make_unique<core::KeplerEphemeris>(bodies);

    sim::SimulationClock clock{core::Epoch::at_now()};
    clock.set_time_scale(sim::TimeScale::real_time);
    clock.set_acceleration(core::kSecondsPerDay);

    AppObjects app{
        Window{{.title = "Solar System", .fullscreen = true}},
        sim::SolarSystem{std::move(ephemeris), clock},
        {},
    };
    app.context = AppContext{&app.window, &app.simulation};

    solar::app::register_key_handlers(app.context);

    return app;
}

void AppObjects::run_loop() {
    auto previous_time = std::chrono::steady_clock::now();

    double log_timer = log_interval; // initially due for logging

    while (!window.should_close()) {
        const auto current_time = std::chrono::steady_clock::now();
        const double delta_seconds =
            std::chrono::duration<double>(current_time - previous_time).count();

        simulation.clock().advance(delta_seconds);

        const double earth_angle = simulation.state("Earth").position.polar_xy().angle;

        Color clear_color = color_from_angle(earth_angle);

        bool paused = simulation.clock().time_scale() == sim::TimeScale::paused;
        if (!paused) {
            log_timer += delta_seconds;
            if (log_timer >= log_interval) {
                log("RGB: ({}) for {:.01f}° at {}", clear_color.to_string(),
                    earth_angle * core::kRadToDeg, simulation.clock().epoch().to_string());
                log_timer -= log_interval;
            }
        }

        window.set_clear_color(clear_color);
        window.clear_frame();

        window.swap_buffers();
        window.poll_events();
        previous_time = current_time;
    }
}

void AppObjects::log_shutdown_report() const {
    log("Earth position at shutdown: {}", simulation.state("Earth").to_string());
    log("Venus position at shutdown: {}", simulation.state("Venus").to_string());
    log("Mars position at shutdown: {}", simulation.state("Mars").to_string());
    log("Jupiter position at shutdown: {}", simulation.state("Jupiter").to_string());

    log("Simulation epoch JD: {:.4f}", simulation.clock().epoch().jd);
    log("Simulation epoch: {}", simulation.clock().epoch().to_string());
}

} // namespace solar::app
