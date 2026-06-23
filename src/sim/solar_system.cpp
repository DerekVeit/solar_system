#include "sim/solar_system.hpp"

namespace solar::sim {

SolarSystem::SolarSystem(std::unique_ptr<core::EphemerisProvider> ephemeris, SimulationClock clock)
    : ephemeris_(std::move(ephemeris)), clock_(clock) {}

core::StateVector SolarSystem::state(const std::string& body_name) const {
    return ephemeris_->state(body_name, clock_.epoch());
}

}  // namespace solar::sim