#include "app/color.hpp"

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

} // namespace

TEST_CASE("Color.as_hsv provdes correct HSV values", "[color]") {
    const ColorToHSVCase test_case = GENERATE(
        ColorToHSVCase{"pure red", Color{1.0f, 0.0f, 0.0f, 0.0f}, ColorHSV{0.0f, 1.0f, 1.0f}});

    INFO(test_case.label);

    ColorHSV hsv = test_case.color.as_hsv();
    CHECK(hsv.h == test_case.expected_hsv.h);
    CHECK(hsv.s == test_case.expected_hsv.s);
    CHECK(hsv.v == test_case.expected_hsv.v);
}
