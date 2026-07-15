#pragma once

#include "fmt/format.h"

#include <string>

namespace solar::app {

struct ColorHSV {
    float h = 0.0f; // radians, [0, 2π)
    float s = 0.0f;
    float v = 0.0f;

    [[nodiscard]] inline std::string to_string() const {
        return fmt::format("{:.02f} {:.02f} {:.02f}", h, s, v);
    }

    [[nodiscard]] bool operator==(const ColorHSV& other) const;
};

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    [[nodiscard]] inline std::string to_string() const {
        return fmt::format("{:.02f} {:.02f} {:.02f} {:.02f}", r, g, b, a);
    }

    [[nodiscard]] bool operator==(const Color& other) const;

    static Color from_hsv(ColorHSV hsv);

    [[nodiscard]] ColorHSV as_hsv() const; // alpha is ignored
};

} // namespace solar::app
