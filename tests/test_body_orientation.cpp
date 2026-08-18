#include "core/body_orientation.hpp"
#include "core/json_loader.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_double3.hpp>

#include <cmath>
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

[[nodiscard]] double sidereal_period_days(const solar::core::BodyRotation& rotation) {
    REQUIRE(std::abs(rotation.W_dot_deg_per_day) > 0.0);
    return 360.0 / std::abs(rotation.W_dot_deg_per_day);
}

} // namespace

TEST_CASE("load_bodies provides IAU-style pole and W for Moon", "[body_orientation][json]") {
    const solar::core::BodyDefinition body = get_body("Moon");
    CHECK(body.pole.ra_deg == Approx(269.9949));
    CHECK(body.pole.dec_deg == Approx(66.5392));
    CHECK(body.rotation.W0_deg == Approx(38.3213));
    CHECK(body.rotation.W_dot_deg_per_day == Approx(13.17635815));
    CHECK(body.primary == "Earth");
    CHECK(body.tidally_locked);
}

TEST_CASE("load_bodies provides IAU-style pole and W for Earth", "[body_orientation][json]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    CHECK(body.pole.ra_deg == Approx(0.0));
    CHECK(body.pole.dec_deg == Approx(90.0));
    CHECK(body.rotation.W0_deg == Approx(190.147));
    CHECK(body.rotation.W_dot_deg_per_day == Approx(360.9856235));
    CHECK(body.rotation.epoch.jd == Approx(solar::core::kJ2000Jd));
}

TEST_CASE("rotation_deg_at_epoch: at rotation epoch equals W0", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const solar::core::Epoch epoch{body.rotation.epoch.jd};

    const double angle = solar::core::rotation_deg_at_epoch(body, epoch);
    CHECK(angle == Approx(body.rotation.W0_deg).margin(1e-5));
}

TEST_CASE("rotation_deg_at_epoch: Earth approx. 1 rotation in a solar day", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const solar::core::Epoch epoch{body.rotation.epoch.jd + 1.0};

    const double angle = solar::core::rotation_deg_at_epoch(body, epoch);
    // W advances ~360.986°/day; after one civil day, residual ~1° past W0.
    CHECK(angle == Approx(body.rotation.W0_deg).margin(2.0));
}

TEST_CASE("rotation_deg_at_epoch: Earth 1 rotation in a sidereal day", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const double sidereal_day = sidereal_period_days(body.rotation);
    const solar::core::Epoch epoch{body.rotation.epoch.jd + sidereal_day};

    const double angle = solar::core::rotation_deg_at_epoch(body, epoch);
    CHECK(angle == Approx(body.rotation.W0_deg).margin(1e-5));
}

struct EpochOffsetRotation {
    std::string label;
    double offset{0};
    glm::dvec3 expected_point{1, 0, 0};
};

TEST_CASE("body_orientation_matrix with synthetic body", "[body_orientation]") {
    constexpr double ecl = 23.44 * solar::core::kDegToRad;
    const EpochOffsetRotation test_case =
        GENERATE(EpochOffsetRotation{"t0 + 0", 0.0, {1, 0, 0}},
                 EpochOffsetRotation{"t0 + P/4", 0.25, {0, std::cos(ecl), -std::sin(ecl)}},
                 EpochOffsetRotation{"t0 + P/2", 0.5, {-1, 0, 0}},
                 EpochOffsetRotation{"t0 + P", 1.0, {1, 0, 0}});
    INFO(test_case.label);

    solar::core::BodyDefinition body{};
    body.pole = {.ra_deg = -90.0, .dec_deg = 90.0};
    const double t0 = solar::core::kJ2000Jd;
    // One full turn per day: Ẇ = 360°/day.
    body.rotation = {.W0_deg = 0.0, .W_dot_deg_per_day = 360.0, .epoch = {t0}};

    const glm::dmat3 matrix = solar::core::body_orientation_matrix(body, {t0 + test_case.offset});

    const glm::dvec3 point{1.0, 0.0, 0.0};
    const glm::dvec3 point_after = matrix * point;

    CHECK(point_after.x == Approx(test_case.expected_point.x).margin(1e-5));
    CHECK(point_after.y == Approx(test_case.expected_point.y).margin(1e-5));
    CHECK(point_after.z == Approx(test_case.expected_point.z).margin(1e-5));
}

TEST_CASE("body_orientation_matrix result tilts Earth Z 23.44°", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};

    const glm::dmat3 matrix = solar::core::body_orientation_matrix(body, epoch);

    const glm::dvec3 point{0.0, 0.0, 1.0};
    const glm::dvec3 point_after = matrix * point;

    const double cos_tilt = glm::dot(point_after, glm::dvec3{0.0, 0.0, 1.0});
    CHECK(cos_tilt == Approx(std::cos(23.44 * solar::core::kDegToRad)).margin(1e-5));
}

TEST_CASE("Moon prime meridian stays phased with the Kepler orbit", "[body_orientation]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris{bodies};
    const solar::core::BodyDefinition moon = get_body("Moon");
    const double mu = solar::core::orbital_mu(ephemeris, moon);

    const auto facing_deg = [&](solar::core::Epoch epoch) {
        const glm::dvec3 to_earth =
            -glm::normalize(ephemeris.relative_state("Moon", epoch).position.km);
        const glm::dmat3 R = solar::core::body_orientation_matrix(moon, epoch, mu);
        const glm::dvec3 x_axis = glm::normalize(R * glm::dvec3{1.0, 0.0, 0.0});
        return std::acos(glm::clamp(glm::dot(x_axis, to_earth), -1.0, 1.0)) *
               solar::core::kRadToDeg;
    };

    const solar::core::Epoch j2000{solar::core::kJ2000Jd};
    const double period_days =
        solar::core::orbital_period_seconds(mu, moon.elements) / solar::core::kSecondsPerDay;
    const solar::core::Epoch after_period{solar::core::kJ2000Jd + period_days};
    const solar::core::Epoch now{2460910.0}; // 2026-08-17

    const double at_epoch = facing_deg(j2000);
    CHECK(at_epoch == Approx(7.2).margin(1.0));
    CHECK(facing_deg(after_period) == Approx(at_epoch).margin(0.05));
    // Optical libration only — not the tens of degrees from IAU Ẇ vs osculating n.
    CHECK(facing_deg(now) == Approx(at_epoch).margin(12.0));
}

TEST_CASE("body_orientation_matrix result rotates Earth once in a sidereal day",
          "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const double sidereal_day = sidereal_period_days(body.rotation);
    const solar::core::Epoch epoch_0{solar::core::kJ2000Jd};
    const solar::core::Epoch epoch_1{solar::core::kJ2000Jd + sidereal_day};
    const solar::core::Epoch epoch_2{solar::core::kJ2000Jd + sidereal_day / 4};

    const glm::dmat3 matrix_0 = solar::core::body_orientation_matrix(body, epoch_0);
    const glm::dmat3 matrix_1 = solar::core::body_orientation_matrix(body, epoch_1);
    const glm::dmat3 matrix_2 = solar::core::body_orientation_matrix(body, epoch_2);

    const glm::dvec3 point{1.0, 0.0, 0.0};
    const glm::dvec3 point_0 = matrix_0 * point;
    const glm::dvec3 point_1 = matrix_1 * point;
    const glm::dvec3 point_2 = matrix_2 * point;

    CHECK(glm::dot(point_0, point_1) == Approx(1.0).margin(1e-5));
    CHECK(glm::dot(point_0, point_2) == Approx(0.0).margin(1e-5));
}
