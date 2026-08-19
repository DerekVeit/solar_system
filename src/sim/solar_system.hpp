#pragma once

#include "core/ephemeris.hpp"
#include "core/types.hpp"
#include "sim/clock.hpp"

#include <string>
#include <vector>

namespace solar::sim {

class SolarSystem {
  public:
    SolarSystem(core::EphemerisProviderPtr ephemeris, SimulationClock clock);

    [[nodiscard]] const SimulationClock& clock() const { return clock_; }
    [[nodiscard]] SimulationClock& clock() { return clock_; }

    [[nodiscard]] core::StateVector state(const std::string& body_name) const;
    [[nodiscard]] double rotation_deg(const std::string& body_name) const;
    [[nodiscard]] glm::dmat3 orientation(const std::string& body_name) const;
    [[nodiscard]] const core::EphemerisProvider& ephemeris() const { return *ephemeris_; }

    /// Names of bodies that orbit `primary_name`, sorted by semi-major axis.
    [[nodiscard]] std::vector<std::string> bodies_orbiting(const std::string& primary_name) const;

  private:
    core::EphemerisProviderPtr ephemeris_;
    SimulationClock clock_;
};

} // namespace solar::sim