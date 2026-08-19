#include "sim/solar_system.hpp"

#include "core/body_orientation.hpp"
#include "core/ephemeris.hpp"

namespace solar::sim {

SolarSystem::SolarSystem(core::EphemerisProviderPtr ephemeris, SimulationClock clock)
    : ephemeris_(std::move(ephemeris))
    , clock_(clock) {}

core::StateVector SolarSystem::state(const std::string& body_name) const {
    return ephemeris_->state(body_name, clock_.epoch());
}
glm::dmat3 SolarSystem::orientation(const std::string& body_name) const {
    const core::BodyDefinition* body = core::find_body(*ephemeris_, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }
    return body_orientation_matrix(*body, clock_.epoch(), orbital_mu(*ephemeris_, *body));
}

double SolarSystem::rotation_deg(const std::string& body_name) const {
    return ephemeris_->rotation_deg(body_name, clock_.epoch());
}

std::vector<std::string> SolarSystem::bodies_orbiting(const std::string& primary_name) const {
    std::vector<std::string> names;
    for (const core::BodyDefinition* body : core::bodies_orbiting(*ephemeris_, primary_name)) {
        names.push_back(body->name);
    }
    return names;
}

} // namespace solar::sim