#pragma once

#include "core/ephemeris.hpp"
#include "core/types.hpp"

#include <glm/ext/matrix_double3x3.hpp>

namespace solar::core {

/// Prime-meridian angle W(t) in degrees, wrapped to [0, 360).
/// IAU W0 + Ẇ·d, except tidally locked bodies use Keplerian mean motion
/// (from orbital_mu and a) as Ẇ so spin stays phased with the orbit.
[[nodiscard]] double rotation_deg_at_epoch(const BodyDefinition& body, Epoch epoch,
                                           double orbital_mu = 0.0);

/// Body-fixed → ecliptic draw-frame rotation from IAU (α₀, δ₀, W).
[[nodiscard]] glm::dmat3 body_orientation_matrix(const BodyDefinition& body, Epoch epoch,
                                                 double orbital_mu = 0.0);

} // namespace solar::core
