#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
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

TEST_CASE("bundled catalog satellites are ordered inner to outer", "[ephemeris][json]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris{bodies};

    const auto* sun = solar::core::find_body(ephemeris, "Sun");
    const auto* jupiter = solar::core::find_body(ephemeris, "Jupiter");
    const auto* mercury = solar::core::find_body(ephemeris, "Mercury");
    REQUIRE(sun != nullptr);
    REQUIRE(jupiter != nullptr);
    REQUIRE(mercury != nullptr);

    CHECK(solar::core::innermost_satellite(ephemeris, *sun)->name == "Mercury");
    CHECK(solar::core::innermost_satellite(ephemeris, *jupiter)->name == "Io");
    CHECK(solar::core::innermost_satellite(ephemeris, *mercury) == nullptr);

    const auto jovian = solar::core::bodies_orbiting(ephemeris, "Jupiter");
    REQUIRE(jovian.size() == 4);
    CHECK(jovian[0]->name == "Io");
    CHECK(jovian[3]->name == "Callisto");
    CHECK(solar::core::sibling_by_offset(ephemeris, *jovian[0], 1)->name == "Europa");
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

TEST_CASE("bodies_orbiting lists satellites by increasing semi-major axis", "[ephemeris]") {
    const solar::core::KeplerEphemeris ephemeris{{
        make_body("Sun", "", 1.0e11, 0.0),
        make_body("Earth", "", 4.0e5, 1.0e8),
        make_body("Mars", "", 4.0e4, 2.0e8),
        make_body("Deimos", "Mars", 1.0, 2.4e4),
        make_body("Phobos", "Mars", 1.0, 9.4e3),
    }};

    const auto heliocentric = solar::core::bodies_orbiting(ephemeris, "Sun");
    REQUIRE(heliocentric.size() == 3);
    CHECK(heliocentric[0]->name == "Sun");
    CHECK(heliocentric[1]->name == "Earth");
    CHECK(heliocentric[2]->name == "Mars");

    const auto martian = solar::core::bodies_orbiting(ephemeris, "Mars");
    REQUIRE(martian.size() == 2);
    CHECK(martian[0]->name == "Phobos");
    CHECK(martian[1]->name == "Deimos");
    CHECK(solar::core::bodies_orbiting(ephemeris, "").size() == 3);
}

TEST_CASE("innermost_satellite and sibling_by_offset walk a family", "[ephemeris]") {
    const solar::core::KeplerEphemeris ephemeris{{
        make_body("Sun", "", 1.0e11, 0.0),
        make_body("Earth", "", 4.0e5, 1.0e8),
        make_body("Mars", "", 4.0e4, 2.0e8),
        make_body("Deimos", "Mars", 1.0, 2.4e4),
        make_body("Phobos", "Mars", 1.0, 9.4e3),
    }};
    const auto* sun = solar::core::find_body(ephemeris, "Sun");
    const auto* earth = solar::core::find_body(ephemeris, "Earth");
    const auto* mars = solar::core::find_body(ephemeris, "Mars");
    const auto* phobos = solar::core::find_body(ephemeris, "Phobos");
    REQUIRE(sun != nullptr);
    REQUIRE(earth != nullptr);
    REQUIRE(mars != nullptr);
    REQUIRE(phobos != nullptr);

    CHECK(solar::core::innermost_satellite(ephemeris, *sun)->name == "Earth");
    CHECK(solar::core::innermost_satellite(ephemeris, *mars)->name == "Phobos");
    CHECK(solar::core::innermost_satellite(ephemeris, *earth) == nullptr);
    CHECK(solar::core::innermost_satellite(ephemeris, *phobos) == nullptr);

    CHECK(solar::core::sibling_by_offset(ephemeris, *earth, 1)->name == "Mars");
    CHECK(solar::core::sibling_by_offset(ephemeris, *mars, 1)->name == "Sun");
    CHECK(solar::core::sibling_by_offset(ephemeris, *phobos, 1)->name == "Deimos");
    CHECK(solar::core::sibling_by_offset(ephemeris, *phobos, -1)->name == "Deimos");
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
