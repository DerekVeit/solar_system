#include "app/scene/scene.hpp"

#include "app/logging.hpp"
#include "core/constants.hpp"
#include "core/ephemeris.hpp"

#include <glm/vec3.hpp>
#include <utility>

namespace solar::app {

Scene::Scene(std::unique_ptr<IRenderer> renderer)
    : renderer_(std::move(renderer)) {}

void Scene::add_body(BodyVisual body) { bodies_.push_back(std::move(body)); }

bool Scene::init() {
    if (renderer_ == nullptr) {
        return false;
    }

    std::size_t trail_bodies = 0;
    for (const BodyVisual& body : bodies_) {
        if (body.draws_orbit_trails()) {
            ++trail_bodies;
        }
    }

    const RenderCapacity capacity{
        .max_spheres = bodies_.size(),
        .max_line_vertices = 0,
        .max_line_trail_vertices = trail_bodies * BodyVisual::kTailSamples,
        .max_line_loop_vertices = trail_bodies * BodyVisual::kOrbitSamples,
    };
    return renderer_->init(capacity);
}

void Scene::render(const sim::SolarSystem& simulation, float aspect_ratio, int framebuffer_height) {
    if (renderer_ == nullptr) {
        return;
    }

    camera_.set_aspect_ratio(aspect_ratio);
    update_camera_from_follow(simulation);

    DrawBatch batch;
    batch.spheres.reserve(bodies_.size());
    batch.line_loops.reserve(bodies_.size());
    batch.line_trails.reserve(bodies_.size());

    const ViewFrame view{
        .camera = camera_,
        .framebuffer_height = framebuffer_height,
        .body_scaling = body_scaling_,
    };

    for (const BodyVisual& body : bodies_) {
        body.append_draw(simulation, view, batch);
    }
    renderer_->draw(batch, camera_.view_matrix(), camera_.projection_matrix());
}

void Scene::set_view_half_extent_au(float half_extent_au) {
    camera_.set_half_extent_au(half_extent_au);
}

void Scene::body_scaling(bool enabled) { body_scaling_ = enabled; }

void Scene::pan_view_fraction(float delta_x_fraction, float delta_y_fraction) {
    const float delta_x_au = delta_x_fraction * camera_.view_width_au();
    const float delta_y_au = delta_y_fraction * camera_.view_height_au();

    glm::vec3 right{};
    glm::vec3 up{};
    glm::vec3 forward{};
    camera_.view_basis(right, up, forward);
    const float dx = right.x * delta_x_au + up.x * delta_y_au;
    const float dy = right.y * delta_x_au + up.y * delta_y_au;
    const float dz = right.z * delta_x_au + up.z * delta_y_au;

    if (followed_body_) {
        follow_offset_x_au_ += dx;
        follow_offset_y_au_ += dy;
        follow_offset_z_au_ += dz;
        return;
    }

    camera_.pan_free_target_au(dx, dy, dz);
}

void Scene::reset_view_center() {
    followed_body_ = std::nullopt;
    follow_offset_x_au_ = 0.0f;
    follow_offset_y_au_ = 0.0f;
    follow_offset_z_au_ = 0.0f;
    camera_.reset_to_default_view();
}

void Scene::set_follow_target(const sim::SolarSystem& simulation,
                              std::optional<std::string> body_name) {
    if (!body_name) {
        camera_.capture_free_target_from_resolved();
        camera_.clear_follow();
        followed_body_ = std::nullopt;
        follow_offset_x_au_ = 0.0f;
        follow_offset_y_au_ = 0.0f;
        follow_offset_z_au_ = 0.0f;
        return;
    }

    if (core::find_body(simulation.ephemeris(), *body_name) == nullptr) {
        log("follow target not in catalog: {}", *body_name);
        return;
    }

    followed_body_ = std::move(body_name);
    follow_offset_x_au_ = 0.0f;
    follow_offset_y_au_ = 0.0f;
    follow_offset_z_au_ = 0.0f;
}

std::optional<std::string> Scene::release_from_follow() {
    auto previously_followed_body = std::exchange(followed_body_, std::nullopt);
    if (!previously_followed_body) {
        log("not following any target");
    } else {
        camera_.capture_free_target_from_resolved();
        camera_.clear_follow();
        follow_offset_x_au_ = 0.0f;
        follow_offset_y_au_ = 0.0f;
        follow_offset_z_au_ = 0.0f;
    }

    return previously_followed_body;
}

void Scene::add_yaw(float delta_rad) { camera_.add_yaw(delta_rad); }

void Scene::add_pitch(float delta_rad) { camera_.add_pitch(delta_rad); }

void Scene::reset_orientation() { camera_.reset_orientation(); }

void Scene::update_camera_from_follow(const sim::SolarSystem& simulation) {
    if (!followed_body_) {
        camera_.clear_follow();
        return;
    }

    if (core::find_body(simulation.ephemeris(), *followed_body_) == nullptr) {
        log("follow target no longer in catalog; clearing follow: {}", *followed_body_);
        followed_body_ = std::nullopt;
        camera_.capture_free_target_from_resolved();
        camera_.clear_follow();
        return;
    }

    const core::Displacement position = simulation.state(*followed_body_).position;
    camera_.set_follow_target_au(
        static_cast<float>(position.km.x / core::kAuKm) + follow_offset_x_au_,
        static_cast<float>(position.km.y / core::kAuKm) + follow_offset_y_au_,
        static_cast<float>(position.km.z / core::kAuKm) + follow_offset_z_au_);
}

} // namespace solar::app
