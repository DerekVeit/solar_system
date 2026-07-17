#pragma once

#include "core/types.hpp"

#include <glm/mat4x4.hpp>

namespace solar::app {

/// Scene-owned camera: world framing for the ecliptic view.
///
/// Geometry is emitted in camera-relative km; view and projection matrices
/// map that space to clip coordinates (orthographic for now).
class Camera {
  public:
    [[nodiscard]] float center_x_au() const { return center_x_au_; }
    [[nodiscard]] float center_y_au() const { return center_y_au_; }
    void set_center_au(float x_au, float y_au);
    void reset_center();

    /// Half the visible view height in AU (center to top edge).
    [[nodiscard]] float half_extent_au() const { return half_extent_au_; }
    void set_half_extent_au(float half_extent_au);

    [[nodiscard]] float aspect_ratio() const { return aspect_ratio_; }
    void set_aspect_ratio(float aspect_ratio);

    [[nodiscard]] float view_width_au() const;
    [[nodiscard]] float view_height_au() const;

    void pan_au(float delta_x_au, float delta_y_au);

    /// World position (km) relative to the camera origin, as float km.
    void world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                  float& z_km) const;

    /// Map a world position (km) to OpenGL NDC xy for the current framing.
    void world_to_ndc(const core::Displacement& position, float& x_ndc, float& y_ndc) const;

    /// View matrix (identity while translation is applied as camera-relative positions).
    [[nodiscard]] glm::mat4 view_matrix() const;

    /// Orthographic projection in camera-relative km (matches prior NDC framing).
    [[nodiscard]] glm::mat4 projection_matrix() const;

  private:
    float center_x_au_{0.0f};
    float center_y_au_{0.0f};
    float half_extent_au_{2.0f};
    float aspect_ratio_{1.0f};
};

} // namespace solar::app
