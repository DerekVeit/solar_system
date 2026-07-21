#pragma once

#include "app/color.hpp"

#include <string>
#include <unordered_map>

namespace solar::app {

/// Shading of a body's spherical surface (authored in body_visuals.json).
struct BodySurface {
    Color color{};
    float ambient{};
    float emission{};
};

/// Fully resolved visual parameters for one body (defaults merged with overrides).
struct BodyVisualSpec {
    BodySurface surface{};
    double tail_duration_days{};
    float display_size_factor{};
    bool visible{};
};

/// Loaded body_visuals.json: complete defaults plus per-name resolved specs.
struct BodyVisualConfig {
    BodyVisualSpec defaults{};
    std::unordered_map<std::string, BodyVisualSpec> by_name{};
};

} // namespace solar::app
