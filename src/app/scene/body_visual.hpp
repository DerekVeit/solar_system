#pragma once

#include "app/color.hpp"
#include "app/render/types.hpp"
#include "sim/solar_system.hpp"

#include <string>

namespace solar::app {

/// Visual representation of one simulated body in the ecliptic view.
class BodyVisual {
  public:
    static constexpr std::size_t kOrbitSamples = 256;
    static constexpr std::size_t kTailSamples = 48;
    static constexpr float kTailPointSize = 6.0f;

    static constexpr Color kOrbitTrailColor{0.45f, 0.45f, 0.45f, 0.25f};
    static constexpr Color kTailColor{0.55f, 0.65f, 0.85f, 0.75f};

    BodyVisual(std::string name, Color color, float point_size, double tail_duration_days = 30.0);

    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] bool draws_orbit_trails() const { return name_ != "Sun"; }

    void append_draw(const sim::SolarSystem& simulation, float view_half_extent_au,
                     float aspect_ratio, DrawBatch& batch) const;

  private:
    std::string name_;
    Color color_;
    float point_size_{1.0f};
    double tail_duration_seconds_{0.0};
};

} // namespace solar::app