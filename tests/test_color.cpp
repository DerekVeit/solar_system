#include "app/color.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Color.as_hsv provdes correct HSV values", "[color]") {
    const solar::app::Color color{1.0f, 0.0f, 0.0f, 0.0f};
    solar::app::ColorHSV hsv = color.as_hsv();
    CHECK(hsv.h == 0.0f);
    CHECK(hsv.s == 1.0f);
    CHECK(hsv.v == 1.0f);
}
