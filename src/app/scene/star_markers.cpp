#include "app/scene/star_markers.hpp"

#include "core/constants.hpp"
#include "core/sky_orientation.hpp"

#include <vector>

namespace solar::app {

void append_star_markers(const StarCatalog& catalog, double distance_km, DrawBatch& batch) {
    if (!catalog.visible || distance_km <= 0.0) {
        return;
    }

    for (const Star& star : catalog.stars) {
        const double radius_deg =
            star.marker_radius_deg > 0.0f ? star.marker_radius_deg : catalog.marker_radius_deg;
        const glm::dvec3 dir = core::ecliptic_direction(star.ra_deg, star.dec_deg);
        const std::vector<glm::dvec3> points = core::directional_circle(
            dir, distance_km, radius_deg * core::kDegToRad, StarCatalog::kMarkerSamples);

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
    }
}

} // namespace solar::app
