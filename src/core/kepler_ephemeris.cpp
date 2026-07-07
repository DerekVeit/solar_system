#include "core/kepler_ephemeris.hpp"

#include <stdexcept>
#include <string>

#include "core/kepler.hpp"

namespace solar::core {

KeplerEphemeris::KeplerEphemeris(std::vector<BodyDefinition> bodies)
    : bodies_(std::move(bodies)) {}

StateVector KeplerEphemeris::state(const std::string& body_name, Epoch epoch) const {
    double central_mu = 0.0;
    for (const BodyDefinition& body : bodies_) {
        if (body.name == "Sun") {
            central_mu = body.gravitational_parameter_km3_s2;
            break;
        }
    }
    if (central_mu <= 0.0) {
        throw std::runtime_error(
            "KeplerEphemeris requires a Sun entry with gravitational parameter");
    }

    for (const BodyDefinition& body : bodies_) {
        if (body.name == body_name) {
            if (body.name == "Sun" || body.elements.semi_major_axis_km <= 0.0) {
                return StateVector{};
            }
            return state_from_kepler(central_mu, body.elements, epoch);
        }
    }
    throw std::invalid_argument("unknown body: " + body_name);
}

} // namespace solar::core