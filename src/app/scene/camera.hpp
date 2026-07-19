#pragma once

#include "core/constants.hpp"
#include "core/types.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace solar::app {

/// Scene-owned camera: orbit look-at + yaw/pitch/radius, orthographic framing.
///
/// Free and follow both use a look-at target; eye = target - radius * forward(yaw, pitch).
/// Follow updates the target from a body; free pans the target.
/// View basis: yaw sets a horizontal right, forward from yaw/pitch, up = right × forward.
/// Geometry is eye-relative km.
class Camera {
  public:
    static constexpr float kDefaultRadiusAu = 5.0f;           // distance from target
    static constexpr float kDefaultYawRad = core::kPi / 2;    // π/2 = pointed at world +Y
    static constexpr float kDefaultPitchRad = -core::kPi / 2; // -π/2 = pointed down, world -Z
    static constexpr float kYawPitchStepRad = core::kPi / 36; // 5°
    static constexpr float kPitchLimitRad = core::kPi / 2;    // no further than straight up or down

    [[nodiscard]] float half_extent_au() const { return half_extent_au_; }
    void set_half_extent_au(float half_extent_au);

    [[nodiscard]] float aspect_ratio() const { return aspect_ratio_; }
    void set_aspect_ratio(float aspect_ratio);

    [[nodiscard]] float view_width_au() const;
    [[nodiscard]] float view_height_au() const;

    [[nodiscard]] float yaw_rad() const { return yaw_rad_; }
    [[nodiscard]] float pitch_rad() const { return pitch_rad_; }
    [[nodiscard]] float radius_au() const { return radius_au_; }

    void add_yaw(float delta_rad);
    void add_pitch(float delta_rad);
    void reset_orientation();

    /// Free look-at target in AU (used when not following).
    void pan_free_target_au(float delta_x_au, float delta_y_au, float delta_z_au);

    /// Follow look-at point in AU (body + offset). Call each frame while following.
    void set_follow_target_au(float x_au, float y_au, float z_au);
    void clear_follow();
    [[nodiscard]] bool following() const { return following_; }

    /// Keep framing after releasing follow: free target becomes the current look-at.
    void capture_free_target_from_resolved();

    void reset_to_default_view();

    void world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                  float& z_km) const;

    [[nodiscard]] glm::mat4 view_matrix() const;
    [[nodiscard]] glm::mat4 projection_matrix() const;

    /// Unit view axes: right, up, forward (toward target).
    void view_basis(glm::vec3& right_au, glm::vec3& up_au, glm::vec3& forward_au) const;

  private:
    struct Resolved {
        glm::vec3 eye_au{0.0f, 0.0f, kDefaultRadiusAu};
        glm::vec3 target_au{0.0f, 0.0f, 0.0f};
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

    float free_target_x_au_{0.0f};
    float free_target_y_au_{0.0f};
    float free_target_z_au_{0.0f};

    float follow_target_x_au_{0.0f};
    float follow_target_y_au_{0.0f};
    float follow_target_z_au_{0.0f};
    bool following_{false};

    float yaw_rad_{kDefaultYawRad};
    float pitch_rad_{kDefaultPitchRad};
    float radius_au_{kDefaultRadiusAu};

    float half_extent_au_{2.0f};
    float aspect_ratio_{1.0f};
};

} // namespace solar::app
