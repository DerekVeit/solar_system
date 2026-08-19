#include "core/json_loader.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("load_bodies reads bundled planet data", "[json]") {
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    REQUIRE(bodies.size() == 31);
    CHECK(bodies[0].name == "Sun");
    CHECK(bodies[1].name == "Mercury");
    CHECK(bodies[3].name == "Earth");
    CHECK(bodies[3].primary.empty());
    CHECK(bodies[3].elements.semi_major_axis_km > 149000000.0);
    CHECK(bodies[4].name == "Moon");
    CHECK(bodies[4].primary == "Earth");
    CHECK(bodies[6].name == "Phobos");
    CHECK(bodies[6].primary == "Mars");
    CHECK(bodies[9].name == "Io");
    CHECK(bodies[9].primary == "Jupiter");
    CHECK(bodies[29].name == "Pluto");
    CHECK(bodies[29].primary.empty());
    CHECK(bodies[30].name == "Charon");
    CHECK(bodies[30].primary == "Pluto");
}
