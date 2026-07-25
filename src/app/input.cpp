#include "app/input.hpp"

#include "app/follow_targets.hpp"
#include "app/logging.hpp"
#include "app/scene/camera.hpp"
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
    if (scene == nullptr) {
        fmt::print(stderr, "app_context->scene is nullptr\n");
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
        case GLFW_KEY_T:
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
            scene->zoom_in();
            break;
        case GLFW_KEY_PAGE_DOWN:
            scene->zoom_out();
            break;
        case GLFW_KEY_B: {
            bool enabled = (mods == GLFW_MOD_SHIFT);
            scene->body_scaling(enabled);
            log("body scaling {}", enabled ? "on" : "off");
        } break;
        case GLFW_KEY_W:
            scene->add_pitch(-Camera::kYawPitchStepRad);
            break;
        case GLFW_KEY_S:
            scene->add_pitch(Camera::kYawPitchStepRad);
            break;
        case GLFW_KEY_A:
            scene->add_yaw(-Camera::kYawPitchStepRad);
            break;
        case GLFW_KEY_D:
            scene->add_yaw(Camera::kYawPitchStepRad);
            break;
        case GLFW_KEY_KP_9:
            scene->set_view_from_north();
            log("view from celestial north");
            break;
        case GLFW_KEY_KP_1:
            scene->set_view_from_south();
            log("view from celestial south");
            break;
        case GLFW_KEY_KP_8:
            scene->set_view_from_ypos();
            log("view from celestial Y+");
            break;
        case GLFW_KEY_KP_2:
            scene->set_view_from_yneg();
            log("view from celestial Y-");
            break;
        case GLFW_KEY_KP_6:
            scene->set_view_from_east();
            log("view from celestial east");
            break;
        case GLFW_KEY_KP_4:
            scene->set_view_from_west();
            log("view from celestial west");
            break;
        case GLFW_KEY_LEFT:
            scene->pan_view_fraction(-Camera::kPanFraction, 0.0f);
            break;
        case GLFW_KEY_RIGHT:
            scene->pan_view_fraction(Camera::kPanFraction, 0.0f);
            break;
        case GLFW_KEY_UP:
            scene->pan_view_fraction(0.0f, Camera::kPanFraction);
            break;
        case GLFW_KEY_DOWN:
            scene->pan_view_fraction(0.0f, -Camera::kPanFraction);
            break;
        case GLFW_KEY_INSERT:
            scene->zoom_on_followed_body();
            break;
        case GLFW_KEY_HOME:
            scene->reset_view_center();
            log("view centered on Sun");
            break;
        case GLFW_KEY_END:
            scene->reset_orientation();
            log("camera orientation reset");
            break;
        case GLFW_KEY_GRAVE_ACCENT: {
            std::optional<std::string> previously_followed_body = scene->release_from_follow();
            if (previously_followed_body.has_value()) {
                log("released from following {}", *previously_followed_body);
            }
            break;
        }
        default:
            if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
                const std::size_t index = static_cast<std::size_t>(key - GLFW_KEY_0);
                const std::string_view target = kFollowTargets[index];
                scene->set_follow_target(*simulation, std::string{target});
                log("following {}", target);
            }
            break;
    }
}

void register_key_handlers(AppContext& app_context) {
    app_context.window->set_user_pointer(&app_context);
    app_context.window->set_key_callback(key_callback);
}

} // namespace solar::app
