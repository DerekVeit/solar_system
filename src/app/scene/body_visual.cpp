#include "app/scene/body_visual.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/types.hpp"

#include <algorithm>

namespace solar::app {

namespace {

[[nodiscard]] float ndc_scale(float view_half_extent_au) {
    return view_half_extent_au * static_cast<float>(core::kAuKm);
}

[[nodiscard]] float ndc_aspect(float aspect_ratio) {
    return aspect_ratio > 0.0f ? aspect_ratio : 1.0f;
}

[[nodiscard]] LineVertex to_line_vertex(const core::Displacement& position, Color color,
                                        float view_half_extent_au, float aspect_ratio) {
    const float scale = ndc_scale(view_half_extent_au);
    const float aspect = ndc_aspect(aspect_ratio);
    return LineVertex{static_cast<float>(position.km.x) / scale / aspect,
                      static_cast<float>(position.km.y) / scale, color};
}

[[nodiscard]] PointInstance to_point_instance(const core::Displacement& position, Color color,
                                              float point_size, float view_half_extent_au,
                                              float aspect_ratio) {
    const LineVertex vertex = to_line_vertex(position, color, view_half_extent_au, aspect_ratio);
    return PointInstance{vertex.x_ndc, vertex.y_ndc, color, point_size};
}

void append_orbit_loop(const sim::SolarSystem& simulation, const core::BodyDefinition& body,
                       double mu, float view_half_extent_au, float aspect_ratio,
                       LinePrimitive& loop) {
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
        loop.vertices.push_back(to_line_vertex(position, BodyVisual::kOrbitTrailColor,
                                               view_half_extent_au, aspect_ratio));
    }
}

void append_tail(const sim::SolarSystem& simulation, const core::BodyDefinition& body, double mu,
                 float view_half_extent_au, float aspect_ratio, double tail_duration_seconds,
                 LinePrimitive& trail, std::vector<PointInstance>& points) {
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
        const LineVertex vertex =
            to_line_vertex(position, tail_color, view_half_extent_au, aspect_ratio);
        trail.vertices.push_back(vertex);
        points.push_back(to_point_instance(position, tail_color, BodyVisual::kTailPointSize,
                                           view_half_extent_au, aspect_ratio));
    }
}

} // namespace

BodyVisual::BodyVisual(std::string name, Color color, double radius_km, double tail_duration_days,
                       float display_size_factor)
    : name_(std::move(name))
    , color_(color)
    , radius_km_(radius_km)
    , tail_duration_seconds_(tail_duration_days * core::kSecondsPerDay)
    , display_size_factor_(display_size_factor) {}

float BodyVisual::point_size_pixels(float view_half_extent_au, int framebuffer_height) const {
    if (radius_km_ <= 0.0 || view_half_extent_au <= 0.0f || framebuffer_height <= 0) {
        return kMinPointSize;
    }

    const float scale_km = view_half_extent_au * static_cast<float>(core::kAuKm);
    const float diameter_ndc = static_cast<float>((2.0 * radius_km_) / scale_km);
    const float point_size =
        diameter_ndc * static_cast<float>(framebuffer_height) * 0.5f * display_size_factor_;
    return std::max(point_size, kMinPointSize);
}

void BodyVisual::append_draw(const sim::SolarSystem& simulation, float view_half_extent_au,
                             float aspect_ratio, int framebuffer_height, DrawBatch& batch) const {
    const float point_size = point_size_pixels(view_half_extent_au, framebuffer_height);

    if (name_ == "Sun") {
        batch.points.push_back(PointInstance{0.0f, 0.0f, color_, point_size});
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

    const core::Displacement position = simulation.state(name_).position;
    batch.points.push_back(
        to_point_instance(position, color_, point_size, view_half_extent_au, aspect_ratio));

    LinePrimitive orbit_loop;
    append_orbit_loop(simulation, *body, mu, view_half_extent_au, aspect_ratio, orbit_loop);
    if (!orbit_loop.vertices.empty()) {
        batch.line_loops.push_back(std::move(orbit_loop));
    }

    LinePrimitive tail_trail;
    append_tail(simulation, *body, mu, view_half_extent_au, aspect_ratio, tail_duration_seconds_,
                tail_trail, batch.points);
    if (!tail_trail.vertices.empty()) {
        batch.line_trails.push_back(std::move(tail_trail));
    }
}

} // namespace solar::app