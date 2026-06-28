#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "core/constants.hpp"
#include "core/kepler.hpp"

using Catch::Approx;

namespace {
constexpr double kSunMu = 132712440041.93938;
}

TEST_CASE("solve_kepler recovers circular orbit eccentric anomaly", "[kepler]") {
    const double mean_anomaly = solar::core::kPi / 3.0;
    const double eccentric_anomaly = solar::core::solve_kepler(mean_anomaly, 0.0);
    CHECK(eccentric_anomaly == Approx(mean_anomaly).margin(1e-10));
}

TEST_CASE("solve_kepler at M = π, e = 0.5, E = M", "[kepler]") {
    const double mean_anomaly = solar::core::kPi;
    const double eccentric_anomaly = solar::core::solve_kepler(mean_anomaly, 0.5);
    CHECK(eccentric_anomaly == Approx(mean_anomaly).margin(1e-10));
}

TEST_CASE("solve_kepler at M = 0, e = π/3, E = π/6", "[kepler]") {
    const double mean_anomaly = 0;
    const double eccentricity = solar::core::kPi / 3;
    const double eccentric_anomaly = solar::core::solve_kepler(mean_anomaly, eccentricity);
    CHECK(eccentric_anomaly == Approx(solar::core::kPi / 6).margin(1e-10));
}

TEST_CASE("solve_kepler at M = π/6, e = π/3, E = π/2", "[kepler]") {
    const double mean_anomaly = solar::core::kPi / 6;
    const double eccentricity = solar::core::kPi / 3;
    const double eccentric_anomaly = solar::core::solve_kepler(mean_anomaly, eccentricity);
    CHECK(eccentric_anomaly == Approx(solar::core::kPi / 2).margin(1e-10));
}

TEST_CASE("state_from_kepler preserves semi-major axis for circular orbit", "[kepler]") {
    const solar::core::KeplerianElements elements{
        .semi_major_axis_km = solar::core::kAuKm,
        .eccentricity = 0.0,
        .inclination_rad = 0.0,
        .longitude_ascending_node_rad = 0.0,
        .argument_periapsis_rad = 0.0,
        .mean_anomaly_at_epoch_rad = 0.0,
        .epoch = solar::core::Epoch{solar::core::kJ2000Jd},
    };

    const auto state = solar::core::state_from_kepler(kSunMu, elements,
                                                      solar::core::Epoch{solar::core::kJ2000Jd});

    CHECK(state.position.length() == Approx(solar::core::kAuKm).margin(1.0));
    CHECK(state.velocity.km_per_s.x == Approx(0.0).margin(1e-6));
    CHECK(state.velocity.km_per_s.y == Approx(29.78).margin(0.05));
}

TEST_CASE("state_from_kepler completes one revolution per orbital period", "[kepler]") {
    const solar::core::KeplerianElements elements{
        .semi_major_axis_km = solar::core::kAuKm,
        .eccentricity = 0.0,
        .inclination_rad = 0.0,
        .longitude_ascending_node_rad = 0.0,
        .argument_periapsis_rad = 0.0,
        .mean_anomaly_at_epoch_rad = 0.0,
        .epoch = solar::core::Epoch{solar::core::kJ2000Jd},
    };

    const auto start = solar::core::state_from_kepler(kSunMu, elements,
                                                     solar::core::Epoch{solar::core::kJ2000Jd});
    const double mean_motion =
        std::sqrt(kSunMu / (solar::core::kAuKm * solar::core::kAuKm * solar::core::kAuKm));
    const double orbital_period_days =
        solar::core::kTwoPi / mean_motion / solar::core::kSecondsPerDay;

    const auto later = solar::core::state_from_kepler(
        kSunMu, elements,
        solar::core::Epoch{solar::core::kJ2000Jd + orbital_period_days});

    CHECK(later.position.km.x == Approx(start.position.km.x).margin(1e3));
    CHECK(later.position.km.y == Approx(start.position.km.y).margin(1e3));
}

TEST_CASE("state_from_kepler advances one quarter orbit in a quarter year", "[kepler]") {
    const solar::core::KeplerianElements elements{
        .semi_major_axis_km = solar::core::kAuKm,
        .eccentricity = 0.0,
        .inclination_rad = 0.0,
        .longitude_ascending_node_rad = 0.0,
        .argument_periapsis_rad = 0.0,
        .mean_anomaly_at_epoch_rad = 0.0,
        .epoch = solar::core::Epoch{solar::core::kJ2000Jd},
    };

    const auto quarter_orbit = solar::core::state_from_kepler(
        kSunMu, elements,
        solar::core::Epoch{solar::core::kJ2000Jd + 365.25 / 4.0});

    CHECK(quarter_orbit.position.km.x == Approx(0.0).margin(2e4));
    CHECK(quarter_orbit.position.km.y == Approx(solar::core::kAuKm).margin(2e4));
}