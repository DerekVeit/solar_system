#include "app/color.hpp"
#include "core/constants.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>

namespace {

using solar::app::Color;
using solar::app::ColorHSV;

struct ColorToHSVCase {
    std::string label;
    Color color;
    ColorHSV expected_hsv;
};

struct ColorFromHSVCase {
    std::string label;
    ColorHSV hsv;
    Color expected_color;
};

} // namespace

TEST_CASE("Color.as_hsv provides correct HSV values", "[color]") {
    const ColorToHSVCase test_case = GENERATE(
        ColorToHSVCase{"pure red", Color{1.0f, 0.0f, 0.0f, 1.0f}, ColorHSV{0.0f, 1.0f, 1.0f}},
        ColorToHSVCase{"pink", Color{1.0f, 0.5f, 0.5f, 1.0f}, ColorHSV{0.0f, 0.5f, 1.0f}},
        ColorToHSVCase{"cyan", Color{0.0f, 1.0f, 1.0f, 1.0f},
                       ColorHSV{solar::core::kPi, 1.0f, 1.0f}},
        ColorToHSVCase{"medium green", Color{0.0f, 0.5f, 0.0f, 1.0f},
                       ColorHSV{solar::core::kPi * 2 / 3, 1.0f, 0.5f}});

    INFO(test_case.label);

    ColorHSV hsv = test_case.color.as_hsv();

    INFO(test_case.expected_hsv.to_string());
    INFO(hsv.to_string());

    CHECK(hsv == test_case.expected_hsv);
}

TEST_CASE("Color::from_hsv provides correct RGB values", "[color]") {
    const ColorFromHSVCase test_case = GENERATE(
        ColorFromHSVCase{"pure red", ColorHSV{0.0f, 1.0f, 1.0f}, Color{1.0f, 0.0f, 0.0f, 1.0f}},
        ColorFromHSVCase{"pink", ColorHSV{0.0f, 0.5f, 1.0f}, Color{1.0f, 0.5f, 0.5f, 1.0f}},
        ColorFromHSVCase{"cyan", ColorHSV{solar::core::kPi, 1.0f, 1.0f},
                         Color{0.0f, 1.0f, 1.0f, 1.0f}},
        ColorFromHSVCase{"medium green", ColorHSV{solar::core::kPi * 2 / 3, 1.0f, 0.5f},
                         Color{0.0f, 0.5f, 0.0f, 1.0f}});

    INFO(test_case.label);

    auto color = Color::from_hsv(test_case.hsv);

    INFO(test_case.expected_color.to_string());
    INFO(color.to_string());

    CHECK(color == test_case.expected_color);
}
