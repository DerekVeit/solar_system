#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <vector>

using Catch::Approx;

namespace {

solar::core::BodyDefinition make_body(std::string name, std::string primary, double mu,
                                      double a_km) {
    solar::core::BodyDefinition body{};
    body.name = std::move(name);
    body.primary = std::move(primary);
    body.gravitational_parameter_km3_s2 = mu;
    body.elements.semi_major_axis_km = a_km;
    body.elements.epoch.jd = solar::core::kJ2000Jd;
    return body;
}

std::vector<solar::core::BodyDefinition> synthetic_earth_moon() {
    return {
        make_body("Sun", "", 1.0e11, 0.0),
        make_body("Earth", "", 4.0e5, 1.0e8),
        make_body("Moon", "Earth", 4.9e3, 4.0e5),
    };
}

} // namespace

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

TEST_CASE("orbital_mu uses the primary, or the Sun when primary is empty", "[ephemeris]") {
    const solar::core::KeplerEphemeris ephemeris{synthetic_earth_moon()};
    const solar::core::BodyDefinition* earth = solar::core::find_body(ephemeris, "Earth");
    const solar::core::BodyDefinition* moon = solar::core::find_body(ephemeris, "Moon");
    REQUIRE(earth != nullptr);
    REQUIRE(moon != nullptr);

    CHECK(solar::core::orbital_mu(ephemeris, *earth) == Approx(1.0e11));
    CHECK(solar::core::orbital_mu(ephemeris, *moon) == Approx(4.0e5));
    CHECK(solar::core::primary_body(ephemeris, *moon)->name == "Earth");
}

TEST_CASE("state composes satellite Kepler onto the primary", "[ephemeris]") {
    const solar::core::KeplerEphemeris ephemeris{synthetic_earth_moon()};
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};

    const solar::core::StateVector earth = ephemeris.state("Earth", epoch);
    const solar::core::StateVector moon = ephemeris.state("Moon", epoch);
    const solar::core::StateVector relative = ephemeris.relative_state("Moon", epoch);

    CHECK(relative.position.length() == Approx(4.0e5).margin(1.0));
    CHECK(glm::length(moon.position.km - earth.position.km) == Approx(4.0e5).margin(1.0));
    CHECK(moon.position.km.x == Approx(earth.position.km.x + relative.position.km.x).margin(1e-6));
    CHECK(moon.position.km.y == Approx(earth.position.km.y + relative.position.km.y).margin(1e-6));
    CHECK(moon.position.km.z == Approx(earth.position.km.z + relative.position.km.z).margin(1e-6));
}

TEST_CASE("Moon relative state matches Horizons geocentric J2000", "[ephemeris][horizons]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris{bodies};
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};

    const solar::core::StateVector relative = ephemeris.relative_state("Moon", epoch);
    // NASA/JPL Horizons DE441, Earth-centered, ecliptic J2000, 2451545.0 TDB.
    CHECK(relative.position.km.x == Approx(-291608.3841877129).margin(1.0));
    CHECK(relative.position.km.y == Approx(-274979.7416731504).margin(1.0));
    CHECK(relative.position.km.z == Approx(36271.19662699287).margin(1.0));

    const solar::core::StateVector moon = ephemeris.state("Moon", epoch);
    const solar::core::StateVector earth = ephemeris.state("Earth", epoch);
    CHECK(glm::length(moon.position.km - earth.position.km) ==
          Approx(relative.position.length()).margin(1e-6));
}

TEST_CASE("KeplerEphemeris rejects an unknown or cyclic primary", "[ephemeris]") {
    CHECK_THROWS_AS(solar::core::KeplerEphemeris{{make_body("Moon", "Earth", 1.0, 4.0e5)}},
                    std::invalid_argument);

    auto self = make_body("Moon", "Moon", 1.0, 4.0e5);
    CHECK_THROWS_AS(solar::core::KeplerEphemeris{{self}}, std::invalid_argument);

    std::vector<solar::core::BodyDefinition> cycle{
        make_body("A", "B", 1.0, 1.0e5),
        make_body("B", "A", 1.0, 1.0e5),
    };
    CHECK_THROWS_AS(solar::core::KeplerEphemeris{cycle}, std::invalid_argument);
}
