#pragma once

#include "core/ephemeris.hpp"
#include "core/types.hpp"

#include <glm/ext/matrix_double3x3.hpp>

namespace solar::core {

/// Prime-meridian angle W(t) in degrees (IAU-style W0 + Ẇ·d), wrapped to [0, 360).
[[nodiscard]] double rotation_deg_at_epoch(const BodyDefinition& body, Epoch epoch);

/// Body-fixed → draw-frame rotation. Currently uses obliquity_deg + W(t) only;
/// BodyPole (α₀, δ₀) is loaded for Option B but not applied here yet.
[[nodiscard]] glm::dmat3 body_orientation_matrix(const BodyDefinition& body, Epoch epoch);

} // namespace solar::core
