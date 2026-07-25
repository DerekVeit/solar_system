#pragma once

#include "core/constants.hpp"
#include "core/types.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace solar::app {

/// Scene-owned camera: orbit look-at + yaw/pitch/radius.
///
/// Single look-at target; eye = target - radius * forward(yaw, pitch).
/// Free pan moves the target; follow overwrites it each frame (body + offset).
/// Unfollow leaves the target where it is.
/// View basis: yaw sets a horizontal right, forward from yaw/pitch, up = right × forward.
/// Geometry is eye-relative km.
class Camera {
  public:
    static constexpr float kDefaultRadiusAu = 5.0f;            // distance from target
    static constexpr float kDefaultYawRad = core::kPi / 2;     // π/2 = pointed at world +Y
    static constexpr float kDefaultPitchRad = -core::kPi / 24; // slightly below the ecliptic
    static constexpr float kYawPitchStepRad = core::kPi / 360; // 0.5°
    static constexpr float kPitchLimitRad = core::kPi / 2; // no further than straight up or down
    static constexpr float kZoomFactor = 1.02f;
    static constexpr float kPanFraction = 0.005;
    static constexpr float kFov = 45.0f; // field of view, degrees

    // Absolute orientations: look at the target from this celestial direction (ecliptic frame).
    // North/south: ±Z. East/west: ±X with +Z up (pitch 0).

    [[nodiscard]] float half_extent_au() const { return half_extent_au_; }
    void set_half_extent_au(float half_extent_au);

    [[nodiscard]] float aspect_ratio() const { return aspect_ratio_; }
    void set_aspect_ratio(float aspect_ratio);

    [[nodiscard]] float view_width_au() const;
    [[nodiscard]] float view_height_au() const;

    [[nodiscard]] float yaw_rad() const { return yaw_rad_; }
    [[nodiscard]] float pitch_rad() const { return pitch_rad_; }
    [[nodiscard]] float radius_au() const { return radius_au_; }
    void set_radius_au(float radius_au);

    void add_yaw(float delta_rad);
    void add_pitch(float delta_rad);
    void reset_orientation();

    /// Set yaw/pitch to view the look-at target from a fixed ecliptic direction.
    void set_view_from_north();
    void set_view_from_south();
    void set_view_from_ypos();
    void set_view_from_yneg();
    void set_view_from_east();
    void set_view_from_west();

    /// Pan the look-at target in AU (free camera).
    void pan_target_au(double delta_x_au, double delta_y_au, double delta_z_au);

    /// Set the look-at point in AU (e.g. each frame while following a body).
    /// Uses double so outer-system follow stays stable when zoomed in (float AU is too coarse).
    void set_target_au(double x_au, double y_au, double z_au);

    void reset_to_default_view();

    void world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                  float& z_km) const;

    [[nodiscard]] glm::mat4 view_matrix() const;
    [[nodiscard]] glm::mat4 projection_matrix() const;

    /// Unit view axes: right, up, forward (toward target).
    void view_basis(glm::vec3& right_au, glm::vec3& up_au, glm::vec3& forward_au) const;

  private:
    struct Resolved {
        glm::dvec3 eye_au{0.0, 0.0, kDefaultRadiusAu};
        glm::dvec3 target_au{0.0, 0.0, 0.0};
    };

    struct Basis {
        glm::vec3 right{1.0f, 0.0f, 0.0f};
        glm::vec3 up{0.0f, 1.0f, 0.0f};
        glm::vec3 forward{0.0f, 0.0f, -1.0f};
    };

    [[nodiscard]] glm::vec3 forward_from_angles() const;
    [[nodiscard]] Basis basis_from_angles() const;
    [[nodiscard]] Resolved resolve() const;
    void clamp_pitch();

    // Double look-at: float lacks precision at ~tens of AU for close-up follow.
    double target_x_au_{0.0};
    double target_y_au_{0.0};
    double target_z_au_{0.0};

    float yaw_rad_{kDefaultYawRad};
    float pitch_rad_{kDefaultPitchRad};
    float radius_au_{kDefaultRadiusAu};

    float half_extent_au_{2.0f};
    float aspect_ratio_{1.0f};
};

} // namespace solar::app
