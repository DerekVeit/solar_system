#pragma once

#include "core/types.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace solar::app {

/// Scene-owned camera: position, orbit angles, and orthographic framing.
///
/// Free mode: eye (x,y,z) is authoritative; look along yaw/pitch.
/// Follow mode: target is authoritative; eye = target - radius * forward(yaw, pitch).
/// Geometry is emitted in camera-relative km; V is the rotation from lookAt.
class Camera {
  public:
    static constexpr float kDefaultRadiusAu = 5.0f;
    static constexpr float kDefaultPitchRad = -1.57079632679f; // -pi/2: look toward -Z from above
    static constexpr float kYawPitchStepRad = 0.0872664626f;   // 5 degrees

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

    /// Free-camera eye in AU (ignored while following until release).
    void set_eye_au(float x_au, float y_au, float z_au);
    void pan_eye_au(float delta_x_au, float delta_y_au, float delta_z_au);

    /// Follow look-at point in AU (body + offset). Call each frame while following.
    void set_follow_target_au(float x_au, float y_au, float z_au);
    void clear_follow();
    [[nodiscard]] bool following() const { return following_; }

    /// Copy resolved eye into free-eye state (e.g. when releasing follow).
    void capture_eye_from_resolved();

    void reset_to_default_view();

    /// Resolve eye/target from mode, then map world km → camera-relative float km.
    void world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                  float& z_km) const;

    [[nodiscard]] glm::mat4 view_matrix() const;
    [[nodiscard]] glm::mat4 projection_matrix() const;

    /// Unit axes of the current view (after resolve): right, up, forward (toward target).
    void view_basis(glm::vec3& right_au, glm::vec3& up_au, glm::vec3& forward_au) const;

  private:
    struct Resolved {
        glm::vec3 eye_au{0.0f, 0.0f, kDefaultRadiusAu};
        glm::vec3 target_au{0.0f, 0.0f, 0.0f};
    };

    [[nodiscard]] glm::vec3 forward_from_angles() const;
    [[nodiscard]] Resolved resolve() const;
    void clamp_pitch();

    float eye_x_au_{0.0f};
    float eye_y_au_{0.0f};
    float eye_z_au_{kDefaultRadiusAu};

    float follow_target_x_au_{0.0f};
    float follow_target_y_au_{0.0f};
    float follow_target_z_au_{0.0f};
    bool following_{false};

    float yaw_rad_{0.0f};
    float pitch_rad_{kDefaultPitchRad};
    float radius_au_{kDefaultRadiusAu};

    float half_extent_au_{2.0f};
    float aspect_ratio_{1.0f};
};

} // namespace solar::app
