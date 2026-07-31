#include "core/body_orientation.hpp"
#include "core/json_loader.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using Catch::Approx;

solar::core::BodyDefinition get_body(std::string body_name) {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris{bodies};
    const solar::core::BodyDefinition* body = solar::core::find_body(ephemeris, body_name);
    REQUIRE(body != nullptr);
    return *body;
}

} // namespace

TEST_CASE("rotation_deg_at_epoch: zero rotation ", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};

    double angle = solar::core::rotation_deg_at_epoch(body, epoch);

    double expected_angle = body.rotation.prime_meridian_deg_at_epoch;
    CHECK(angle == Approx(expected_angle).margin(0.01));
}

TEST_CASE("rotation_deg_at_epoch: Earth approx. 1 rotation in a solar day", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const double solar_day = 1.0;
    const solar::core::Epoch epoch{solar::core::kJ2000Jd + solar_day};

    double angle = solar::core::rotation_deg_at_epoch(body, epoch);

    double expected_angle = body.rotation.prime_meridian_deg_at_epoch;
    CHECK(angle == Approx(expected_angle).margin(2.0));
}

TEST_CASE("rotation_deg_at_epoch: Earth 1 rotation in a sidereal day", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const double sidereal_day = body.rotation.period_s / solar::core::kSecondsPerDay;
    const solar::core::Epoch epoch{solar::core::kJ2000Jd + sidereal_day};

    double angle = solar::core::rotation_deg_at_epoch(body, epoch);

    double expected_angle = body.rotation.prime_meridian_deg_at_epoch;
    CHECK(angle == Approx(expected_angle).margin(0.001));
}
