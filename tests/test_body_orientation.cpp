#include "core/body_orientation.hpp"
#include "core/json_loader.hpp"

#include "core/constants.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
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
    CHECK(angle == Approx(expected_angle).margin(1e-5));
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
    CHECK(angle == Approx(expected_angle).margin(1e-5));
}

struct EpochOffsetRotation {
    std::string label;
    double offset{0};
    glm::dvec3 expected_point{1, 0, 0};
};

TEST_CASE("body_orientation_matrix with synthetic body", "[body_orientation]") {
    const EpochOffsetRotation test_case = GENERATE(EpochOffsetRotation{"t0 + 0", 0.0, {1, 0, 0}},
                                                   EpochOffsetRotation{"t0 + P/4", 0.25, {0, 1, 0}},
                                                   EpochOffsetRotation{"t0 + P/2", 0.5, {-1, 0, 0}},
                                                   EpochOffsetRotation{"t0 + P", 1.0, {1, 0, 0}});
    INFO(test_case.label);

    solar::core::BodyDefinition body{};
    body.obliquity_deg = 0.0;
    const double t0 = solar::core::kJ2000Jd;
    body.rotation = {
        .period_s = solar::core::kSecondsPerDay, .prime_meridian_deg_at_epoch = 0.0, .epoch = {t0}};

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

    CHECK(glm::dot(point_0, point_1) == Approx(1.0).margin(1e-5));
    CHECK(glm::dot(point_0, point_2) == Approx(0.0).margin(1e-5));
}
