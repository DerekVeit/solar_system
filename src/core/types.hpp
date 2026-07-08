#pragma once

#include "core/constants.hpp"

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <glm/glm.hpp>

#include <chrono>
#include <string>

namespace solar::core {

/// Julian Date (days).  J2000.0 = 2451545.0 JD.
using JulianDate = double;

/// Seconds since J2000.0 TT (approximated as UTC for this simulation).
using Duration = std::chrono::duration<double>;

using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>;

struct Epoch {
    JulianDate jd{};

    [[nodiscard]] Duration since_j2000() const {
        constexpr JulianDate kJ2000 = 2451545.0;
        return Duration{(jd - kJ2000) * kSecondsPerDay};
    }

    static Epoch from_j2000_offset(Duration offset) {
        constexpr JulianDate kJ2000 = 2451545.0;
        return Epoch{kJ2000 + offset.count() / kSecondsPerDay};
    }

    static Epoch at_now() {
        using namespace std::chrono;
        const auto now = std::chrono::system_clock::now();
        const sys_days j2000_date{year{2000} / January / 1};
        const sys_seconds j2000 = j2000_date + 12h;
        constexpr JulianDate kJ2000 = 2451545.0;
        const JulianDate jd_now =
            kJ2000 + (duration_cast<seconds>(now - j2000).count() / kSecondsPerDay);
        return Epoch{jd_now};
    }

    [[nodiscard]] TimePoint now() const {
        using namespace std::chrono;
        const sys_days j2000_date{year{2000} / January / 1};
        const sys_seconds j2000 = j2000_date + 12h;
        const auto offset = since_j2000();
        const auto tp = j2000 + duration_cast<seconds>(offset);
        return tp;
    }

    [[nodiscard]] std::string to_string() const {
        const auto tp = now();
        std::string ts = fmt::format("{:%Y-%m-%d %H:%M:%S} UT (approx)", tp);
        return ts;
    }
};

struct Polar {
    double length;
    double angle;
};

struct Displacement {
    glm::dvec3 km{};

    [[nodiscard]] double length() const { return glm::length(km); }

    [[nodiscard]] Polar polar_xy() const {
        const double length{std::sqrt(km.x * km.x + km.y * km.y)};
        const double raw_angle{std::atan2(km.y, km.x)};
        const double angle{raw_angle < 0.0 ? raw_angle + kTwoPi : raw_angle};
        return Polar{length, angle};
    }
};

struct Velocity {
    glm::dvec3 km_per_s{};
};

struct StateVector {
    Displacement position{};
    Velocity velocity{};

    std::string to_string() {
        const auto polar = position.polar_xy();
        return fmt::format("{:.0f} {:.0f} {:.0f} km ({:.0f} km @ {:.0f}°)", position.km.x,
                           position.km.y, position.km.z, polar.length, polar.angle * kRadToDeg);
    }
};

struct KeplerianElements {
    double semi_major_axis_km{};
    double eccentricity{};
    double inclination_rad{};
    double longitude_ascending_node_rad{};
    double argument_periapsis_rad{};
    double mean_anomaly_at_epoch_rad{};
    Epoch epoch{};
};

} // namespace solar::core