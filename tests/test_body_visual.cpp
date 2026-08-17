#include "app/scene/body_visual_loader.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

TEST_CASE("moon_orbit_display_size_factor defaults to 1", "[body_visual][json]") {
    const auto config = solar::app::load_body_visual_config("assets/data/body_visuals.json");
    CHECK(config.defaults.moon_orbit_display_size_factor == Approx(1.0f));
    REQUIRE(config.by_name.contains("Earth"));
    CHECK(config.by_name.at("Earth").moon_orbit_display_size_factor == Approx(1.0f));
}
