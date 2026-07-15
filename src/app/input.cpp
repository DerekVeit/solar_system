#include "app/input.hpp"

#include "app/logging.hpp"
#include "app/scene/scene.hpp"
#include "app/window.hpp"
#include "sim/clock.hpp"
#include "sim/solar_system.hpp"

#include <fmt/core.h>

namespace {

using solar::app::log;

void change_acceleration(solar::sim::SimulationClock& clock, double multiplier) {
    const double accel = clock.acceleration() * multiplier;
    const char* direction = multiplier < 1.0 ? "slower" : "faster";
    log("{}  {}: {}", clock.epoch().to_string(), direction, accel);
    clock.set_acceleration(accel);
    if (clock.time_scale() != solar::sim::TimeScale::accelerated) {
        log("(not currently in the accelerated time scale)");
    }
}

} // namespace

namespace solar::app {

void key_callback(GLFWwindow* glfw_window, int key, int /*scancode*/, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    auto* app_context = static_cast<AppContext*>(glfwGetWindowUserPointer(glfw_window));
    if (app_context == nullptr) {
        return;
    }

    Window* window = app_context->window;
    sim::SolarSystem* simulation = app_context->simulation;
    Scene* scene = app_context->scene;

    if (window == nullptr) {
        fmt::print(stderr, "app_context->window is nullptr\n");
        return;
    }
    if (simulation == nullptr) {
        fmt::print(stderr, "app_context->simulation is nullptr\n");
        return;
    }
    sim::SimulationClock& clock = simulation->clock();

    switch (key) {
        case GLFW_KEY_ESCAPE:
            log("escaping");
            window->request_close();
            break;
        case GLFW_KEY_SPACE:
            log("{}  pausing", clock.epoch().to_string());
            clock.set_time_scale(sim::TimeScale::paused);
            break;
        case GLFW_KEY_R:
            log("{}  real", clock.epoch().to_string());
            clock.set_time_scale(sim::TimeScale::real_time);
            break;
        case GLFW_KEY_A:
            log("{}  accelerated", clock.epoch().to_string());
            clock.set_time_scale(sim::TimeScale::accelerated);
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
        case GLFW_KEY_PAGE_UP:
            scene->set_scale(scene->scale() * 0.80f);
            break;
        case GLFW_KEY_PAGE_DOWN:
            scene->set_scale(scene->scale() * 1.25f);
            break;
        default:
            break;
    }
}

void register_key_handlers(AppContext& app_context) {
    app_context.window->set_user_pointer(&app_context);
    app_context.window->set_key_callback(key_callback);
}

} // namespace solar::app
