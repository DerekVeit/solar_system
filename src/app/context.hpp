#pragma once

#include "app/window.hpp"
#include "sim/solar_system.hpp"

namespace solar::app {

struct AppContext {
    Window *window;
    sim::SolarSystem *simulation;
};

} // namespace solar::app
