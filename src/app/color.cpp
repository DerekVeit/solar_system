#include "app/color.hpp"
#include "core/constants.hpp"

#include <cmath>
#include <limits>

namespace {

static bool zero(float f1) { return f1 < std::numeric_limits<float>::epsilon(); }

static bool equal(float f1, float f2) {
    return (std::fabs(f1 - f2) <= std::numeric_limits<float>::epsilon());
}

}; // namespace

namespace solar::app {

ColorHSV Color::as_hsv() const {
    ColorHSV hsv;

    float min = r < g ? r : g;
    min = min < b ? min : b;

    float max = r > g ? r : g;
    max = max > b ? max : b;

    // value
    hsv.v = max;

    const float chroma = max - min;
    if (zero(chroma)) {
        hsv.s = 0.0f;
        hsv.h = 0.0f;
        return hsv;
    }

    // saturation
    hsv.s = max > 0.0f ? chroma / max : 0.0f;

    // hue
    if (equal(r, max)) {
        hsv.h = (core::kPi / 3.0f) * ((g - b) / chroma);
    } else if (equal(g, max)) {
        hsv.h = (core::kPi / 3.0f) * ((b - r) / chroma + 2.0f);
    } else {
        hsv.h = (core::kPi / 3.0f) * ((r - g) / chroma + 4.0f);
    }

    return hsv;
}

bool ColorHSV::operator==(const ColorHSV& other) const {
    return equal(h, other.h) && equal(s, other.s) && equal(v, other.v);
}

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
