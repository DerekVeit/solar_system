#pragma once

#include "app/color.hpp"
#include "app/scene/body_visual.hpp"
#include "app/scene/scene.hpp"
#include "core/ephemeris.hpp"

#include <optional>
#include <string_view>

namespace solar::app {

struct BodyVisualDefaults {
    Color color{0.7f, 0.7f, 0.8f, 1.0f};
    double tail_duration_days{30.0};
    float display_size_factor{500.0f};
};

struct BodyVisualOverride {
    std::string_view name;
    std::optional<Color> color;
    std::optional<double> tail_duration_days;
    std::optional<float> display_size_factor;
};

struct BodyVisualSettings {
    Color color;
    double tail_duration_days;
    float display_size_factor;
};

[[nodiscard]] BodyVisualDefaults default_body_visual_defaults();

[[nodiscard]] BodyVisualSettings resolve_body_visual_settings(const BodyVisualDefaults& defaults,
                                                              const BodyVisualOverride* override);

[[nodiscard]] BodyVisual make_body_visual(const core::BodyDefinition& body,
                                          const BodyVisualSettings& settings);

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog);

} // namespace solar::app
