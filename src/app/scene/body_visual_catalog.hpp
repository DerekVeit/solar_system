#pragma once

#include "app/color.hpp"
#include "app/scene/body_visual.hpp"
#include "app/scene/scene.hpp"
#include "core/ephemeris.hpp"

#include <span>
#include <string_view>

namespace solar::app {

/// Presentation settings for one simulated body in the scene view.
struct BodyVisualSpec {
    std::string_view name;
    Color color;
    double tail_duration_days{30.0};
    float display_size_factor{500.0f};
};

[[nodiscard]] std::span<const BodyVisualSpec> default_body_visual_specs();

[[nodiscard]] BodyVisual make_body_visual(const BodyVisualSpec& spec,
                                          const std::vector<core::BodyDefinition>& catalog);

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog);

} // namespace solar::app
