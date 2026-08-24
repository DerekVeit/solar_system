#include "app/render/types.hpp"
#include "app/scene/star_catalog_loader.hpp"
#include "app/scene/star_markers.hpp"
#include "app/scene/stroke_font.hpp"
#include "core/sky_orientation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/ext/vector_double3.hpp>
#include <glm/geometric.hpp>

#include <cmath>
#include <set>
#include <string>
#include <vector>

using Catch::Approx;

TEST_CASE("stars.json includes Dubhe at J2000 RA/Dec", "[stars][json]") {
    const auto catalog = solar::app::load_star_catalog("assets/data/stars.json");
    REQUIRE(catalog.visible);
    CHECK(catalog.marker_radius_deg == Approx(0.35f));

    const solar::app::Star* dubhe = nullptr;
    for (const solar::app::Star& star : catalog.stars) {
        if (star.name == "Dubhe") {
            dubhe = &star;
        }
    }
    REQUIRE(dubhe != nullptr);
    CHECK(dubhe->ra_deg == Approx(165.9320).margin(1e-4));
    CHECK(dubhe->dec_deg == Approx(61.7510).margin(1e-4));

    std::set<std::string> names;
    for (const solar::app::Star& star : catalog.stars) {
        names.insert(star.name);
    }
    for (const char* name : {"Dubhe", "Merak", "Phecda", "Megrez", "Alioth", "Mizar", "Alkaid"}) {
        CHECK(names.contains(name));
    }
}

TEST_CASE("ecliptic_direction is a unit vector in the draw frame", "[stars]") {
    const glm::dvec3 dir = solar::core::ecliptic_direction(165.9320, 61.7510);
    CHECK(glm::length(dir) == Approx(1.0).margin(1e-12));
    CHECK(dir.z > 0.5);
}

TEST_CASE("directional_circle is an angular ring around the direction", "[stars]") {
    const glm::dvec3 dir{0.0, 0.0, 1.0};
    constexpr double kDistance = 1000.0;
    constexpr double kRadius = 0.1;
    constexpr std::size_t kSamples = 12;
    const auto ring = solar::core::directional_circle(dir, kDistance, kRadius, kSamples);
    REQUIRE(ring.size() == kSamples);

    const double expected_dot = std::cos(kRadius);
    for (const glm::dvec3& point : ring) {
        CHECK(glm::length(point) == Approx(kDistance).margin(1e-9));
        CHECK(glm::dot(glm::normalize(point), dir) == Approx(expected_dot).margin(1e-9));
    }

    CHECK(solar::core::directional_circle(dir, kDistance, kRadius, 2).empty());
    CHECK(solar::core::directional_circle({0.0, 0.0, 0.0}, kDistance, kRadius, kSamples).empty());
}

TEST_CASE("stroke font emits line pairs for letters and skips spaces", "[stars][font]") {
    CHECK(solar::app::stroke_text_line_vertex_count("") == 0);
    CHECK(solar::app::stroke_text_line_vertex_count(" ") == 0);
    CHECK(solar::app::stroke_text_line_vertex_count("A") >= 8);
    CHECK(solar::app::stroke_text_line_vertex_count("Dubhe") >
          solar::app::stroke_text_line_vertex_count("A"));
    CHECK(solar::app::stroke_text_line_vertex_count("Galactic centre") >
          solar::app::stroke_text_line_vertex_count("Dubhe"));

    std::vector<glm::dvec3> endpoints;
    solar::app::append_stroke_text("A", {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
                                   endpoints);
    REQUIRE(endpoints.size() == solar::app::stroke_text_line_vertex_count("A"));
    REQUIRE(endpoints.size() >= 2);
    CHECK(endpoints.size() % 2 == 0);
    for (const glm::dvec3& point : endpoints) {
        CHECK(point.x >= -0.1);
        CHECK(point.x <= solar::app::kStrokeFontAdvance);
        CHECK(point.y >= -2.1);
        CHECK(point.y <= solar::app::kStrokeFontCapHeight + 0.1);
        CHECK(point.z == Approx(0.0).margin(1e-12));
    }
}

TEST_CASE("append_star_markers draws a reticle and a name label", "[stars]") {
    solar::app::StarCatalog catalog{};
    catalog.stars.push_back(
        solar::app::Star{.name = "Dubhe", .ra_deg = 165.9320, .dec_deg = 61.7510});

    solar::app::DrawBatch hidden;
    catalog.visible = false;
    solar::app::append_star_markers(catalog, 1000.0, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, hidden);
    CHECK(hidden.line_loops.empty());
    CHECK(hidden.lines.empty());

    catalog.visible = true;
    solar::app::DrawBatch batch;
    solar::app::append_star_markers(catalog, 1000.0, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, batch);
    REQUIRE(batch.line_loops.size() == 1);
    REQUIRE(batch.lines.size() == 1);
    CHECK(batch.line_loops[0].vertices.size() == solar::app::StarCatalog::kMarkerSamples);
    CHECK(batch.lines[0].vertices.size() == solar::app::stroke_text_line_vertex_count("Dubhe"));
    CHECK(solar::app::star_marker_line_vertex_capacity(catalog) == batch.lines[0].vertices.size());

    const glm::dvec3 dir = solar::core::ecliptic_direction(165.9320, 61.7510);
    for (const auto& vertex : batch.lines[0].vertices) {
        const glm::dvec3 point{vertex.x_km, vertex.y_km, vertex.z_km};
        CHECK(glm::length(point) == Approx(1000.0).margin(1e-3));
        CHECK(glm::dot(glm::normalize(point), dir) > 0.99);
        CHECK(vertex.color == catalog.color);
    }
}
