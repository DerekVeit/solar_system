#pragma once

#include "app/scene/camera.hpp"

namespace solar::app {

/// Per-frame draw parameters: camera snapshot plus display options.
struct ViewFrame {
    Camera camera{};
    int framebuffer_height{0};
    bool body_scaling{true};
};

} // namespace solar::app
