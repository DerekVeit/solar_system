#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <cmath>
#include <string>

#include "core/constants.hpp"
#include "core/kepler.hpp"

using Catch::Approx;

namespace {
constexpr double kSunMu = 132712440041.93938;

using solar::core::kPi;

struct KeplerCase {
    std::string label;
    double mean_anomaly;
    double eccentricity;
    double expected_eccentric_anomaly;
};
}  // namespace

TEST_CASE("solve_kepler recovers expected eccentric anomaly", "[kepler]") {
    const KeplerCase test_case = GENERATE(
        KeplerCase{"circular orbit", kPi / 3.0, 0.0, kPi / 3.0},
        KeplerCase{"M = pi, e = 0.5", kPi, 0.5, kPi},
        KeplerCase{"M = 0, e = pi/3", 0.0, kPi / 3.0, kPi / 6.0},
        KeplerCase{"M = pi/6, e = pi/3", kPi / 6.0, kPi / 3.0, kPi / 2.0});

    INFO(test_case.label);

    const double eccentric_anomaly =
        solar::core::solve_kepler(test_case.mean_anomaly, test_case.eccentricity);

    CHECK(eccentric_anomaly ==
          Approx(test_case.expected_eccentric_anomaly).margin(1e-10));
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