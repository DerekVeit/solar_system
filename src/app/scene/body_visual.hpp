#pragma once

#include "app/color.hpp"
#include "app/render/types.hpp"
#include "app/scene/view_frame.hpp"
#include "sim/solar_system.hpp"

#include <string>

namespace solar::app {

/// Visual representation of one simulated body.
class BodyVisual {
  public:
    static constexpr std::size_t kOrbitSamples = 256;
    static constexpr std::size_t kTailSamples = 48;
    static constexpr float kMinScreenDiameterPx = 2.0f;

    static constexpr Color kOrbitTrailColor{0.45f, 0.45f, 0.45f, 0.35f};
    static constexpr Color kTailColor{0.55f, 0.65f, 0.85f, 0.55f};

    BodyVisual(std::string name, Color color, float ambient, float emission, double radius_km,
               double tail_duration_days, float display_size_factor, bool draws_orbit_trails);

    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] bool draws_orbit_trails() const { return draws_orbit_trails_; }

    void append_draw(const sim::SolarSystem& simulation, const ViewFrame& view,
                     DrawBatch& batch) const;

  private:
    /// Drawn sphere radius in km (physical radius, display factor, min screen size).
    [[nodiscard]] float drawn_radius_km(const ViewFrame& view) const;

    std::string name_;
    Color color_;
    float ambient_{0.2f};
    float emission_{0.0f};
    double radius_km_{0.0};
    double tail_duration_seconds_{0.0};
    float display_size_factor_{1.0f};
    bool draws_orbit_trails_{false};
};

} // namespace solar::app
