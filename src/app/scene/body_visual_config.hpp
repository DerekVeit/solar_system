#pragma once

#include "app/color.hpp"

#include <optional>
#include <string>
#include <vector>

namespace solar::app {

struct BodyVisualDefaults {
    Color color{0.7f, 0.7f, 0.8f, 1.0f};
    double tail_duration_days{30.0};
    float display_size_factor{500.0f};
    bool visible{true};
};

struct BodyVisualOverrideEntry {
    std::string name;
    std::optional<Color> color;
    std::optional<double> tail_duration_days;
    std::optional<float> display_size_factor;
    std::optional<bool> visible;
};

struct BodyVisualSettings {
    Color color;
    double tail_duration_days;
    float display_size_factor;
    bool visible;
};

struct BodyVisualConfig {
    BodyVisualDefaults defaults;
    std::vector<BodyVisualOverrideEntry> overrides;
};

} // namespace solar::app
