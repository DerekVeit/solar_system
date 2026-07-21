#pragma once

#include "app/color.hpp"

#include <optional>
#include <string>
#include <vector>

namespace solar::app {

struct BodyVisualDefaults {
    Color color{0.7f, 0.7f, 0.8f, 1.0f};
    float ambient{0.2f};
    float emission{0.0f};
    double tail_duration_days{30.0};
    float display_size_factor{500.0f};
    bool visible{true};
};

struct BodyVisualOverrideEntry {
    std::string name;
    std::optional<Color> color;
    std::optional<float> ambient;
    std::optional<float> emission;
    std::optional<double> tail_duration_days;
    std::optional<float> display_size_factor;
    std::optional<bool> visible;
};

struct BodyVisualSettings {
    Color color;
    float ambient;
    float emission;
    double tail_duration_days;
    float display_size_factor;
    bool visible;
};

struct BodyVisualConfig {
    BodyVisualDefaults defaults;
    std::vector<BodyVisualOverrideEntry> overrides;
};

} // namespace solar::app
