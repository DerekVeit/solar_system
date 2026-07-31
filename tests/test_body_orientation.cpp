#include "core/body_orientation.hpp"
#include "core/json_loader.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_double3.hpp>

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

TEST_CASE("body_orientation_matrix result tilts Earth Z 23.44°", "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};

    const glm::dmat3 matrix = solar::core::body_orientation_matrix(body, epoch);

    const glm::dvec3 point{0.0, 0.0, 1.0};
    const glm::dvec3 point_after = matrix * point;

    // crudely assuming the tilt is purely about the X axis
    CHECK(glm::degrees(glm::asin(point_after.y)) == Approx(23.44).margin(0.0001));
}

TEST_CASE("body_orientation_matrix result rotates Earth once in a sidereal day",
          "[body_orientation]") {
    const solar::core::BodyDefinition body = get_body("Earth");
    const double sidereal_day = body.rotation.period_s / solar::core::kSecondsPerDay;
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

    CHECK(glm::dot(point_0, point_1) == Approx(1.0).margin(0.001));
    CHECK(glm::dot(point_0, point_2) == Approx(0.0).margin(0.001));
}
