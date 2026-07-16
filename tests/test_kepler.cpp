#include "core/constants.hpp"
#include "core/kepler.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <cmath>
#include <string>
#include <tuple>

using Catch::Approx;

namespace {
constexpr double kSunMu = 132712440041.93938; // km³/s²
constexpr double kYearDays = 365.25;

using solar::core::kPi;

struct KeplerCase {
    std::string label;
    double mean_anomaly;
    double eccentricity;
    double expected_eccentric_anomaly;
};
} // namespace

TEST_CASE("solve_kepler recovers expected eccentric anomaly", "[kepler]") {
    const KeplerCase test_case =
        GENERATE(KeplerCase{"circular orbit", kPi / 3.0, 0.0, kPi / 3.0},
                 KeplerCase{"M = pi, e = 0.5", kPi, 0.5, kPi},
                 KeplerCase{"M = 0, e = pi/3", 0.0, kPi / 3.0, kPi / 6.0},
                 KeplerCase{"M = pi/6, e = pi/3", kPi / 6.0, kPi / 3.0, kPi / 2.0});

    INFO(test_case.label);

    const double eccentric_anomaly =
        solar::core::solve_kepler(test_case.mean_anomaly, test_case.eccentricity);

    CHECK(eccentric_anomaly == Approx(test_case.expected_eccentric_anomaly).margin(1e-10));
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

    const auto state =
        solar::core::state_from_kepler(kSunMu, elements, solar::core::Epoch{solar::core::kJ2000Jd});

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

    const auto start =
        solar::core::state_from_kepler(kSunMu, elements, solar::core::Epoch{solar::core::kJ2000Jd});
    const double mean_motion =
        std::sqrt(kSunMu / (solar::core::kAuKm * solar::core::kAuKm * solar::core::kAuKm));
    const double orbital_period_days =
        solar::core::kTwoPi / mean_motion / solar::core::kSecondsPerDay;

    const auto later = solar::core::state_from_kepler(
        kSunMu, elements, solar::core::Epoch{solar::core::kJ2000Jd + orbital_period_days});

    CHECK(later.position.km.x == Approx(start.position.km.x).margin(1e3));
    CHECK(later.position.km.y == Approx(start.position.km.y).margin(1e3));
}

TEST_CASE("state_from_kepler advances circular orbit to expected positions", "[kepler]") {
    using Row = std::tuple<double, double, double>;

    const auto [days, expected_x, expected_y] = GENERATE(table<double, double, double>({
        Row{kYearDays / 4.0, 0.0, solar::core::kAuKm},
        Row{kYearDays / 2.0, -solar::core::kAuKm, 0.0},
        Row{3.0 * kYearDays / 4.0, 0.0, -solar::core::kAuKm},
    }));

    const solar::core::KeplerianElements elements{
        .semi_major_axis_km = solar::core::kAuKm,
        .eccentricity = 0.0,
        .inclination_rad = 0.0,
        .longitude_ascending_node_rad = 0.0,
        .argument_periapsis_rad = 0.0,
        .mean_anomaly_at_epoch_rad = 0.0,
        .epoch = solar::core::Epoch{solar::core::kJ2000Jd},
    };

    const auto state = solar::core::state_from_kepler(
        kSunMu, elements, solar::core::Epoch{solar::core::kJ2000Jd + days});

    CHECK(state.position.km.x == Approx(expected_x).margin(2e4));
    CHECK(state.position.km.y == Approx(expected_y).margin(2e4));
}

TEST_CASE("normalize_angle wraps radians to 0..2pi", "[kepler]") {
    using solar::core::kTwoPi;
    using solar::core::normalize_angle;

    CHECK(normalize_angle(0.0) == Approx(0.0));
    CHECK(normalize_angle(kTwoPi) == Approx(0.0).margin(1e-12));
    CHECK(normalize_angle(-kPi) == Approx(kPi));
    CHECK(normalize_angle(3.0 * kTwoPi + kPi / 4.0) == Approx(kPi / 4.0));
}

TEST_CASE("mean_motion and orbital_period_seconds are consistent", "[kepler]") {
    const solar::core::KeplerianElements elements{
        .semi_major_axis_km = solar::core::kAuKm,
        .eccentricity = 0.0,
        .inclination_rad = 0.0,
        .longitude_ascending_node_rad = 0.0,
        .argument_periapsis_rad = 0.0,
        .mean_anomaly_at_epoch_rad = 0.0,
        .epoch = solar::core::Epoch{solar::core::kJ2000Jd},
    };

    const double motion = solar::core::mean_motion(kSunMu, elements);
    const double period = solar::core::orbital_period_seconds(kSunMu, elements);

    CHECK(motion ==
          Approx(std::sqrt(kSunMu / (solar::core::kAuKm * solar::core::kAuKm * solar::core::kAuKm)))
              .margin(1e-12));
    CHECK(period == Approx(solar::core::kTwoPi / motion).margin(1e-6));
    CHECK(period / solar::core::kSecondsPerDay == Approx(kYearDays).margin(0.5));
}

TEST_CASE("epoch_before_mean_anomaly recovers target mean anomaly", "[kepler]") {
    const solar::core::KeplerianElements elements{
        .semi_major_axis_km = solar::core::kAuKm,
        .eccentricity = 0.1,
        .inclination_rad = 0.0,
        .longitude_ascending_node_rad = 0.0,
        .argument_periapsis_rad = 0.0,
        .mean_anomaly_at_epoch_rad = kPi / 4.0,
        .epoch = solar::core::Epoch{solar::core::kJ2000Jd},
    };

    const solar::core::Epoch reference{solar::core::kJ2000Jd + 17.0};
    const double target_mean_anomaly = kPi / 3.0;
    const solar::core::Epoch earlier =
        solar::core::epoch_before_mean_anomaly(kSunMu, elements, reference, target_mean_anomaly);

    const double recovered = solar::core::mean_anomaly_at_epoch(kSunMu, elements, earlier);

    CHECK(recovered == Approx(target_mean_anomaly).margin(1e-10));
}