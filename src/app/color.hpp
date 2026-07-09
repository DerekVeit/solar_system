#pragma once

#include "fmt/format.h"

#include <string>

namespace solar::app {

struct Color {
    float r;
    float g;
    float b;
    float a;

    [[nodiscard]] inline std::string to_string() const {
        return fmt::format("{:.02f} {:.02f} {:.02f} {:.02f}", r, g, b, a);
    }
};

Color color_from_angle(double angle);

} // namespace solar::app
