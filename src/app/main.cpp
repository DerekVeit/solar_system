#include <chrono>
#include <filesystem>
#include <string>

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>

#include "app/logging.hpp"
#include "app/window.hpp"
#include "core/constants.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"
#include "sim/clock.hpp"
#include "sim/solar_system.hpp"

namespace {

using solar::app::log;

std::filesystem::path asset_path(const std::string& relative) {
    const std::filesystem::path executable =
        std::filesystem::canonical("/proc/self/exe").parent_path();
    return executable / "assets" / relative;
}

void change_acceleration(solar::sim::SimulationClock& clock, double multiplier) {
    const double accel = clock.acceleration() * multiplier;
    const char* direction = multiplier < 1.0 ? "slower" : "faster";
    log("{}  {}: {}", clock.epoch().to_string(), direction, accel);
    clock.set_acceleration(accel);
    if (clock.time_scale() != solar::sim::TimeScale::accelerated) {
        log("(not currently in the accelerated time scale)");
    }
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    auto* simulation = static_cast<solar::sim::SolarSystem*>(
        glfwGetWindowUserPointer(window));
    if (simulation == nullptr) {
        return;
    }

    solar::sim::SimulationClock& clock = simulation->clock();

    switch (key) {
        case GLFW_KEY_ESCAPE:
            log("escaping");
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            log("{}  pausing", clock.epoch().to_string());
            clock.set_time_scale(solar::sim::TimeScale::paused);
            break;
        case GLFW_KEY_R:
            log("{}  real", clock.epoch().to_string());
            clock.set_time_scale(solar::sim::TimeScale::real_time);
            break;
        case GLFW_KEY_A:
            log("{}  accelerated", clock.epoch().to_string());
            clock.set_time_scale(solar::sim::TimeScale::accelerated);
            break;
        case GLFW_KEY_MINUS:
        case GLFW_KEY_KP_SUBTRACT:
            change_acceleration(clock, 0.5);
            break;
        case GLFW_KEY_EQUAL:
            if (mods == GLFW_MOD_SHIFT) {
                change_acceleration(clock, 2.0);
            }
            break;
        case GLFW_KEY_KP_ADD:
            change_acceleration(clock, 2.0);
            break;
        default:
            break;
    }
}

std::string planet_report(solar::core::StateVector planet_state) {
    const auto position = planet_state.position;
    const auto polar = position.polar_xy();
    return fmt::format("{:.0f} {:.0f} {:.0f} km ({:.0f} km @ {:.0f}°)",
                       position.km.x, position.km.y, position.km.z,
                       polar.length, polar.angle * solar::core::kRadToDeg);
}

}  // namespace

int main() {
    log("----------------------------------------");
    try {
        const auto bodies = solar::core::load_bodies(asset_path("data/bodies.json"));
        auto ephemeris =
            std::make_unique<solar::core::KeplerEphemeris>(bodies);

        //solar::sim::SimulationClock clock{solar::core::Epoch{solar::core::kJ2000Jd}};
        solar::sim::SimulationClock clock{solar::core::Epoch{}.now()};
        clock.set_time_scale(solar::sim::TimeScale::real_time);
        clock.set_acceleration(solar::core::kSecondsPerDay);

        solar::sim::SolarSystem simulation{std::move(ephemeris), clock};

        solar::app::Window window{{.title = "Solar System", .fullscreen = true}};
        glfwSetWindowUserPointer(window.handle(), &simulation);
        glfwSetKeyCallback(window.handle(), key_callback);

        auto previous_time = std::chrono::steady_clock::now();

        double log_timer = 1.0;

        while (!window.should_close()) {
            const auto current_time = std::chrono::steady_clock::now();
            const double delta_seconds =
                std::chrono::duration<double>(current_time - previous_time).count();

            simulation.clock().advance(delta_seconds);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const double earth_angle = simulation.state("Earth").position.polar_xy().angle;

            float red(0.0f), green(0.0f), blue(0.0f);
            if (std::cos(earth_angle) < 0) {
                green = -std::cos(earth_angle);
            } else {
                red = std::cos(earth_angle);
            }
            blue = ((-std::sin(earth_angle) + 1.0f) / 2.0f) * 0.8f + 0.2f;

            bool paused = simulation.clock().time_scale() == solar::sim::TimeScale::paused;
            if (!paused) {
                log_timer += delta_seconds;
                if (log_timer >= 1.0) {
                    log("RGB: ({:.02f} {:.02f} {:.02f}) for {:.01f}° at {}",
                        red, green, blue,
                        earth_angle * solar::core::kRadToDeg,
                        simulation.clock().epoch().to_string());
                    log_timer -= 1.0;
                }
            }

            window.set_clear_color(red, green, blue, 1.0f);
            window.swap_buffers();
            window.poll_events();
            previous_time = current_time;
        }

        log("Earth position at shutdown: {}", planet_report(simulation.state("Earth")));
        log("Venus position at shutdown: {}", planet_report(simulation.state("Venus")));
        log("Mars position at shutdown: {}", planet_report(simulation.state("Mars")));
        log("Jupiter position at shutdown: {}", planet_report(simulation.state("Jupiter")));

        log("Simulation epoch JD: {:.4f}", simulation.clock().epoch().jd);
        log("Simulation epoch: {}", simulation.clock().epoch().to_string());

        return 0;
    } catch (const std::exception& error) {
        fmt::print(stderr, "fatal error: {}\n", error.what());
        return 1;
    }
}