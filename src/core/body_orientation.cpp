#include "core/body_orientation.hpp"

#include "core/constants.hpp"
#include "core/types.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace solar::core {

namespace {

double modulo(double a, double b) { return !b ? a : a - b * floor(a / b); }

} // namespace

[[nodiscard]] double rotation_deg_at_epoch(const BodyDefinition& body, Epoch epoch) {
    // W(t) = W0 + Ẇ * d  (IAU WGCCRE); d = days from rotation.epoch.
    const BodyRotation rotation = body.rotation;
    const double days = epoch.jd - rotation.epoch.jd;
    return modulo(rotation.W0_deg + rotation.W_dot_deg_per_day * days, 360.0);
}

[[nodiscard]] glm::dmat3 body_orientation_matrix(const BodyDefinition& body, Epoch epoch) {
    const double spin_rad = rotation_deg_at_epoch(body, epoch) * kDegToRad;
    const double obliquity_rad = body.obliquity_deg * kDegToRad;

    const glm::dmat3 R_spin = glm::rotate(glm::dmat4{1.0}, spin_rad, {0, 0, 1});
    const glm::dmat3 R_obliquity = glm::rotate(glm::dmat4{1.0}, obliquity_rad, {-1, 0, 0});

    return R_obliquity * glm::dmat3(R_spin);
}

} // namespace solar::core
