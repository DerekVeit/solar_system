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

void Camera::set_view_from_north() {
    // From +Z looking toward -Z (down onto the ecliptic).
    yaw_rad_ = kDefaultYawRad;
    pitch_rad_ = -kPitchLimitRad;
}

void Camera::set_view_from_south() {
    // From -Z looking toward +Z.
    yaw_rad_ = kDefaultYawRad;
    pitch_rad_ = kPitchLimitRad;
}

void Camera::set_view_from_ypos() {
    // From +Y looking toward -Y; +Z is up.
    yaw_rad_ = -core::kPi / 2;
    pitch_rad_ = 0.0f;
}

void Camera::set_view_from_yneg() {
    // From -Y looking toward +Y; +Z is up.
    yaw_rad_ = core::kPi / 2;
    pitch_rad_ = 0.0f;
}

void Camera::set_view_from_east() {
    // From +X looking toward -X; +Z is up.
    yaw_rad_ = core::kPi;
    pitch_rad_ = 0.0f;
}

void Camera::set_view_from_west() {
    // From -X looking toward +X; +Z is up.
    yaw_rad_ = 0.0f;
    pitch_rad_ = 0.0f;
}

void Camera::pan_target_au(double delta_x_au, double delta_y_au, double delta_z_au) {
    target_x_au_ += delta_x_au;
    target_y_au_ += delta_y_au;
    target_z_au_ += delta_z_au;
}

void Camera::set_target_au(double x_au, double y_au, double z_au) {
    target_x_au_ = x_au;
    target_y_au_ = y_au;
    target_z_au_ = z_au;
}

void Camera::reset_to_default_view() {
    reset_orientation();
    target_x_au_ = 0.0;
    target_y_au_ = 0.0;
    target_z_au_ = 0.0;
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
    resolved.target_au = glm::dvec3{target_x_au_, target_y_au_, target_z_au_};
    const glm::dvec3 forward{forward_from_angles()};
    resolved.eye_au = resolved.target_au - forward * static_cast<double>(radius_au_);
    return resolved;
}

void Camera::world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                      float& z_km) const {
    // Subtract in double so eye-relative km stay smooth far from the Sun; cast only the result.
    const Resolved resolved = resolve();
    const glm::dvec3 eye_km = resolved.eye_au * core::kAuKm;
    const glm::dvec3 relative_km = position.km - eye_km;
    x_km = static_cast<float>(relative_km.x);
    y_km = static_cast<float>(relative_km.y);
    z_km = static_cast<float>(relative_km.z);
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
        near_km = 0.1f * radius_au_ * au_km;
    }
    const float far_km = 150.0f * au_km;
    const float fov = glm::radians(kFov);
    return glm::perspective(fov, aspect_ratio_, near_km, far_km);
}

} // namespace solar::app
