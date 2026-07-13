#pragma once

#include "fmt/format.h"

#include <string>

namespace solar::app {

struct ColorHSV {
    float h = 0.0f;
    float s = 0.0f;
    float v = 0.0f;

    [[nodiscard]] inline std::string to_string() const {
        return fmt::format("{:.02f} {:.02f} {:.02f}", h, s, v);
    }

    bool operator==(const ColorHSV& other) const;
};

struct Color {
    float r;
    float g;
    float b;
    float a;

    [[nodiscard]] inline std::string to_string() const {
        return fmt::format("{:.02f} {:.02f} {:.02f} {:.02f}", r, g, b, a);
    }

    [[nodiscard]] ColorHSV as_hsv() const;
};

Color color_from_angle(double angle);

} // namespace solar::app
