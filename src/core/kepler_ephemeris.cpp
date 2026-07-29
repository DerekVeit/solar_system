#include "core/kepler_ephemeris.hpp"

#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/types.hpp"

#include <stdexcept>
#include <string>

namespace solar::core {

namespace {

double modulo(double a, double b) { return !b ? a : a - b * floor(a / b); }

} // namespace

KeplerEphemeris::KeplerEphemeris(std::vector<BodyDefinition> bodies)
    : bodies_(std::move(bodies)) {}

StateVector KeplerEphemeris::state(const std::string& body_name, Epoch epoch) const {
    const double central_mu = central_gravitational_parameter(*this);
    if (central_mu <= 0.0) {
        throw std::runtime_error(
            "KeplerEphemeris requires a Sun entry with gravitational parameter");
    }

    const BodyDefinition* body = find_body(*this, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }
    if (body->name == "Sun" || body->elements.semi_major_axis_km <= 0.0) {
        return StateVector{};
    }
    return state_from_kepler(central_mu, body->elements, epoch);
}

double KeplerEphemeris::rotation_deg(const std::string& body_name, Epoch epoch) const {
    const BodyDefinition* body = find_body(*this, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }
    const BodyRotation rotation = body->rotation;
    if (!rotation.period_s) {
        return rotation.prime_meridian_deg_at_epoch;
    }
    const Duration elapsed{(epoch.jd - rotation.epoch.jd) * kSecondsPerDay};
    const double rotations = elapsed.count() / rotation.period_s;
    return modulo((rotation.prime_meridian_deg_at_epoch + 360 * rotations), 360.0);
}

} // namespace solar::core