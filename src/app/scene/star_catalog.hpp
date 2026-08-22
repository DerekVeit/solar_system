#pragma once

#include "app/color.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace solar::app {

struct Star {
    std::string name{};
    double ra_deg{};
    double dec_deg{};
    /// Angular radius of this marker; 0 means use the catalog default.
    float marker_radius_deg{0.0f};
};

struct StarCatalog {
    static constexpr std::size_t kMarkerSamples = 48;
    static constexpr float kDistanceFarFraction = 0.9f;

    std::vector<Star> stars{};
    Color color{1.0f, 0.85f, 0.35f, 0.9f};
    float marker_radius_deg{0.35f};
    bool visible{true};
};

} // namespace solar::app
