#include "app/scene/sky_loader.hpp"
#include "core/constants.hpp"
#include "core/sky_orientation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>

using Catch::Approx;

namespace {

[[nodiscard]] double wrap_deg(double deg) {
    deg = std::fmod(deg, 360.0);
    if (deg < 0.0) {
        deg += 360.0;
    }
    return deg;
}

} // namespace

TEST_CASE("galactic_from_equatorial maps the IAU pole and centre", "[sky]") {
    const glm::dmat3 R = solar::core::galactic_from_equatorial();
    const glm::dvec3 ngp = solar::core::equatorial_direction(solar::core::kNorthGalacticPoleRaDeg,
                                                             solar::core::kNorthGalacticPoleDecDeg);
    const glm::dvec3 gc = solar::core::equatorial_direction(solar::core::kGalacticCenterRaDeg,
                                                            solar::core::kGalacticCenterDecDeg);

    const glm::dvec3 ngp_gal = R * ngp;
    CHECK(ngp_gal.x == Approx(0.0).margin(1e-8));
    CHECK(ngp_gal.y == Approx(0.0).margin(1e-8));
    CHECK(ngp_gal.z == Approx(1.0).margin(1e-8));

    const glm::dvec3 gc_gal = R * gc;
    CHECK(gc_gal.x == Approx(1.0).margin(1e-4));
    CHECK(gc_gal.y == Approx(0.0).margin(1e-4));
    CHECK(gc_gal.z == Approx(0.0).margin(1e-4));

    const glm::dvec3 ncp_gal = R * glm::dvec3{0.0, 0.0, 1.0};
    const double ncp_lon_deg = wrap_deg(std::atan2(ncp_gal.y, ncp_gal.x) * solar::core::kRadToDeg);
    const double ncp_lat_deg = std::asin(std::clamp(ncp_gal.z, -1.0, 1.0)) * solar::core::kRadToDeg;
    CHECK(ncp_lon_deg == Approx(122.93192).margin(0.05));
    CHECK(ncp_lat_deg == Approx(27.12825).margin(0.05));
}

TEST_CASE("ecliptic_from_equatorial tilts by the mean obliquity", "[sky]") {
    const glm::dvec3 eq_north = solar::core::ecliptic_from_equatorial({0.0, 0.0, 1.0});
    CHECK(eq_north.x == Approx(0.0).margin(1e-12));
    CHECK(eq_north.y ==
          Approx(std::sin(solar::core::kMeanObliquityDeg * solar::core::kDegToRad)).margin(1e-12));
    CHECK(eq_north.z ==
          Approx(std::cos(solar::core::kMeanObliquityDeg * solar::core::kDegToRad)).margin(1e-12));

    const glm::dvec3 equinox = solar::core::ecliptic_from_equatorial({1.0, 0.0, 0.0});
    CHECK(equinox.x == Approx(1.0).margin(1e-12));
    CHECK(equinox.y == Approx(0.0).margin(1e-12));
    CHECK(equinox.z == Approx(0.0).margin(1e-12));
}

TEST_CASE("tex_from_ecliptic sends the galactic centre to the map equator", "[sky]") {
    const glm::dvec3 gc_eq = solar::core::equatorial_direction(solar::core::kGalacticCenterRaDeg,
                                                               solar::core::kGalacticCenterDecDeg);
    const glm::dvec3 gc_ecl = solar::core::ecliptic_from_equatorial(gc_eq);

    const glm::dvec3 dir0 = solar::core::tex_from_ecliptic(0.0) * gc_ecl;
    CHECK(dir0.x == Approx(1.0).margin(1e-4));
    CHECK(dir0.y == Approx(0.0).margin(1e-4));
    CHECK(dir0.z == Approx(0.0).margin(1e-4));
    CHECK(solar::core::equirectangular_uv(dir0).y == Approx(0.5).margin(1e-3));

    // SSS 2k star maps pack the bulge near u = 1050/2048.
    constexpr double kOffsetDeg = 184.7;
    const glm::dvec3 dir = solar::core::tex_from_ecliptic(kOffsetDeg) * gc_ecl;
    const glm::dvec2 uv = solar::core::equirectangular_uv(dir);
    CHECK(uv.x == Approx(184.7 / 360.0).margin(1e-4));
    CHECK(uv.y == Approx(0.5).margin(1e-3));

    const glm::dvec3 ngp_ecl =
        solar::core::ecliptic_from_equatorial(solar::core::equatorial_direction(
            solar::core::kNorthGalacticPoleRaDeg, solar::core::kNorthGalacticPoleDecDeg));
    const glm::dvec2 pole_uv =
        solar::core::equirectangular_uv(solar::core::tex_from_ecliptic(kOffsetDeg) * ngp_ecl);
    CHECK(pole_uv.y == Approx(0.0).margin(1e-4));
}

TEST_CASE("sky.json names the star map and longitude origin", "[sky][json]") {
    const auto spec = solar::app::load_sky_config("assets/data/sky.json");
    CHECK_FALSE(spec.texture.empty());
    CHECK(spec.longitude_offset_deg == Approx(184.7f));
    CHECK(spec.brightness == Approx(2.0f));
    CHECK(spec.visible);
}

TEST_CASE("equirectangular_uv matches the globe mesh convention", "[sky]") {
    CHECK(solar::core::equirectangular_uv({1.0, 0.0, 0.0}).x == Approx(0.0).margin(1e-12));
    CHECK(solar::core::equirectangular_uv({1.0, 0.0, 0.0}).y == Approx(0.5).margin(1e-12));
    CHECK(solar::core::equirectangular_uv({0.0, 1.0, 0.0}).x == Approx(0.25).margin(1e-12));
    CHECK(solar::core::equirectangular_uv({0.0, 0.0, 1.0}).y == Approx(0.0).margin(1e-12));
    CHECK(solar::core::equirectangular_uv({0.0, 0.0, -1.0}).y == Approx(1.0).margin(1e-12));
}
