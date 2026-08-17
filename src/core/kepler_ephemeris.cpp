#include "core/kepler_ephemeris.hpp"

#include "core/body_orientation.hpp"
#include "core/ephemeris.hpp"
#include "core/kepler.hpp"
#include "core/types.hpp"

#include <stdexcept>
#include <string>
#include <unordered_set>

namespace solar::core {

namespace {

void validate_primary_hierarchy(const std::vector<BodyDefinition>& bodies) {
    auto lookup = [&](const std::string& name) -> const BodyDefinition* {
        for (const BodyDefinition& body : bodies) {
            if (body.name == name) {
                return &body;
            }
        }
        return nullptr;
    };

    for (const BodyDefinition& body : bodies) {
        if (body.primary.empty()) {
            continue;
        }
        if (body.primary == body.name) {
            throw std::invalid_argument(body.name + " cannot be its own primary");
        }
        if (lookup(body.primary) == nullptr) {
            throw std::invalid_argument(body.name + " has unknown primary: " + body.primary);
        }

        std::unordered_set<std::string> seen{body.name};
        std::string current = body.primary;
        while (!current.empty()) {
            if (seen.contains(current)) {
                throw std::invalid_argument("orbital primary cycle involving " + body.name);
            }
            seen.insert(current);
            const BodyDefinition* node = lookup(current);
            if (node == nullptr) {
                break;
            }
            current = node->primary;
        }
    }
}

StateVector compose(const StateVector& primary, const StateVector& relative) {
    return StateVector{
        .position = Displacement{primary.position.km + relative.position.km},
        .velocity = Velocity{primary.velocity.km_per_s + relative.velocity.km_per_s},
    };
}

} // namespace

KeplerEphemeris::KeplerEphemeris(std::vector<BodyDefinition> bodies)
    : bodies_(std::move(bodies)) {
    validate_primary_hierarchy(bodies_);
}

StateVector KeplerEphemeris::kepler_relative(const BodyDefinition& body, Epoch epoch) const {
    if (body.elements.semi_major_axis_km <= 0.0) {
        return StateVector{};
    }
    const double mu = orbital_mu(*this, body);
    if (mu <= 0.0) {
        throw std::runtime_error("no gravitational parameter for primary of " + body.name);
    }
    return state_from_kepler(mu, body.elements, epoch);
}

StateVector KeplerEphemeris::relative_state(const std::string& body_name, Epoch epoch) const {
    const BodyDefinition* body = find_body(*this, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }
    return kepler_relative(*body, epoch);
}

StateVector KeplerEphemeris::state(const std::string& body_name, Epoch epoch) const {
    const BodyDefinition* body = find_body(*this, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }

    const StateVector relative = kepler_relative(*body, epoch);
    if (body->primary.empty()) {
        return relative;
    }
    return compose(state(body->primary, epoch), relative);
}

double KeplerEphemeris::rotation_deg(const std::string& body_name, Epoch epoch) const {
    const BodyDefinition* body = find_body(*this, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }
    return rotation_deg_at_epoch(*body, epoch);
}

} // namespace solar::core
