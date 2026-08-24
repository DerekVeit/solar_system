#include "app/scene/sky_figure_lines.hpp"

#include "core/constants.hpp"
#include "core/sky_orientation.hpp"

#include <algorithm>
#include <cmath>

namespace solar::app {

namespace {

void append_edge(const Star& from, const Star& to, double distance_km, double gap_rad,
                 const Color& color, LinePrimitive& primitive) {
    const glm::dvec3 a = core::ecliptic_direction(from.ra_deg, from.dec_deg);
    const glm::dvec3 b = core::ecliptic_direction(to.ra_deg, to.dec_deg);
    const auto inset = core::inset_great_circle(a, b, gap_rad);
    if (!inset) {
        return;
    }
    const glm::dvec3 start = distance_km * inset->first;
    const glm::dvec3 end = distance_km * inset->second;
    primitive.vertices.push_back(LineVertex{static_cast<float>(start.x),
                                            static_cast<float>(start.y),
                                            static_cast<float>(start.z), color});
    primitive.vertices.push_back(LineVertex{static_cast<float>(end.x), static_cast<float>(end.y),
                                            static_cast<float>(end.z), color});
}

} // namespace

std::size_t sky_figure_line_vertex_capacity(const SkyFigureCatalog& figures) {
    std::size_t max_vertices = 0;
    for (const SkyFigure& figure : figures.figures) {
        std::size_t vertices = 0;
        for (const std::vector<int>& polyline : figure.polylines) {
            if (polyline.size() >= 2) {
                vertices += 2 * (polyline.size() - 1);
            }
        }
        max_vertices = std::max(max_vertices, vertices);
    }
    return max_vertices;
}

void append_sky_figures(const SkyFigureCatalog& figures, const StarCatalog& stars,
                        double distance_km, DrawBatch& batch) {
    if (!figures.visible || distance_km <= 0.0) {
        return;
    }

    const double gap_rad = static_cast<double>(figures.line_gap_deg) * core::kDegToRad;
    for (const SkyFigure& figure : figures.figures) {
        if (!figure.visible) {
            continue;
        }
        LinePrimitive primitive;
        std::size_t edge_vertices = 0;
        for (const std::vector<int>& polyline : figure.polylines) {
            if (polyline.size() >= 2) {
                edge_vertices += 2 * (polyline.size() - 1);
            }
        }
        primitive.vertices.reserve(edge_vertices);
        for (const std::vector<int>& polyline : figure.polylines) {
            for (std::size_t i = 1; i < polyline.size(); ++i) {
                const Star* from = star_by_hip(stars, polyline[i - 1]);
                const Star* to = star_by_hip(stars, polyline[i]);
                if (from == nullptr || to == nullptr) {
                    continue;
                }
                append_edge(*from, *to, distance_km, gap_rad, figures.color, primitive);
            }
        }
        if (primitive.vertices.size() >= 2) {
            batch.lines.push_back(std::move(primitive));
        }
    }
}

} // namespace solar::app
