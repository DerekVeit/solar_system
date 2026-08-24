#include "app/render/types.hpp"
#include "app/scene/sky_figure_lines.hpp"
#include "app/scene/sky_figure_loader.hpp"
#include "app/scene/star_catalog_loader.hpp"
#include "core/constants.hpp"
#include "core/sky_orientation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using Catch::Approx;

namespace {

std::filesystem::path write_temp_json(const std::string& name, const std::string& body) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream output{path};
    output << body;
    return path;
}

} // namespace

TEST_CASE("constellations.json lists showpiece constellations and asterisms",
          "[sky][figures][json]") {
    const auto stars = solar::app::load_star_catalog("assets/data/stars.json");
    const auto catalog = solar::app::load_sky_figures("assets/data/constellations.json", stars);

    REQUIRE(catalog.visible);
    CHECK(catalog.line_gap_deg == Approx(0.5f));

    int constellations = 0;
    int asterisms = 0;
    const solar::app::SkyFigure* dipper = nullptr;
    const solar::app::SkyFigure* orion = nullptr;
    for (const solar::app::SkyFigure& figure : catalog.figures) {
        if (figure.kind == solar::app::SkyFigureKind::constellation) {
            ++constellations;
        } else if (figure.kind == solar::app::SkyFigureKind::asterism) {
            ++asterisms;
        }
        if (figure.id == "dipper") {
            dipper = &figure;
        }
        if (figure.id == "ori") {
            orion = &figure;
        }
    }
    CHECK(constellations == 40);
    CHECK(asterisms == 7);

    REQUIRE(dipper != nullptr);
    CHECK(dipper->name == "Big Dipper");
    CHECK(dipper->kind == solar::app::SkyFigureKind::asterism);
    REQUIRE(dipper->polylines.size() == 2);
    CHECK(dipper->polylines[0] ==
          std::vector<int>({54061, 53910, 58001, 59774, 62956, 65378, 67301}));
    CHECK(dipper->polylines[1] == std::vector<int>({54061, 59774}));
    REQUIRE(orion != nullptr);
    CHECK(orion->kind == solar::app::SkyFigureKind::constellation);
    CHECK_FALSE(orion->polylines.empty());
}

TEST_CASE("sky figure loader rejects an unknown HIP", "[sky][figures]") {
    const auto stars = solar::app::load_star_catalog("assets/data/stars.json");
    const auto path = write_temp_json(
        "solar_unknown_hip_figures.json",
        R"({"figures":[{"id":"x","name":"X","kind":"asterism","polylines":[[54061,1]]}]})");
    CHECK_THROWS_AS(solar::app::load_sky_figures(path, stars), std::runtime_error);
}

TEST_CASE("append_sky_figures draws inset segments without reticles", "[sky][figures]") {
    const auto stars = solar::app::load_star_catalog("assets/data/stars.json");
    const auto bundled = solar::app::load_sky_figures("assets/data/constellations.json", stars);
    solar::app::SkyFigureCatalog figures = bundled;
    figures.figures.clear();
    for (const solar::app::SkyFigure& figure : bundled.figures) {
        if (figure.id == "dipper") {
            figures.figures.push_back(figure);
        }
    }
    REQUIRE(figures.figures.size() == 1);
    constexpr double kDistance = 1000.0;

    solar::app::DrawBatch hidden;
    figures.visible = false;
    solar::app::append_sky_figures(figures, stars, kDistance, hidden);
    CHECK(hidden.lines.empty());
    CHECK(hidden.line_loops.empty());

    figures.visible = true;
    solar::app::DrawBatch batch;
    solar::app::append_sky_figures(figures, stars, kDistance, batch);
    REQUIRE(batch.lines.size() == 1);
    CHECK(batch.line_loops.empty());
    CHECK(batch.lines[0].vertices.size() == 14);
    CHECK(solar::app::sky_figure_line_vertex_capacity(figures) == 14);

    const double gap_rad = figures.line_gap_deg * solar::core::kDegToRad;
    for (const auto& vertex : batch.lines[0].vertices) {
        const glm::dvec3 point{vertex.x_km, vertex.y_km, vertex.z_km};
        CHECK(glm::length(point) == Approx(kDistance).margin(1e-3));
        CHECK(vertex.color == figures.color);

        bool near_a_star = false;
        for (const solar::app::Star& star : stars.stars) {
            if (star.hip == 0) {
                continue;
            }
            const glm::dvec3 dir = solar::core::ecliptic_direction(star.ra_deg, star.dec_deg);
            const double angle =
                std::acos(std::clamp(glm::dot(glm::normalize(point), dir), -1.0, 1.0));
            if (std::abs(angle - gap_rad) < 1e-4) {
                near_a_star = true;
                break;
            }
        }
        CHECK(near_a_star);
    }

    solar::app::DrawBatch all;
    solar::app::append_sky_figures(bundled, stars, kDistance, all);
    CHECK(all.line_loops.empty());
    CHECK(all.lines.size() == bundled.figures.size());
}
