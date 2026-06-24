#pragma once

#include <chrono>
#include <glm/glm.hpp>

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