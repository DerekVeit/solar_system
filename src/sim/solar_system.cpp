#include "sim/solar_system.hpp"

#include "core/ephemeris.hpp"

namespace solar::sim {

SolarSystem::SolarSystem(core::EphemerisProviderPtr ephemeris, SimulationClock clock)
    : ephemeris_(std::move(ephemeris))
    , clock_(clock) {}

core::StateVector SolarSystem::state(const std::string& body_name) const {
    return ephemeris_->state(body_name, clock_.epoch());
}

double SolarSystem::rotation_deg(const std::string& body_name) const {
    return ephemeris_->rotation_deg(body_name, clock_.epoch());
}

} // namespace solar::sim