#include "app/logging.hpp"

#include "app/input.hpp"

namespace solar::app {

void key_callback(GLFWwindow* glfw_window, int key, int /*scancode*/, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    auto* app_context = static_cast<solar::app::AppContext*>(
        glfwGetWindowUserPointer(glfw_window));
    if (app_context == nullptr) {
        return;
    }

    auto window = app_context->window;
    auto simulation = app_context->simulation;
    auto clock = simulation->clock();

    switch (key) {
        case GLFW_KEY_ESCAPE:
            log("escaping");
            window->request_close();
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
            simulation->change_acceleration(0.5);
            break;
        case GLFW_KEY_EQUAL:
            if (mods == GLFW_MOD_SHIFT) {
                simulation->change_acceleration(2.0);
            }
            break;
        case GLFW_KEY_KP_ADD:
            simulation->change_acceleration(2.0);
            break;
        default:
            break;
    }
}

void register_key_handlers(solar::app::AppContext& app_context) {
    app_context.window->set_user_pointer(&app_context.simulation);
    app_context.window->set_key_callback(key_callback);
}

}

