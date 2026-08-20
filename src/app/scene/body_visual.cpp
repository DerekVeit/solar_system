#include "app/scene/body_visual.hpp"

#include "app/render/types.hpp"
#include "app/scene/body_visual_config.hpp"
#include "app/scene/body_visual_geometry.hpp"
#include "app/scene/view_frame.hpp"
#include "core/ephemeris.hpp"
#include "core/types.hpp"

#include <glm/ext/matrix_double3x3.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <unordered_set>

namespace solar::app {

namespace {

[[nodiscard]] LineVertex to_line_vertex(const core::Displacement& position, Color color,
                                        const ViewFrame& view) {
    float x_km = 0.0f;
    float y_km = 0.0f;
    float z_km = 0.0f;
    view.camera.world_to_camera_relative(position, x_km, y_km, z_km);
    return LineVertex{x_km, y_km, z_km, color};
}

[[nodiscard]] SphereInstance to_sphere_instance(const core::Displacement& position,
                                                const BodySurface& surface, glm::vec3 light_dir,
                                                bool show_graticules, float radius_km,
                                                glm::mat3 rotation, const ViewFrame& view) {
    const LineVertex vertex = to_line_vertex(position, surface.color, view);
    return SphereInstance{vertex.x_km, vertex.y_km, vertex.z_km, radius_km,
                          rotation,    surface,     light_dir,   show_graticules};
}

[[nodiscard]] RingInstance to_ring_instance(const core::Displacement& position, glm::vec3 light_dir,
                                            const RingSpec& spec, float size_factor,
                                            glm::mat3 rotation, const ViewFrame& view) {
    const LineVertex vertex = to_line_vertex(position, Color{0, 0, 0, 1}, view);
    const float factor = size_factor > 0.0f ? size_factor : 1.0f;
    return RingInstance{vertex.x_km,
                        vertex.y_km,
                        vertex.z_km,
                        static_cast<float>(spec.inner_radius_km * factor),
                        static_cast<float>(spec.outer_radius_km * factor),
                        rotation,
                        spec.map,
                        light_dir};
}

} // namespace

BodyVisual::BodyVisual(const core::BodyDefinition& body, BodyVisualSpec spec,
                       float orbit_display_size_factor)
    : name_(body.name)
    , primary_(body.primary)
    , surface_(spec.surface)
    , rings_(spec.rings)
    , radius_km_(body.radius_km)
    , tail_duration_seconds_(spec.tail_duration_days * core::kSecondsPerDay)
    , display_size_factor_(spec.display_size_factor)
    , orbit_display_size_factor_(orbit_display_size_factor)
    , draws_orbit_trails_(body.elements.semi_major_axis_km > 0.0) {}

core::Displacement BodyVisual::drawn_position(const sim::SolarSystem& simulation,
                                              bool body_scaling) const {
    return drawn_position(simulation, body_scaling, simulation.clock().epoch());
}

core::Displacement BodyVisual::drawn_position(const sim::SolarSystem& simulation, bool body_scaling,
                                              core::Epoch epoch) const {
    const double factor =
        orbit_display_factor(body_scaling, !primary_.empty(), orbit_display_size_factor_);
    return solar::app::drawn_position(simulation.ephemeris(), name_, epoch, factor);
}

float BodyVisual::drawn_radius_km(const ViewFrame& view) const {
    const float size_factor = view.body_scaling ? display_size_factor_ : 1.0f;
    const float radius_km = static_cast<float>(radius_km_) * size_factor;
    return std::max(radius_km, 0.0f);
}

void BodyVisual::append_draw(const sim::SolarSystem& simulation, const ViewFrame& view,
                             DrawBatch& batch, bool show_graticules) const {
    const float radius_km = drawn_radius_km(view);
    const core::Displacement physical = simulation.state(name_).position;
    const core::Displacement position = drawn_position(simulation, view.body_scaling);
    glm::vec3 light_dir{0.0f, 0.0f, 1.0f};
    if (surface_.emission < 1.0f) {
        const core::Displacement sun_position = simulation.state("Sun").position;
        const glm::dvec3 delta = sun_position.km - physical.km;
        const double len2 = glm::dot(delta, delta);
        if (len2 > 0.0) {
            light_dir = glm::vec3(glm::normalize(delta));
        }
    }
    // narrowing the base type from double to float (dmat3 → mat3) here for rendering
    const glm::mat3 rotation = simulation.orientation(name_);

    batch.spheres.push_back(to_sphere_instance(position, surface_, light_dir, show_graticules,
                                               radius_km, rotation, view));

    const float size_factor = view.body_scaling ? display_size_factor_ : 1.0f;
    for (const RingSpec& spec : rings_) {
        batch.rings.push_back(
            to_ring_instance(position, light_dir, spec, size_factor, rotation, view));
    }

    if (!draws_orbit_trails_) {
        return;
    }

    const core::BodyDefinition* body = core::find_body(simulation.ephemeris(), name_);
    if (body == nullptr) {
        return;
    }

    const double orbit_factor =
        orbit_display_factor(view.body_scaling, !body->primary.empty(), orbit_display_size_factor_);
    const core::Epoch epoch_now = simulation.clock().epoch();

    LinePrimitive orbit_loop;
    for (const core::Displacement& loop_position : orbit_loop_positions(
             simulation.ephemeris(), *body, epoch_now, orbit_factor, kOrbitSamples)) {
        orbit_loop.vertices.push_back(to_line_vertex(loop_position, kOrbitTrailColor, view));
    }
    if (!orbit_loop.vertices.empty()) {
        batch.line_loops.push_back(std::move(orbit_loop));
    }

    LinePrimitive tail_trail;
    const auto tail = tail_positions(simulation.ephemeris(), *body, epoch_now, orbit_factor,
                                     tail_duration_seconds_, kTailSamples);
    for (std::size_t sample = 0; sample < tail.size(); ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(tail.size() - 1);
        Color tail_color = kTailColor;
        tail_color.a = kTailColor.a * static_cast<float>(1.0 - fraction);
        tail_trail.vertices.push_back(to_line_vertex(tail[sample], tail_color, view));
    }
    if (!tail_trail.vertices.empty()) {
        batch.line_trails.push_back(std::move(tail_trail));
    }
}

std::unordered_set<std::string> BodyVisual::texture_paths() const {
    std::unordered_set<std::string> paths{};
    if (!surface_.textures.diffuse.empty()) {
        paths.insert(surface_.textures.diffuse);
    }
    if (!surface_.textures.night.empty()) {
        paths.insert(surface_.textures.night);
    }
    if (!surface_.textures.clouds.empty()) {
        paths.insert(surface_.textures.clouds);
    }
    if (!surface_.textures.normal.empty()) {
        paths.insert(surface_.textures.normal);
    }
    if (!surface_.textures.specular.empty()) {
        paths.insert(surface_.textures.specular);
    }
    for (const RingSpec& ring : rings_) {
        if (!ring.map.empty()) {
            paths.insert(ring.map);
        }
    }
    return paths;
}

} // namespace solar::app
