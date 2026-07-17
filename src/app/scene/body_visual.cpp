#include "app/scene/body_visual.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/types.hpp"

#include <algorithm>

namespace solar::app {

namespace {

[[nodiscard]] LineVertex to_line_vertex(const core::Displacement& position, Color color,
                                        const ViewFrame& view) {
    float x_ndc = 0.0f;
    float y_ndc = 0.0f;
    view.camera.world_to_ndc(position, x_ndc, y_ndc);
    return LineVertex{x_ndc, y_ndc, color};
}

[[nodiscard]] PointInstance to_point_instance(const core::Displacement& position, Color color,
                                              float point_size, const ViewFrame& view) {
    const LineVertex vertex = to_line_vertex(position, color, view);
    return PointInstance{vertex.x_ndc, vertex.y_ndc, color, point_size};
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
                 const ViewFrame& view, double tail_duration_seconds, LinePrimitive& trail,
                 std::vector<PointInstance>& points) {
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
        const LineVertex vertex = to_line_vertex(position, tail_color, view);
        trail.vertices.push_back(vertex);
        points.push_back(to_point_instance(position, tail_color, BodyVisual::kTailPointSize, view));
    }
}

} // namespace

BodyVisual::BodyVisual(std::string name, Color color, double radius_km, double tail_duration_days,
                       float display_size_factor, bool draws_orbit_trails)
    : name_(std::move(name))
    , color_(color)
    , radius_km_(radius_km)
    , tail_duration_seconds_(tail_duration_days * core::kSecondsPerDay)
    , display_size_factor_(display_size_factor)
    , draws_orbit_trails_(draws_orbit_trails) {}

float BodyVisual::point_size_pixels(const ViewFrame& view) const {
    const float half_extent_au = view.camera.half_extent_au();
    if (radius_km_ <= 0.0 || half_extent_au <= 0.0f || view.framebuffer_height <= 0) {
        return kMinPointSize;
    }

    const float scale_km = half_extent_au * static_cast<float>(core::kAuKm);
    const float diameter_ndc = static_cast<float>((2.0 * radius_km_) / scale_km);
    const float size_factor = view.body_scaling ? display_size_factor_ : 1.0f;
    const float point_size =
        diameter_ndc * static_cast<float>(view.framebuffer_height) * 0.5f * size_factor;
    return std::max(point_size, kMinPointSize);
}

void BodyVisual::append_draw(const sim::SolarSystem& simulation, const ViewFrame& view,
                             DrawBatch& batch) const {
    const float point_size = point_size_pixels(view);
    const core::Displacement position = simulation.state(name_).position;
    batch.points.push_back(to_point_instance(position, color_, point_size, view));

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
    append_tail(simulation, *body, mu, view, tail_duration_seconds_, tail_trail, batch.points);
    if (!tail_trail.vertices.empty()) {
        batch.line_trails.push_back(std::move(tail_trail));
    }
}

} // namespace solar::app