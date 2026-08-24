#include "app/scene/scene.hpp"

#include "app/logging.hpp"
#include "app/scene/body_visual.hpp"
#include "app/scene/sky_figure_lines.hpp"
#include "app/scene/star_markers.hpp"
#include "app/scene/texture.hpp"
#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/sky_orientation.hpp"

#include <algorithm>
#include <cmath>
#include <glm/ext/vector_double3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <optional>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>

namespace solar::app {

namespace {

std::unordered_set<std::string> texture_paths_of_bodies(std::span<BodyVisual> bodies) {
    std::unordered_set<std::string> paths{};
    for (const BodyVisual& body : bodies) {
        paths.merge(body.texture_paths());
    }
    return paths;
}

const std::optional<BodyVisual> find_body_visual(std::span<BodyVisual> bodies,
                                                 const std::string& name) {
    for (const BodyVisual& body : bodies) {
        if (body.name() == name) {
            return body;
        }
    }
    return std::nullopt;
}

} // namespace

Scene::Scene(std::unique_ptr<IRenderer> renderer)
    : renderer_(std::move(renderer)) {}

void Scene::add_body(BodyVisual body) { bodies_.push_back(std::move(body)); }

void Scene::set_sky(SkySpec spec) { sky_ = std::move(spec); }

void Scene::set_star_catalog(StarCatalog catalog) { star_catalog_ = std::move(catalog); }

void Scene::set_sky_figures(SkyFigureCatalog catalog) { sky_figures_ = std::move(catalog); }

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

    // Always reserve a star-marker loop so C can show them even if they start hidden.
    const std::size_t star_loop_vertices =
        star_catalog_.stars.empty() ? 0 : StarCatalog::kMarkerSamples;
    const RenderCapacity capacity{
        .max_spheres = bodies_.size(),
        .max_line_vertices = std::max(star_marker_line_vertex_capacity(star_catalog_),
                                      sky_figure_line_vertex_capacity(sky_figures_)),
        .max_line_trail_vertices = trail_bodies * BodyVisual::kTailSamples,
        .max_line_loop_vertices =
            std::max(trail_bodies * BodyVisual::kOrbitSamples, star_loop_vertices),
    };
    if (!renderer_->init(capacity)) {
        return false;
    }
    std::unordered_set<std::string> texture_paths = texture_paths_of_bodies(bodies_);
    for (std::string path : texture_paths) {
        TextureImage image = image_from_path(path);
        if (image.valid()) {
            renderer_->upload_texture(path, image);
        } else {
            log("failed to get valid image from path {}", path);
        }
    }

    if (sky_.visible && !sky_.texture.empty()) {
        TextureImage image = image_from_path(sky_.texture);
        if (image.valid()) {
            renderer_->upload_texture(sky_.texture, image, TextureFilter::linear);
            const glm::mat3 rotation =
                glm::mat3(core::tex_from_ecliptic(sky_.longitude_offset_deg));
            renderer_->set_sky(sky_.texture, rotation, sky_.brightness);
        } else {
            log("failed to get valid sky image from path {}", sky_.texture);
        }
    }
    return true;
}

void Scene::render(const sim::SolarSystem& simulation, float aspect_ratio, int framebuffer_height) {
    if (renderer_ == nullptr) {
        return;
    }

    camera_.set_aspect_ratio(aspect_ratio);
    update_camera_from_follow(simulation);

    DrawBatch batch;
    batch.spheres.reserve(bodies_.size());
    batch.line_loops.reserve(bodies_.size() + star_catalog_.stars.size());
    batch.line_trails.reserve(bodies_.size());
    batch.lines.reserve(star_catalog_.stars.size() + sky_figures_.figures.size());

    const ViewFrame view{
        .camera = camera_,
        .framebuffer_height = framebuffer_height,
        .body_scaling = body_scaling_,
    };

    for (const BodyVisual& body : bodies_) {
        body.append_draw(simulation, view, batch, graticules_);
    }
    const double marker_distance_km =
        static_cast<double>(Camera::kFarAu) * StarCatalog::kDistanceFarFraction * core::kAuKm;
    glm::vec3 view_right{};
    glm::vec3 view_up{};
    glm::vec3 view_forward{};
    camera_.view_basis(view_right, view_up, view_forward);
    append_sky_figures(sky_figures_, star_catalog_, marker_distance_km, batch);
    append_star_markers(star_catalog_, marker_distance_km, glm::dvec3{view_right},
                        glm::dvec3{view_up}, batch);
    renderer_->draw(batch, camera_.view_matrix(), camera_.projection_matrix());
}

