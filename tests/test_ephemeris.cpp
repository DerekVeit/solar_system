#include "core/ephemeris.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("find_body returns catalog entries by name", "[ephemeris]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris{bodies};

    const solar::core::BodyDefinition* earth = solar::core::find_body(ephemeris, "Earth");
    REQUIRE(earth != nullptr);
    CHECK(earth->name == "Earth");
    const solar::core::BodyDefinition* pluto = solar::core::find_body(ephemeris, "Pluto");
    REQUIRE(pluto != nullptr);
    CHECK(pluto->name == "Pluto");
    CHECK(solar::core::find_body(ephemeris, "Eris") == nullptr);
}

TEST_CASE("central_gravitational_parameter returns Sun mu", "[ephemeris]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris{bodies};

    const solar::core::BodyDefinition* sun = solar::core::find_body(ephemeris, "Sun");
    REQUIRE(sun != nullptr);
    CHECK(solar::core::central_gravitational_parameter(ephemeris) ==
          sun->gravitational_parameter_km3_s2);
}
