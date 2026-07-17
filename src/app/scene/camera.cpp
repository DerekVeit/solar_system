#include "app/scene/camera.hpp"

#include "core/constants.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace solar::app {

void Camera::set_center_au(float x_au, float y_au) {
    center_x_au_ = x_au;
    center_y_au_ = y_au;
}

void Camera::reset_center() {
    center_x_au_ = 0.0f;
    center_y_au_ = 0.0f;
}

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

void Camera::pan_au(float delta_x_au, float delta_y_au) {
    center_x_au_ += delta_x_au;
    center_y_au_ += delta_y_au;
}

void Camera::world_to_camera_relative(const core::Displacement& position, float& x_km, float& y_km,
                                      float& z_km) const {
    const float center_x_km = center_x_au_ * static_cast<float>(core::kAuKm);
    const float center_y_km = center_y_au_ * static_cast<float>(core::kAuKm);
    x_km = static_cast<float>(position.km.x) - center_x_km;
    y_km = static_cast<float>(position.km.y) - center_y_km;
    z_km = static_cast<float>(position.km.z);
}

void Camera::world_to_ndc(const core::Displacement& position, float& x_ndc, float& y_ndc) const {
    const float scale_km = half_extent_au_ * static_cast<float>(core::kAuKm);
    const float center_x_km = center_x_au_ * static_cast<float>(core::kAuKm);
    const float center_y_km = center_y_au_ * static_cast<float>(core::kAuKm);
    x_ndc = (static_cast<float>(position.km.x) - center_x_km) / scale_km / aspect_ratio_;
    y_ndc = (static_cast<float>(position.km.y) - center_y_km) / scale_km;
}

glm::mat4 Camera::view_matrix() const {
    // Translation is applied when building camera-relative vertex positions.
    return glm::mat4{1.0f};
}

glm::mat4 Camera::projection_matrix() const {
    const float half_height_km = half_extent_au_ * static_cast<float>(core::kAuKm);
    const float half_width_km = half_height_km * aspect_ratio_;
    // Deep enough for inclined orbits in camera-relative km.
    const float depth_km = static_cast<float>(core::kAuKm) * 50.0f;
    return glm::ortho(-half_width_km, half_width_km, -half_height_km, half_height_km, -depth_km,
                      depth_km);
}

} // namespace solar::app
