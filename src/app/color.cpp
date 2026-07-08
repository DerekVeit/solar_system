#include "app/color.hpp"

#include <cmath>

namespace solar::app {

Color color_from_angle(double angle) {
    float red(0.0f);
    float green(0.0f);
    float blue(0.0f);

    if (std::cos(angle) < 0) {
        green = -std::cos(angle);
    } else {
        red = std::cos(angle);
    }
    blue = ((-std::sin(angle) + 1.0f) / 2.0f) * 0.8f + 0.2f;

    return Color{red, green, blue, 1.0f};
}

} // namespace solar::app
