#pragma once

#include "core/types.hpp"

namespace solar::core {

/// Standard gravitational parameter (km^3 / s^2).
using GravitationalParameter = double;

/// Wrap radians to [0, 2π).
[[nodiscard]] double normalize_angle(double radians);

/// Mean motion (rad/s) from semi-major axis and central gravitational parameter.
[[nodiscard]] double mean_motion(GravitationalParameter mu, const KeplerianElements& elements);

/// Orbital period (seconds) from semi-major axis and central gravitational parameter.
[[nodiscard]] double orbital_period_seconds(GravitationalParameter mu,
                                            const KeplerianElements& elements);

/// Epoch at which mean anomaly equals target_mean_anomaly, reached by stepping backward from
/// reference_epoch along the orbit.
[[nodiscard]] Epoch epoch_before_mean_anomaly(GravitationalParameter mu,
                                              const KeplerianElements& elements,
                                              Epoch reference_epoch, double target_mean_anomaly);

/// Solve Kepler's equation M = E - e sin(E) with Newton-Raphson iteration.
[[nodiscard]] double solve_kepler(double mean_anomaly_rad, double eccentricity);

/// Position in the orbital plane (perifocal frame), km.
[[nodiscard]] Displacement position_in_orbital_plane(const KeplerianElements& elements,
                                                     double eccentric_anomaly);

/// Mean anomaly (radians, [0, 2π)) implied by elements at the given epoch.
[[nodiscard]] double mean_anomaly_at_epoch(GravitationalParameter mu,
                                           const KeplerianElements& elements, Epoch epoch);

/// Osculating Keplerian state relative to the attracting center at the given epoch.
[[nodiscard]] StateVector state_from_kepler(GravitationalParameter mu,
                                            const KeplerianElements& elements, Epoch epoch);

} // namespace solar::core