#include "app/scene/star_markers.hpp"

#include "app/scene/stroke_font.hpp"
#include "core/constants.hpp"
#include "core/sky_orientation.hpp"

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace solar::app {

namespace {

[[nodiscard]] bool tangent_axes(const glm::dvec3& dir, const glm::dvec3& view_right,
                                const glm::dvec3& view_up, glm::dvec3& right, glm::dvec3& up) {
    right = view_right - glm::dot(view_right, dir) * dir;
    if (glm::dot(right, right) < 1e-12) {
        right = view_up - glm::dot(view_up, dir) * dir;
    }
    if (glm::dot(right, right) < 1e-12) {
        return false;
    }
    right = glm::normalize(right);

    up = view_up - glm::dot(view_up, dir) * dir;
    up = up - glm::dot(up, right) * right;
    if (glm::dot(up, up) < 1e-12) {
        up = glm::cross(dir, right);
    }
    if (glm::dot(up, up) < 1e-12) {
        return false;
    }
    up = glm::normalize(up);
    return true;
}

void append_label(const Star& star, const glm::dvec3& dir, double distance_km, double radius_rad,
                  const glm::dvec3& view_right, const glm::dvec3& view_up, const Color& color,
                  DrawBatch& batch) {
    glm::dvec3 right{};
    glm::dvec3 up{};
    if (!tangent_axes(dir, view_right, view_up, right, up)) {
        return;
    }

    const double height_rad = StarCatalog::kLabelHeightDeg * core::kDegToRad;
    const double gap_rad = StarCatalog::kLabelGapDeg * core::kDegToRad;
    const double unit_km = distance_km * std::tan(height_rad / kStrokeFontCapHeight);
    const double offset_km = distance_km * std::tan(radius_rad + gap_rad);
    const glm::dvec3 origin =
        distance_km * dir + offset_km * right - 0.5 * kStrokeFontCapHeight * unit_km * up;

    std::vector<glm::dvec3> endpoints;
    endpoints.reserve(stroke_text_line_vertex_count(star.name));
    append_stroke_text(star.name, origin, right * unit_km, up * unit_km, endpoints);

    LinePrimitive label;
    label.vertices.reserve(endpoints.size());
    for (const glm::dvec3& point : endpoints) {
        const glm::dvec3 on_sphere = glm::normalize(point) * distance_km;
        label.vertices.push_back(LineVertex{static_cast<float>(on_sphere.x),
                                            static_cast<float>(on_sphere.y),
                                            static_cast<float>(on_sphere.z), color});
    }
    if (label.vertices.size() >= 2) {
        batch.lines.push_back(std::move(label));
    }
}

} // namespace

std::size_t star_marker_line_vertex_capacity(const StarCatalog& catalog) {
    std::size_t max_vertices = 0;
    for (const Star& star : catalog.stars) {
        max_vertices = std::max(max_vertices, stroke_text_line_vertex_count(star.name));
    }
    return max_vertices;
}

void append_star_markers(const StarCatalog& catalog, double distance_km,
                         const glm::dvec3& view_right, const glm::dvec3& view_up,
                         DrawBatch& batch) {
    if (!catalog.visible || distance_km <= 0.0) {
        return;
    }

    for (const Star& star : catalog.stars) {
        if (star.name.empty()) {
            continue;
        }
        const double radius_deg =
            star.marker_radius_deg > 0.0f ? star.marker_radius_deg : catalog.marker_radius_deg;
        const double radius_rad = radius_deg * core::kDegToRad;
        const glm::dvec3 dir = core::ecliptic_direction(star.ra_deg, star.dec_deg);
        const std::vector<glm::dvec3> points =
            core::directional_circle(dir, distance_km, radius_rad, StarCatalog::kMarkerSamples);

        LinePrimitive loop;
        loop.vertices.reserve(points.size());
        for (const glm::dvec3& point : points) {
            loop.vertices.push_back(LineVertex{static_cast<float>(point.x),
                                               static_cast<float>(point.y),
                                               static_cast<float>(point.z), catalog.color});
        }
        if (loop.vertices.size() >= 2) {
            batch.line_loops.push_back(std::move(loop));
        }
        append_label(star, dir, distance_km, radius_rad, view_right, view_up, catalog.color, batch);
    }
}

} // namespace solar::app
