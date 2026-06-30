#include <catch2/catch_test_macros.hpp>

#include "core/json_loader.hpp"

TEST_CASE("load_bodies reads bundled planet data", "[json]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    REQUIRE(bodies.size() == 5);
    CHECK(bodies[0].name == "Sun");
    CHECK(bodies[2].name == "Earth");
    CHECK(bodies[2].elements.semi_major_axis_km > 149000000.0);
}