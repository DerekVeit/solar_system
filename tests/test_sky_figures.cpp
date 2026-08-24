#include "app/scene/sky_figure_loader.hpp"
#include "app/scene/star_catalog_loader.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using Catch::Approx;

namespace {

std::filesystem::path write_temp_json(const std::string& name, const std::string& body) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream output{path};
    output << body;
    return path;
}

} // namespace

TEST_CASE("constellations.json lists the Big Dipper as an asterism", "[sky][figures][json]") {
    const auto stars = solar::app::load_star_catalog("assets/data/stars.json");
    const auto catalog = solar::app::load_sky_figures("assets/data/constellations.json", stars);

    REQUIRE(catalog.visible);
    CHECK(catalog.line_gap_deg == Approx(0.5f));
    REQUIRE(catalog.figures.size() == 1);

    const solar::app::SkyFigure& dipper = catalog.figures.front();
    CHECK(dipper.id == "dipper");
    CHECK(dipper.name == "Big Dipper");
    CHECK(dipper.kind == solar::app::SkyFigureKind::asterism);
    REQUIRE(dipper.polylines.size() == 2);
    CHECK(dipper.polylines[0] ==
          std::vector<int>({54061, 53910, 58001, 59774, 62956, 65378, 67301}));
    CHECK(dipper.polylines[1] == std::vector<int>({54061, 59774}));
    for (int hip : dipper.polylines[0]) {
        CHECK(solar::app::star_by_hip(stars, hip) != nullptr);
    }
}

TEST_CASE("sky figure loader rejects an unknown HIP", "[sky][figures]") {
    const auto stars = solar::app::load_star_catalog("assets/data/stars.json");
    const auto path = write_temp_json(
        "solar_unknown_hip_figures.json",
        R"({"figures":[{"id":"x","name":"X","kind":"asterism","polylines":[[54061,1]]}]})");
    CHECK_THROWS_AS(solar::app::load_sky_figures(path, stars), std::runtime_error);
}
