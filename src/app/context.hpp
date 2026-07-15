#pragma once

#include "app/scene/scene.hpp"
#include "app/window.hpp"
#include "sim/solar_system.hpp"

namespace solar::app {

struct AppContext {
    Window* window;
    sim::SolarSystem* simulation;
    app::Scene* scene;
};

} // namespace solar::app
