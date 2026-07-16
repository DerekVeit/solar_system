#include "core/kepler_ephemeris.hpp"

#include "core/kepler.hpp"

#include <stdexcept>
#include <string>

namespace solar::core {

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

} // namespace solar::core