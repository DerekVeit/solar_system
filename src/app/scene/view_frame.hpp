#pragma once

namespace solar::app {

/// Per-frame view parameters owned by Scene and passed to body visuals.
struct ViewFrame {
    float half_extent_au{2.0f};
    float aspect_ratio{1.0f};
    int framebuffer_height{0};
};

} // namespace solar::app
