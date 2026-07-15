#pragma once

#include "core/types.hpp"

namespace solar::core {

/// Standard gravitational parameter (km^3 / s^2).
using GravitationalParameter = double;

/// Solve Kepler's equation M = E - e sin(E) with Newton-Raphson iteration.
[[nodiscard]] double solve_kepler(double mean_anomaly_rad, double eccentricity);

/// Position in the orbital plane (perifocal frame), km.
[[nodiscard]] Displacement position_in_orbital_plane(const KeplerianElements& elements,
                                                     double eccentric_anomaly);

/// Mean anomaly (radians, [0, 2π)) implied by elements at the given epoch.
[[nodiscard]] double mean_anomaly_at_epoch(GravitationalParameter mu,
                                           const KeplerianElements& elements, Epoch epoch);

/// Heliocentric state from osculating Keplerian elements at the given epoch.
[[nodiscard]] StateVector state_from_kepler(GravitationalParameter mu,
                                            const KeplerianElements& elements, Epoch epoch);

} // namespace solar::core