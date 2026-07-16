#include "core/ephemeris.hpp"

namespace solar::core {

const BodyDefinition* find_body(const EphemerisProvider& ephemeris, const std::string& name) {
    for (const BodyDefinition& body : ephemeris.bodies()) {
        if (body.name == name) {
            return &body;
        }
    }
    return nullptr;
}

double central_gravitational_parameter(const EphemerisProvider& ephemeris) {
    const BodyDefinition* sun = find_body(ephemeris, "Sun");
    if (sun == nullptr) {
        return 0.0;
    }
    return sun->gravitational_parameter_km3_s2;
}

} // namespace solar::core