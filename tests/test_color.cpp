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

struct ColorRoundTripCase {
    std::string label;
    Color color;
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

TEST_CASE("round trip RGB->HSV->RGB", "[color]") {
    const ColorRoundTripCase test_case = GENERATE(
        ColorRoundTripCase{"butterscotch", Color{0.7568627f, 0.4901960f, 0.0666666f, 1.0f}},
        ColorRoundTripCase{"subtle gray", Color{0.5333333f, 0.5411765f, 0.5215686f, 1.0f}},
        ColorRoundTripCase{"pure gray", Color{0.5f, 0.5f, 0.5f, 1.0f}},
        ColorRoundTripCase{"black", Color{0.0f, 0.0f, 0.0f, 1.0f}},
        ColorRoundTripCase{"white", Color{1.0f, 1.0f, 1.0f, 1.0f}},
        ColorRoundTripCase{"chartreuse", Color{0.7882353f, 0.9607843f, 0.5215686f, 1.0f}},
        ColorRoundTripCase{"dark brown", Color{0.2235294f, 0.2039216f, 0.1333333f, 1.0f}},
        ColorRoundTripCase{"soft blue", Color{0.2039216f, 0.3960784f, 0.6431373f, 1.0f}});

    INFO(test_case.label);

    ColorHSV hsv = test_case.color.as_hsv();
    auto returned_color = Color::from_hsv(hsv);

    INFO(test_case.color.to_string());
    INFO(returned_color.to_string());

    CHECK(returned_color == test_case.color);
}
