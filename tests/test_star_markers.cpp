#include "app/scene/star_catalog_loader.hpp"
#include "core/sky_orientation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/geometric.hpp>

#include <cmath>

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
