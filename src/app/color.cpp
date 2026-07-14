#include "app/color.hpp"
#include "core/constants.hpp"

#include <cmath>

namespace {

inline constexpr float kColorEpsilon = 1e-7f;

static bool zero(float f1) { return f1 < kColorEpsilon; }

static bool equal(float f1, float f2) { return (std::fabs(f1 - f2) <= kColorEpsilon); }

}; // namespace

namespace solar::app {

bool Color::operator==(const Color& other) const {
    return equal(r, other.r) && equal(g, other.g) && equal(b, other.b) && equal(a, other.a);
}

Color Color::from_hsv(ColorHSV hsv) {

    float chroma = hsv.s * hsv.v;
    float sector = hsv.h * 3 / core::kPi;
    float second = chroma * (1 - std::fabs(std::fmod(sector, 2.0f) - 1.0f));
    float m = hsv.v - chroma;

    float c = chroma + m;
    float x = second + m;
    switch (static_cast<int>(std::floor(sector))) {
        case 0:
            return Color{c, x, m, 1.0f};
        case 1:
            return Color{x, c, m, 1.0f};
        case 2:
            return Color{m, c, x, 1.0f};
        case 3:
            return Color{m, x, c, 1.0f};
        case 4:
            return Color{x, m, c, 1.0f};
        case 5:
            return Color{c, m, x, 1.0f};
        default:
            return Color{0.0f, 0.0f, 0.0f, 0.0f};
    }
}

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

    float factor = 0.12;
    red *= factor;
    green *= factor;
    blue *= factor;

    return Color{red, green, blue, 1.0f};
}

} // namespace solar::app
