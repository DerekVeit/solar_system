#include "app/scene/body_visual.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/types.hpp"
#include "glm/geometric.hpp"

#include <algorithm>

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

[[nodiscard]] SphereInstance to_sphere_instance(const core::Displacement& position, Color color,
                                                float ambient, float emission, glm::vec3 light_dir,
                                                float radius_km, const ViewFrame& view) {
    const LineVertex vertex = to_line_vertex(position, color, view);
    return SphereInstance{vertex.x_km, vertex.y_km, vertex.z_km, radius_km,
                          color,       ambient,     emission,    light_dir};
}

void append_orbit_loop(const sim::SolarSystem& simulation, const core::BodyDefinition& body,
                       double mu, const ViewFrame& view, LinePrimitive& loop) {
    const double period_seconds = core::orbital_period_seconds(mu, body.elements);
    if (period_seconds <= 0.0) {
        return;
    }

    const core::Epoch epoch_now = simulation.clock().epoch();
    loop.vertices.reserve(BodyVisual::kOrbitSamples);

    for (std::size_t sample = 0; sample < BodyVisual::kOrbitSamples; ++sample) {
        const double fraction =
            static_cast<double>(sample) / static_cast<double>(BodyVisual::kOrbitSamples);
        const double elapsed_seconds = period_seconds * fraction;
        const core::Epoch sample_epoch{epoch_now.jd -
                                       (period_seconds - elapsed_seconds) / core::kSecondsPerDay};
        const core::Displacement position =
            simulation.ephemeris().state(body.name, sample_epoch).position;
        loop.vertices.push_back(to_line_vertex(position, BodyVisual::kOrbitTrailColor, view));
    }
}

void append_tail(const sim::SolarSystem& simulation, const core::BodyDefinition& body, double mu,
                 const ViewFrame& view, double tail_duration_seconds, LinePrimitive& trail) {
    if (tail_duration_seconds <= 0.0 || BodyVisual::kTailSamples < 2) {
        return;
    }

    const core::Epoch epoch_now = simulation.clock().epoch();
    const double mean_anomaly_now = core::mean_anomaly_at_epoch(mu, body.elements, epoch_now);
    const double mean_motion_value = core::mean_motion(mu, body.elements);
    if (mean_motion_value <= 0.0) {
        return;
    }

    const double tail_mean_anomaly_span = mean_motion_value * tail_duration_seconds;
    trail.vertices.reserve(BodyVisual::kTailSamples);

    for (std::size_t sample = 0; sample < BodyVisual::kTailSamples; ++sample) {
        const double fraction =
            static_cast<double>(sample) / static_cast<double>(BodyVisual::kTailSamples - 1);
        const double target_mean_anomaly =
            core::normalize_angle(mean_anomaly_now - tail_mean_anomaly_span * fraction);
        const core::Epoch sample_epoch =
            core::epoch_before_mean_anomaly(mu, body.elements, epoch_now, target_mean_anomaly);
        const core::Displacement position =
            simulation.ephemeris().state(body.name, sample_epoch).position;
        Color tail_color = BodyVisual::kTailColor;
        tail_color.a = BodyVisual::kTailColor.a * static_cast<float>(1.0 - fraction);
        trail.vertices.push_back(to_line_vertex(position, tail_color, view));
    }
}

} // namespace

BodyVisual::BodyVisual(std::string name, Color color, float ambient, float emission,
                       double radius_km, double tail_duration_days, float display_size_factor,
                       bool draws_orbit_trails)
    : name_(std::move(name))
    , color_(color)
    , ambient_(ambient)
    , emission_(emission)
    , radius_km_(radius_km)
    , tail_duration_seconds_(tail_duration_days * core::kSecondsPerDay)
    , display_size_factor_(display_size_factor)
    , draws_orbit_trails_(draws_orbit_trails) {}

float BodyVisual::drawn_radius_km(const ViewFrame& view) const {
    const float half_extent_au = view.camera.half_extent_au();
    const float size_factor = view.body_scaling ? display_size_factor_ : 1.0f;
    if (radius_km_ <= 0.0 || half_extent_au <= 0.0f || view.framebuffer_height <= 0 ||
        size_factor <= 0.0f) {
        return 0.0f;
    }

    float radius_km = static_cast<float>(radius_km_) * size_factor;

    // Keep a ~2px screen diameter floor (same idea as the old point-sprite minimum).
    // Ortho: full height is 2 * half_extent; diameter_px = radius * height / half_extent_km.
    const float half_extent_km = half_extent_au * static_cast<float>(core::kAuKm);
    const float min_radius_km =
        (kMinScreenDiameterPx * half_extent_km) / static_cast<float>(view.framebuffer_height);
    return std::max(radius_km, min_radius_km);
}

void BodyVisual::append_draw(const sim::SolarSystem& simulation, const ViewFrame& view,
                             DrawBatch& batch) const {
    const float radius_km = drawn_radius_km(view);
    const core::Displacement position = simulation.state(name_).position;
    const core::Displacement sun_position = simulation.state("Sun").position;
    const glm::vec3 light_dir = glm::normalize(sun_position.km - position.km);
    batch.spheres.push_back(
        to_sphere_instance(position, color_, ambient_, emission_, light_dir, radius_km, view));

    if (!draws_orbit_trails_) {
        return;
    }

    const core::BodyDefinition* body = core::find_body(simulation.ephemeris(), name_);
    if (body == nullptr) {
        return;
    }

    const double mu = core::central_gravitational_parameter(simulation.ephemeris());
    if (mu <= 0.0) {
        return;
    }

    LinePrimitive orbit_loop;
    append_orbit_loop(simulation, *body, mu, view, orbit_loop);
    if (!orbit_loop.vertices.empty()) {
        batch.line_loops.push_back(std::move(orbit_loop));
    }

    LinePrimitive tail_trail;
    append_tail(simulation, *body, mu, view, tail_duration_seconds_, tail_trail);
    if (!tail_trail.vertices.empty()) {
        batch.line_trails.push_back(std::move(tail_trail));
    }
}

} // namespace solar::app
