#include "app/scene/camera.hpp"

#include "core/constants.hpp"

#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

namespace solar::app {

void Camera::set_half_extent_au(float half_extent_au) {
    if (half_extent_au > 0.0f) {
        half_extent_au_ = half_extent_au;
    }
}

void Camera::set_aspect_ratio(float aspect_ratio) {
    aspect_ratio_ = aspect_ratio > 0.0f ? aspect_ratio : 1.0f;
}

float Camera::view_width_au() const { return 2.0f * half_extent_au_ * aspect_ratio_; }

float Camera::view_height_au() const { return 2.0f * half_extent_au_; }

void Camera::set_radius_au(float radius_au) { radius_au_ = radius_au; }

void Camera::clamp_pitch() {
    if (pitch_rad_ > kPitchLimitRad) {
        pitch_rad_ = kPitchLimitRad;
    } else if (pitch_rad_ < -kPitchLimitRad) {
        pitch_rad_ = -kPitchLimitRad;
    }
}

void Camera::add_yaw(float delta_rad) { yaw_rad_ += delta_rad; }

void Camera::add_pitch(float delta_rad) {
    pitch_rad_ += delta_rad;
    clamp_pitch();
}

void Camera::reset_orientation() {
    yaw_rad_ = kDefaultYawRad;
    pitch_rad_ = kDefaultPitchRad;
    radius_au_ = kDefaultRadiusAu;
}

void Camera::pan_free_target_au(float delta_x_au, float delta_y_au, float delta_z_au) {
    free_target_x_au_ += delta_x_au;
    free_target_y_au_ += delta_y_au;
    free_target_z_au_ += delta_z_au;
}

void Camera::set_follow_target_au(float x_au, float y_au, float z_au) {
    follow_target_x_au_ = x_au;
    follow_target_y_au_ = y_au;
    follow_target_z_au_ = z_au;
    following_ = true;
}

void Camera::clear_follow() { following_ = false; }

void Camera::capture_free_target_from_resolved() {
    const Resolved resolved = resolve();
    free_target_x_au_ = resolved.target_au.x;
    free_target_y_au_ = resolved.target_au.y;
    free_target_z_au_ = resolved.target_au.z;
}

void Camera::reset_to_default_view() {
    following_ = false;
    reset_orientation();
    free_target_x_au_ = 0.0f;
    free_target_y_au_ = 0.0f;
    free_target_z_au_ = 0.0f;
    follow_target_x_au_ = 0.0f;
    follow_target_y_au_ = 0.0f;
    follow_target_z_au_ = 0.0f;
}

glm::vec3 Camera::forward_from_angles() const {
    const float cos_pitch = std::cos(pitch_rad_);
    return glm::vec3{cos_pitch * std::cos(yaw_rad_), cos_pitch * std::sin(yaw_rad_),
                     std::sin(pitch_rad_)};
}

Camera::Basis Camera::basis_from_angles() const {
    // yaw starts at π/2 (0 from forward)
    const float yaw_right = yaw_rad_ - kDefaultYawRad;
    const glm::vec3 right{std::cos(yaw_right), std::sin(yaw_right), 0.0f};
    const glm::vec3 forward = forward_from_angles();
    const glm::vec3 up = glm::normalize(glm::cross(right, forward));
    return Basis{right, up, forward};
}

Camera::Resolved Camera::resolve() const {
    Resolved resolved;
    if (following_) {
        resolved.target_au =
            glm::vec3{follow_target_x_au_, follow_target_y_au_, follow_target_z_au_};
    } else {
        resolved.target_au = glm::vec3{free_target_x_au_, free_target_y_au_, free_target_z_au_};
    }
    resolved.eye_au = resolved.target_au - forward_from_angles() * radius_au_;
    return resolved;
}

void Camera::world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                      float& z_km) const {
    const Resolved resolved = resolve();
    const float eye_x_km = resolved.eye_au.x * static_cast<float>(core::kAuKm);
    const float eye_y_km = resolved.eye_au.y * static_cast<float>(core::kAuKm);
    const float eye_z_km = resolved.eye_au.z * static_cast<float>(core::kAuKm);
    x_km = static_cast<float>(position.km.x) - eye_x_km;
    y_km = static_cast<float>(position.km.y) - eye_y_km;
    z_km = static_cast<float>(position.km.z) - eye_z_km;
}

void Camera::view_basis(glm::vec3& right_au, glm::vec3& up_au, glm::vec3& forward_au) const {
    const Basis basis = basis_from_angles();
    right_au = basis.right;
    up_au = basis.up;
    forward_au = basis.forward;
}

glm::mat4 Camera::view_matrix() const {
    const Basis basis = basis_from_angles();
    // Eye-relative positions: rotation only (eye at the origin).
    return glm::lookAt(glm::vec3{0.0f}, basis.forward, basis.up);
}

glm::mat4 Camera::projection_matrix() const {
    const float au_km = static_cast<float>(core::kAuKm);
    float near_km = 0.01f * au_km;
    if (radius_au_ < 1.0f) {
        near_km = 0.01f * radius_au_ * au_km;
    }
    const float far_km = 100.0f * au_km;
    const float fov = glm::radians(45.0f);
    return glm::perspective(fov, aspect_ratio_, near_km, far_km);
}

} // namespace solar::app
