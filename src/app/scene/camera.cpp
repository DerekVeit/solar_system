#include "app/scene/camera.hpp"

#include "core/constants.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace solar::app {

namespace {

[[nodiscard]] glm::vec3 stable_up(const glm::vec3& eye_au, const glm::vec3& target_au) {
    const glm::vec3 forward = glm::normalize(target_au - eye_au);
    const glm::vec3 world_up{0.0f, 0.0f, 1.0f};
    if (std::abs(glm::dot(forward, world_up)) > 0.999f) {
        return glm::vec3{0.0f, 1.0f, 0.0f};
    }
    return world_up;
}

} // namespace

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

void Camera::clamp_pitch() {
    constexpr float kLimit = glm::half_pi<float>();
    if (pitch_rad_ > kLimit) {
        pitch_rad_ = kLimit;
    } else if (pitch_rad_ < -kLimit) {
        pitch_rad_ = -kLimit;
    }
}

void Camera::add_yaw(float delta_rad) { yaw_rad_ += delta_rad; }

void Camera::add_pitch(float delta_rad) {
    pitch_rad_ += delta_rad;
    clamp_pitch();
}

void Camera::reset_orientation() {
    yaw_rad_ = 0.0f;
    pitch_rad_ = kDefaultPitchRad;
    radius_au_ = kDefaultRadiusAu;
}

void Camera::set_eye_au(float x_au, float y_au, float z_au) {
    eye_x_au_ = x_au;
    eye_y_au_ = y_au;
    eye_z_au_ = z_au;
}

void Camera::pan_eye_au(float delta_x_au, float delta_y_au, float delta_z_au) {
    eye_x_au_ += delta_x_au;
    eye_y_au_ += delta_y_au;
    eye_z_au_ += delta_z_au;
}

void Camera::set_follow_target_au(float x_au, float y_au, float z_au) {
    follow_target_x_au_ = x_au;
    follow_target_y_au_ = y_au;
    follow_target_z_au_ = z_au;
    following_ = true;
}

void Camera::clear_follow() { following_ = false; }

void Camera::capture_eye_from_resolved() {
    const Resolved resolved = resolve();
    eye_x_au_ = resolved.eye_au.x;
    eye_y_au_ = resolved.eye_au.y;
    eye_z_au_ = resolved.eye_au.z;
}

void Camera::reset_to_default_view() {
    following_ = false;
    reset_orientation();
    eye_x_au_ = 0.0f;
    eye_y_au_ = 0.0f;
    eye_z_au_ = radius_au_;
    follow_target_x_au_ = 0.0f;
    follow_target_y_au_ = 0.0f;
    follow_target_z_au_ = 0.0f;
}

glm::vec3 Camera::forward_from_angles() const {
    const float cos_pitch = std::cos(pitch_rad_);
    return glm::vec3{cos_pitch * std::cos(yaw_rad_), cos_pitch * std::sin(yaw_rad_),
                     std::sin(pitch_rad_)};
}

Camera::Resolved Camera::resolve() const {
    Resolved resolved;
    const glm::vec3 forward = forward_from_angles();

    if (following_) {
        resolved.target_au =
            glm::vec3{follow_target_x_au_, follow_target_y_au_, follow_target_z_au_};
        resolved.eye_au = resolved.target_au - forward * radius_au_;
    } else {
        resolved.eye_au = glm::vec3{eye_x_au_, eye_y_au_, eye_z_au_};
        resolved.target_au = resolved.eye_au + forward;
    }

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
    const Resolved resolved = resolve();
    forward_au = glm::normalize(resolved.target_au - resolved.eye_au);
    const glm::vec3 up_hint = stable_up(resolved.eye_au, resolved.target_au);
    right_au = glm::normalize(glm::cross(forward_au, up_hint));
    up_au = glm::normalize(glm::cross(right_au, forward_au));
}

glm::mat4 Camera::view_matrix() const {
    const Resolved resolved = resolve();
    const glm::vec3 eye_km = resolved.eye_au * static_cast<float>(core::kAuKm);
    const glm::vec3 target_km = resolved.target_au * static_cast<float>(core::kAuKm);
    const glm::vec3 up = stable_up(resolved.eye_au, resolved.target_au);
    // Vertices are already eye-relative; use lookAt from the origin.
    return glm::lookAt(glm::vec3{0.0f}, target_km - eye_km, up);
}

glm::mat4 Camera::projection_matrix() const {
    const float half_height_km = half_extent_au_ * static_cast<float>(core::kAuKm);
    const float half_width_km = half_height_km * aspect_ratio_;
    const float depth_km = static_cast<float>(core::kAuKm) * 50.0f;
    return glm::ortho(-half_width_km, half_width_km, -half_height_km, half_height_km, -depth_km,
                      depth_km);
}

} // namespace solar::app
