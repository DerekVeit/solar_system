#include "app/scene/body_visual_geometry.hpp"
#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/geometric.hpp>

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

void check_equal(const solar::core::Displacement& actual, const solar::core::Displacement& expected,
                 double margin) {
    CHECK(actual.km.x == Approx(expected.km.x).margin(margin));
    CHECK(actual.km.y == Approx(expected.km.y).margin(margin));
    CHECK(actual.km.z == Approx(expected.km.z).margin(margin));
}

solar::core::Epoch shifted(const solar::core::Epoch& epoch, double seconds) {
    return solar::core::Epoch{epoch.jd + seconds / solar::core::kSecondsPerDay};
}

} // namespace

TEST_CASE("orbit_display_factor is 1 without scaling or a primary", "[body_visual][geometry]") {
    CHECK(solar::app::orbit_display_factor(true, true, 15.0) == Approx(15.0));
    CHECK(solar::app::orbit_display_factor(true, false, 15.0) == Approx(1.0));
    CHECK(solar::app::orbit_display_factor(false, true, 15.0) == Approx(1.0));
    CHECK(solar::app::orbit_display_factor(false, false, 15.0) == Approx(1.0));
}

TEST_CASE("offset_from_primary scales only the relative vector", "[body_visual][geometry]") {
    const solar::core::Displacement primary{{10.0, 20.0, 30.0}};
    const solar::core::Displacement relative{{1.0, 2.0, 3.0}};

    check_equal(solar::app::offset_from_primary(primary, relative, 1.0),
                solar::core::Displacement{{11.0, 22.0, 33.0}}, 1e-12);
    check_equal(solar::app::offset_from_primary(primary, relative, 15.0),
                solar::core::Displacement{{25.0, 50.0, 75.0}}, 1e-12);
}

TEST_CASE("drawn_position scales satellite orbits without changing state()",
          "[body_visual][geometry]") {
    const solar::core::KeplerEphemeris ephemeris{synthetic_earth_moon()};
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};

    const solar::core::StateVector earth = ephemeris.state("Earth", epoch);
    const solar::core::StateVector moon = ephemeris.state("Moon", epoch);
    const solar::core::StateVector relative = ephemeris.relative_state("Moon", epoch);

    check_equal(solar::app::drawn_position(ephemeris, "Moon", epoch, 1.0), moon.position, 1e-9);
    check_equal(solar::app::drawn_position(ephemeris, "Moon", epoch, 15.0),
                solar::app::offset_from_primary(earth.position, relative.position, 15.0), 1e-9);
    check_equal(solar::app::drawn_position(ephemeris, "Earth", epoch, 1.0), earth.position, 1e-9);
    CHECK(glm::length(moon.position.km - earth.position.km) == Approx(4.0e5).margin(1.0));
    CHECK_THROWS_AS(solar::app::drawn_position(ephemeris, "Phobos", epoch, 1.0),
                    std::invalid_argument);
}

TEST_CASE("orbit_loop_positions freeze the primary and close around it",
          "[body_visual][geometry]") {
    const solar::core::KeplerEphemeris ephemeris{synthetic_earth_moon()};
    const solar::core::BodyDefinition* moon = solar::core::find_body(ephemeris, "Moon");
    REQUIRE(moon != nullptr);

    const solar::core::Epoch epoch_now{solar::core::kJ2000Jd + 80.0};
    constexpr double kFactor = 15.0;
    constexpr std::size_t kSamples = 8;
    const auto loop =
        solar::app::orbit_loop_positions(ephemeris, *moon, epoch_now, kFactor, kSamples);
    REQUIRE(loop.size() == kSamples);

    const solar::core::Displacement earth_now = ephemeris.state("Earth", epoch_now).position;
    const solar::core::Displacement earth_then =
        ephemeris.state("Earth", solar::core::Epoch{solar::core::kJ2000Jd}).position;
    CHECK(glm::length(earth_now.km - earth_then.km) > 1.0e6);

    for (const solar::core::Displacement& position : loop) {
        CHECK(glm::length(position.km - earth_now.km) == Approx(kFactor * 4.0e5).margin(1.0));
    }
    check_equal(loop.front(), solar::app::drawn_position(ephemeris, "Moon", epoch_now, kFactor),
                1e-6);

    const double mu = solar::core::orbital_mu(ephemeris, *moon);
    const double period_seconds = solar::core::orbital_period_seconds(mu, moon->elements);
    const solar::core::Epoch half_period_ago = shifted(epoch_now, -0.5 * period_seconds);
    const solar::core::Displacement frozen_halfway = solar::app::offset_from_primary(
        earth_now, ephemeris.relative_state("Moon", half_period_ago).position, kFactor);
    const std::size_t halfway = kSamples / 2;
    check_equal(loop[halfway], frozen_halfway, 1e-6);
    CHECK(glm::length(loop[halfway].km -
                      solar::app::drawn_position(ephemeris, "Moon", half_period_ago, kFactor).km) >
          1.0e5);

    const solar::core::BodyDefinition* sun = solar::core::find_body(ephemeris, "Sun");
    REQUIRE(sun != nullptr);
    CHECK(solar::app::orbit_loop_positions(ephemeris, *sun, epoch_now, 1.0, kSamples).empty());
}

TEST_CASE("tail_positions sample the composed path uniformly in time", "[body_visual][geometry]") {
    const solar::core::KeplerEphemeris ephemeris{synthetic_earth_moon()};
    const solar::core::BodyDefinition* moon = solar::core::find_body(ephemeris, "Moon");
    REQUIRE(moon != nullptr);

    const solar::core::Epoch epoch_now{solar::core::kJ2000Jd + 80.0};
    const double mu = solar::core::orbital_mu(ephemeris, *moon);
    const double period_seconds = solar::core::orbital_period_seconds(mu, moon->elements);
    constexpr double kFactor = 15.0;
    constexpr std::size_t kSamples = 5;
    const double duration = 2.0 * period_seconds;

    const auto tail =
        solar::app::tail_positions(ephemeris, *moon, epoch_now, kFactor, duration, kSamples);
    REQUIRE(tail.size() == kSamples);

    for (std::size_t sample = 0; sample < kSamples; ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(kSamples - 1);
        const solar::core::Epoch sample_epoch = shifted(epoch_now, -duration * fraction);
        check_equal(tail[sample],
                    solar::app::drawn_position(ephemeris, "Moon", sample_epoch, kFactor), 1e-6);
        CHECK(glm::length(tail[sample].km - ephemeris.state("Earth", sample_epoch).position.km) ==
              Approx(kFactor * 4.0e5).margin(1.0));
    }

    const solar::core::Displacement one_period_ago =
        solar::app::drawn_position(ephemeris, "Moon", shifted(epoch_now, -period_seconds), kFactor);
    CHECK(glm::length(tail.back().km - one_period_ago.km) > 1.0e5);

    CHECK(solar::app::tail_positions(ephemeris, *moon, epoch_now, kFactor, 0.0, kSamples).empty());
    CHECK(solar::app::tail_positions(ephemeris, *moon, epoch_now, kFactor, duration, 1).empty());
}
