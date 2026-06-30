#include <chrono>
#include <filesystem>
#include <string>

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>

#include "app/window.hpp"
#include "core/constants.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"
#include "sim/clock.hpp"
#include "sim/solar_system.hpp"

namespace {

std::filesystem::path asset_path(const std::string& relative) {
    const std::filesystem::path executable =
        std::filesystem::canonical("/proc/self/exe").parent_path();
    return executable / "assets" / relative;
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

}  // namespace

std::string planet_report(solar::core::StateVector planet_state) {
    auto position = planet_state.position;
    auto polar = position.polar_xy();
    return fmt::format("{:.0f} {:.0f} {:.0f} km ({:.0f} km @ {:.0f}°)\n",
                       position.km.x, position.km.y, position.km.z,
                       polar.length, polar.angle * solar::core::kRadToDeg);
}

int main() {
    try {
        const auto bodies = solar::core::load_bodies(asset_path("data/bodies.json"));
        auto ephemeris =
            std::make_unique<solar::core::KeplerEphemeris>(bodies);

        //solar::sim::SimulationClock clock{solar::core::Epoch{solar::core::kJ2000Jd}};
        solar::sim::SimulationClock clock{solar::core::Epoch{}.now()};
        clock.set_time_scale(solar::sim::TimeScale::real_time);
        clock.set_acceleration(86400.0);

        solar::sim::SolarSystem simulation{std::move(ephemeris), clock};

        solar::app::Window window{{.title = "Solar System", .fullscreen = true}};
        glfwSetKeyCallback(window.handle(), key_callback);

        auto previous_time = std::chrono::steady_clock::now();

        while (!window.should_close()) {
            const auto current_time = std::chrono::steady_clock::now();
            const double delta_seconds =
                std::chrono::duration<double>(current_time - previous_time).count();
            previous_time = current_time;

            simulation.clock().advance(delta_seconds);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            window.swap_buffers();
            window.poll_events();
        }

        fmt::print("Earth position at shutdown: {}", planet_report(simulation.state("Earth")));
        fmt::print("Venus position at shutdown: {}", planet_report(simulation.state("Venus")));
        fmt::print("Mars position at shutdown: {}", planet_report(simulation.state("Mars")));
        fmt::print("Jupiter position at shutdown: {}", planet_report(simulation.state("Jupiter")));

        fmt::print("Simulation epoch JD: {:.4f}\n", simulation.clock().epoch().jd);
        fmt::print("Simulation epoch: {}\n", simulation.clock().epoch().to_string());

        return 0;
    } catch (const std::exception& error) {
        fmt::print(stderr, "fatal error: {}\n", error.what());
        return 1;
    }
}