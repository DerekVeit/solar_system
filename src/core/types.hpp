#pragma once

#include <chrono>
#include <ctime>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <glm/glm.hpp>
#include <string>

namespace solar::core {

/// Julian Date (days).  J2000.0 = 2451545.0 JD.
using JulianDate = double;

/// Seconds since J2000.0 TT (approximated as UTC for this simulation).
using Duration = std::chrono::duration<double>;

struct Epoch {
    JulianDate jd{};

    [[nodiscard]] Duration since_j2000() const {
        constexpr JulianDate kJ2000 = 2451545.0;
        return Duration{(jd - kJ2000) * 86400.0};
    }

    static Epoch from_j2000_offset(Duration offset) {
        constexpr JulianDate kJ2000 = 2451545.0;
        return Epoch{kJ2000 + offset.count() / 86400.0};
    }

    static Epoch now() {
        using namespace std::chrono;
        const auto now = std::chrono::system_clock::now();
        const sys_days j2000_date{year{2000}/January/1};
        const sys_seconds j2000 = j2000_date + 12h;
        constexpr JulianDate kJ2000 = 2451545.0;
        const JulianDate jd_now = kJ2000 + (duration_cast<seconds>(now - j2000).count() / 86400.0);
        return Epoch{jd_now};
    }

    [[nodiscard]] std::string to_string() const {
        using namespace std::chrono;
        const sys_days j2000_date{year{2000}/January/1};
        const sys_seconds j2000 = j2000_date + 12h;
        const auto offset = since_j2000();
        const auto tp = j2000 + duration_cast<seconds>(offset);
        std::string ts = fmt::format("{:%Y-%m-%d %H:%M:%S} UT (approx)", tp);
        return ts;
    }
};

struct Displacement {
    glm::dvec3 km{};

    [[nodiscard]] double length() const { return glm::length(km); }
};

struct Velocity {
    glm::dvec3 km_per_s{};
};

struct StateVector {
    Displacement position{};
    Velocity velocity{};
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

}  // namespace solar::core