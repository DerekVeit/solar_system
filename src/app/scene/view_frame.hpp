#pragma once

namespace solar::app {

/// Per-frame view parameters owned by Scene and passed to body visuals.
struct ViewFrame {
    float half_extent_au{2.0f};
    float aspect_ratio{1.0f};
    int framebuffer_height{0};
    bool body_scaling{true};
    /// Camera center in world coordinates (AU, ecliptic plane).
    float center_x_au{0.0f};
    float center_y_au{0.0f};
};

} // namespace solar::app
