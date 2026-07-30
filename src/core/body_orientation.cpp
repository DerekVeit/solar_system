#include "core/body_orientation.hpp"

#include "core/constants.hpp"
#include "core/types.hpp"

namespace solar::core {

namespace {

double modulo(double a, double b) { return !b ? a : a - b * floor(a / b); }

} // namespace

[[nodiscard]] double rotation_deg_at_epoch(const BodyDefinition& body, Epoch epoch) {
    const BodyRotation rotation = body.rotation;
    if (!rotation.period_s) {
        return rotation.prime_meridian_deg_at_epoch;
    }
    const Duration elapsed{(epoch.jd - rotation.epoch.jd) * kSecondsPerDay};
    const double rotations = elapsed.count() / rotation.period_s;
    return modulo((rotation.prime_meridian_deg_at_epoch + 360 * rotations), 360.0);
}

} // namespace solar::core

