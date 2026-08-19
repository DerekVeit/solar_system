#include "core/ephemeris.hpp"

#include <algorithm>
#include <string_view>

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

namespace {

[[nodiscard]] bool orbits_primary(const BodyDefinition& body, std::string_view primary_name) {
    const bool heliocentric = primary_name.empty() || primary_name == "Sun";
    if (heliocentric) {
        return body.primary.empty() || body.primary == "Sun";
    }
    return body.primary == primary_name;
}

} // namespace

std::vector<const BodyDefinition*> bodies_orbiting(const EphemerisProvider& ephemeris,
                                                   std::string_view primary_name) {
    std::vector<const BodyDefinition*> result;
    for (const BodyDefinition& body : ephemeris.bodies()) {
        if (orbits_primary(body, primary_name)) {
            result.push_back(&body);
        }
    }
    std::sort(result.begin(), result.end(), [](const BodyDefinition* a, const BodyDefinition* b) {
        return a->elements.semi_major_axis_km < b->elements.semi_major_axis_km;
    });
    return result;
}

const BodyDefinition* innermost_satellite(const EphemerisProvider& ephemeris,
                                          const BodyDefinition& body) {
    for (const BodyDefinition* child : bodies_orbiting(ephemeris, body.name)) {
        if (child->name != body.name && child->elements.semi_major_axis_km > 0.0) {
            return child;
        }
    }
    return nullptr;
}

const BodyDefinition* sibling_by_offset(const EphemerisProvider& ephemeris,
                                        const BodyDefinition& body, int offset) {
    const std::string_view family =
        body.primary.empty() ? std::string_view{"Sun"} : std::string_view{body.primary};
    const std::vector<const BodyDefinition*> peers = bodies_orbiting(ephemeris, family);
    if (peers.empty()) {
        return nullptr;
    }
    const auto it = std::find_if(peers.begin(), peers.end(), [&](const BodyDefinition* peer) {
        return peer->name == body.name;
    });
    if (it == peers.end()) {
        return nullptr;
    }
    const int n = static_cast<int>(peers.size());
    int index = static_cast<int>(it - peers.begin());
    index = (index + offset) % n;
    if (index < 0) {
        index += n;
    }
    return peers[static_cast<std::size_t>(index)];
}

} // namespace solar::core