void Scene::set_view_half_extent_au(float half_extent_au) {
    camera_.set_half_extent_au(half_extent_au);
}

void Scene::body_scaling(bool enabled) { body_scaling_ = enabled; }

void Scene::set_star_markers_visible(bool visible) { star_catalog_.visible = visible; }

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

    camera_.pan_target_au(dx, dy, dz);
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
        follow_offset_x_au_ = 0.0f;
        follow_offset_y_au_ = 0.0f;
        follow_offset_z_au_ = 0.0f;
    }

    return previously_followed_body;
}

void Scene::add_yaw(float delta_rad) { camera_.add_yaw(delta_rad); }

void Scene::add_pitch(float delta_rad) { camera_.add_pitch(delta_rad); }

void Scene::reset_orientation() { camera_.reset_orientation(); }

void Scene::set_view_from_north() { camera_.set_view_from_north(); }

void Scene::set_view_from_south() { camera_.set_view_from_south(); }

void Scene::set_view_from_ypos() { camera_.set_view_from_ypos(); }

void Scene::set_view_from_yneg() { camera_.set_view_from_yneg(); }

void Scene::set_view_from_east() { camera_.set_view_from_east(); }

void Scene::set_view_from_west() { camera_.set_view_from_west(); }

void Scene::zoom_on_followed_body() {
    if (!followed_body_) {
        log("zoom_on_followed_body: not following any body");
        return;
    }
    auto body = find_body_visual(bodies_, *followed_body_);
    if (!body) {
        log("zoom_on_followed_body: body not found in scene.bodies: {}", *followed_body_);
        return;
    }
    const float body_radius_au = body->display_size_km(body_scaling_) / core::kAuKm;
    const float camera_radius_au =
        body_radius_au / std::sin(1.00f * glm::radians(Camera::kFov) / 2.0f);
    camera_.set_radius_au(camera_radius_au);
    log("camera_radius_au: {}", camera_.radius_au());
}

void Scene::zoom_in() { camera_.set_radius_au(camera_.radius_au() / Camera::kZoomFactor); }

void Scene::zoom_out() { camera_.set_radius_au(camera_.radius_au() * Camera::kZoomFactor); }

void Scene::update_camera_from_follow(const sim::SolarSystem& simulation) {
    if (!followed_body_) {
        return;
    }

    if (core::find_body(simulation.ephemeris(), *followed_body_) == nullptr) {
        log("follow target no longer in catalog; clearing follow: {}", *followed_body_);
        followed_body_ = std::nullopt;
        follow_offset_x_au_ = 0.0f;
        follow_offset_y_au_ = 0.0f;
        follow_offset_z_au_ = 0.0f;
        return;
    }

    core::Displacement position = simulation.state(*followed_body_).position;
    if (const auto visual = find_body_visual(bodies_, *followed_body_)) {
        position = visual->drawn_position(simulation, body_scaling_);
    }
    camera_.set_target_au(position.km.x / core::kAuKm + follow_offset_x_au_,
                          position.km.y / core::kAuKm + follow_offset_y_au_,
                          position.km.z / core::kAuKm + follow_offset_z_au_);
}

} // namespace solar::app
