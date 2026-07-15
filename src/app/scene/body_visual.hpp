#pragma once

#include "app/color.hpp"
#include "app/render/types.hpp"
#include "core/constants.hpp"
#include "sim/solar_system.hpp"

#include <string>
#include <vector>

namespace solar::app {

/// Visual representation of one simulated body in the ecliptic view.
class BodyVisual {
  public:
    BodyVisual(std::string name, Color color, float point_size);

    [[nodiscard]] const std::string& name() const { return name_; }

    void append_point(const sim::SolarSystem& simulation, float view_half_extent_au,
                      float aspect_ratio, std::vector<PointInstance>& points) const;

  private:
    std::string name_;
    Color color_;
    float point_size_{1.0f};
};

} // namespace solar::app