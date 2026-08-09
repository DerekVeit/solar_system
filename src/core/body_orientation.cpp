#include "core/body_orientation.hpp"

#include "core/constants.hpp"
#include "core/types.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace solar::core {

namespace {

double modulo(double a, double b) { return !b ? a : a - b * floor(a / b); }

glm::dmat3 rotate_x(double angle_deg) {
    return glm::rotate(glm::dmat4{1.0}, angle_deg * kDegToRad, {1, 0, 0});
}

glm::dmat3 rotate_z(double angle_deg) {
    return glm::rotate(glm::dmat4{1.0}, angle_deg * kDegToRad, {0, 0, 1});
}

} // namespace

[[nodiscard]] double rotation_deg_at_epoch(const BodyDefinition& body, Epoch epoch) {
    // W(t) = W0 + Ẇ * d  (IAU WGCCRE); d = days from rotation.epoch.
    const BodyRotation rotation = body.rotation;
    const double days = epoch.jd - rotation.epoch.jd;
    return modulo(rotation.W0_deg + rotation.W_dot_deg_per_day * days, 360.0);
}

[[nodiscard]] glm::dmat3 body_orientation_matrix(const BodyDefinition& body, Epoch epoch) {
    const glm::dmat3 R_icrf = rotate_z(90.0 + body.pole.ra_deg) *
                              rotate_x(90.0 - body.pole.dec_deg) *
                              rotate_z(rotation_deg_at_epoch(body, epoch));

    const glm::dmat3 R_ecl = rotate_x(-23.44);

    return R_ecl * R_icrf;
}

} // namespace solar::core
