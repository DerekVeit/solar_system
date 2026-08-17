#pragma once

#include "app/color.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace solar::app {

struct BodySurfaceTextures {
    float longitude_offset_deg{0.0f};
    std::string diffuse;
    std::string night;
    std::string clouds;
    std::string normal;
    std::string specular;
};

/// Shading of a body's spherical surface (authored in body_visuals.json).
struct BodySurface {
    Color color{};
    float ambient{};
    float emission{};
    BodySurfaceTextures textures{};
};

struct RingSpec {
    std::string map;
    double inner_radius_km;
    double outer_radius_km;
};

/// Fully resolved visual parameters for one body (defaults merged with overrides).
struct BodyVisualSpec {
    BodySurface surface{};
    std::vector<RingSpec> rings{};
    double tail_duration_days{};
    float display_size_factor{};
    /// Stretch applied to this body's satellites (primary → moon vectors) when
    /// body scaling is on. Ignored on the satellites themselves.
    float moon_orbit_display_size_factor{1.0f};
    bool visible{};
};

/// Loaded body_visuals.json: complete defaults plus per-name resolved specs.
struct BodyVisualConfig {
    BodyVisualSpec defaults{};
    std::unordered_map<std::string, BodyVisualSpec> by_name{};
};

} // namespace solar::app
