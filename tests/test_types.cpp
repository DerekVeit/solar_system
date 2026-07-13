#include "core/constants.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cmath>
#include <string>
#include <tuple>

using Catch::Approx;
using Catch::Matchers::ContainsSubstring;

namespace {

struct PolarCase {
    std::string label;
    double x;
    double y;
    double expected_length;
    double expected_angle;
};

constexpr double kEps = 1e-12;

} // namespace

TEST_CASE("Epoch.to_string returns ISO-like timestamp string for J2000", "[types]") {
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};
    const std::string ts = epoch.to_string();
    CHECK_THAT(ts, ContainsSubstring("(approx)"));
    CHECK_THAT(ts, ContainsSubstring("2000-01-01"));
}

TEST_CASE("Epoch.to_string for a later date", "[types]") {
    const solar::core::Epoch epoch{solar::core::kJ2000Jd + 35};
    const std::string ts = epoch.to_string();
    CHECK_THAT(ts, ContainsSubstring("2000-02-05"));
}

TEST_CASE("Epoch.to_string for a fractional day later", "[types]") {
    const solar::core::Epoch epoch{solar::core::kJ2000Jd + 0.5};
    const std::string ts = epoch.to_string();
    CHECK_THAT(ts, ContainsSubstring("2000-01-02 00:00:00"));
}

TEST_CASE("Displacement.polar_xy converts quadrants to 0..2pi angles", "[types]") {
    using Row = std::tuple<std::string, double, double, double, double>;

    const auto [label, x, y, expected_length, expected_angle] =
        GENERATE(table<std::string, double, double, double, double>({
            Row{"Q1: +x +y", 3.0, 4.0, 5.0, std::atan2(4.0, 3.0)},
            Row{"Q2: -x +y", -3.0, 4.0, 5.0, std::atan2(4.0, -3.0)},
            Row{"Q3: -x -y", -3.0, -4.0, 5.0, std::atan2(-4.0, -3.0) + solar::core::kTwoPi},
            Row{"Q4: +x -y", 3.0, -4.0, 5.0, std::atan2(-4.0, 3.0) + solar::core::kTwoPi},
        }));

    INFO(label);

    const solar::core::Displacement displacement{{x, y, 0.0}};
    const solar::core::Polar polar = displacement.polar_xy();

    CHECK(polar.length == Approx(expected_length));
    CHECK(polar.angle == Approx(expected_angle).margin(kEps));
    CHECK(polar.angle >= 0.0);
    CHECK(polar.angle < solar::core::kTwoPi);
}

TEST_CASE("Displacement.polar_xy handles atan2 edge cases on axes", "[types]") {
    using Row = std::tuple<std::string, double, double, double, double>;

    const auto [label, x, y, expected_length, expected_angle] =
        GENERATE(table<std::string, double, double, double, double>({
            Row{"+x axis", 2.0, 0.0, 2.0, 0.0},
            Row{"-x axis (atan2 = pi)", -2.0, 0.0, 2.0, solar::core::kPi},
            Row{"+y axis (atan2 = pi/2)", 0.0, 3.0, 3.0, solar::core::kPi / 2.0},
            Row{"-y axis (atan2 = -pi/2 -> 3pi/2)", 0.0, -3.0, 3.0, 3.0 * solar::core::kPi / 2.0},
        }));

    INFO(label);

    const solar::core::Displacement displacement{{x, y, 0.0}};
    const solar::core::Polar polar = displacement.polar_xy();

    CHECK(polar.length == Approx(expected_length));
    CHECK(polar.angle == Approx(expected_angle).margin(kEps));
}

TEST_CASE("Displacement.polar_xy normalizes negative atan2 results", "[types]") {
    // std::atan2 returns values in (-pi, pi]; Q3/Q4 on the axes land below zero.
    const solar::core::Displacement displacement{{1.0, -1.0, 0.0}};
    const solar::core::Polar polar = displacement.polar_xy();

    const double raw_atan2 = std::atan2(-1.0, 1.0);
    CHECK(raw_atan2 == Approx(-solar::core::kPi / 4.0).margin(kEps));
    CHECK(raw_atan2 < 0.0);

    CHECK(polar.angle == Approx(7.0 * solar::core::kPi / 4.0).margin(kEps));
    CHECK(polar.angle >= 0.0);
}

TEST_CASE("Displacement.polar_xy at origin has zero length", "[types]") {
    const solar::core::Displacement displacement{{0.0, 0.0, 0.0}};
    const solar::core::Polar polar = displacement.polar_xy();

    CHECK(polar.length == Approx(0.0).margin(kEps));
    CHECK(std::isfinite(polar.angle));
}
