#pragma once

namespace solar::app {

struct Color {
    float r;
    float g;
    float b;
    float a;
};

Color color_from_angle(double angle);

} // namespace solar::app
