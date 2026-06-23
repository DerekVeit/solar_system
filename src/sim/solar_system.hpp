#pragma once

#include <memory>
#include <string>

#include "core/ephemeris.hpp"
#include "core/types.hpp"
#include "sim/clock.hpp"

namespace solar::sim {

class SolarSystem {
  public:
    SolarSystem(std::unique_ptr<core::EphemerisProvider> ephemeris, SimulationClock clock);

    [[nodiscard]] const SimulationClock& clock() const { return clock_; }
    [[nodiscard]] SimulationClock& clock() { return clock_; }

    [[nodiscard]] core::StateVector state(const std::string& body_name) const;
    [[nodiscard]] const core::EphemerisProvider& ephemeris() const { return *ephemeris_; }

  private:
    std::unique_ptr<core::EphemerisProvider> ephemeris_;
    SimulationClock clock_;
};

}  // namespace solar::sim