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

const BodyDefinition* primary_body(const EphemerisProvider& ephemeris, const BodyDefinition& body) {
    if (body.primary.empty()) {
        return find_body(ephemeris, "Sun");
    }
    return find_body(ephemeris, body.primary);
}

double orbital_mu(const EphemerisProvider& ephemeris, const BodyDefinition& body) {
    const BodyDefinition* primary = primary_body(ephemeris, body);
    if (primary == nullptr) {
        return 0.0;
    }
    return primary->gravitational_parameter_km3_s2;
}

} // namespace solar::core