#include "sim/solar_system.hpp"
#include "app/logging.hpp"
#include "core/ephemeris.hpp"

namespace {
using solar::app::log;
}

namespace solar::sim {

SolarSystem::SolarSystem(core::EphemerisProviderPtr ephemeris, SimulationClock clock)
    : ephemeris_(std::move(ephemeris))
    , clock_(clock) {}

core::StateVector SolarSystem::state(const std::string& body_name) const {
    return ephemeris_->state(body_name, clock_.epoch());
}

void SolarSystem::change_acceleration(double multiplier) {
    const double accel = clock_.acceleration() * multiplier;
    const char* direction = multiplier < 1.0 ? "slower" : "faster";
    log("{}  {}: {}", clock_.epoch().to_string(), direction, accel);
    clock_.set_acceleration(accel);
    if (clock_.time_scale() != solar::sim::TimeScale::accelerated) {
        log("(not currently in the accelerated time scale)");
    }
}

} // namespace solar::sim