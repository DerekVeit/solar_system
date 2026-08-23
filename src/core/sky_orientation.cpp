#include "core/sky_orientation.hpp"

#include "core/constants.hpp"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace solar::core {

namespace {

glm::dmat3 rotate_x(double angle_deg) {
    return glm::rotate(glm::dmat4{1.0}, angle_deg * kDegToRad, {1, 0, 0});
}

glm::dmat3 rotate_y(double angle_deg) {
    return glm::rotate(glm::dmat4{1.0}, angle_deg * kDegToRad, {0, 1, 0});
}

glm::dmat3 rotate_z(double angle_deg) {
    return glm::rotate(glm::dmat4{1.0}, angle_deg * kDegToRad, {0, 0, 1});
}

} // namespace

glm::dvec3 equatorial_direction(double ra_deg, double dec_deg) {
    const double ra = ra_deg * kDegToRad;
    const double dec = dec_deg * kDegToRad;
    const double cos_dec = std::cos(dec);
    return {cos_dec * std::cos(ra), cos_dec * std::sin(ra), std::sin(dec)};
}

glm::dvec3 ecliptic_from_equatorial(const glm::dvec3& equatorial) {
    return rotate_x(-kMeanObliquityDeg) * equatorial;
}

glm::dmat3 galactic_from_equatorial() {
    const glm::dvec3 z =
        glm::normalize(equatorial_direction(kNorthGalacticPoleRaDeg, kNorthGalacticPoleDecDeg));
    glm::dvec3 x = equatorial_direction(kGalacticCenterRaDeg, kGalacticCenterDecDeg);
    x = glm::normalize(x - glm::dot(x, z) * z);
    const glm::dvec3 y = glm::cross(z, x);
    return glm::transpose(glm::dmat3{x, y, z});
}

glm::dvec2 equirectangular_uv(const glm::dvec3& dir) {
    const glm::dvec3 n = glm::normalize(dir);
    double u = std::atan2(n.y, n.x) / kTwoPi;
    if (u < 0.0) {
        u += 1.0;
    }
    const double lat = std::asin(std::clamp(n.z, -1.0, 1.0));
    const double v = 0.5 - lat / kPi;
    return {u, v};
}

glm::dmat3 tex_from_ecliptic(double longitude_offset_deg) {
    // Ry(180°) sends the galactic centre to −X (u = 0.5) and the NGP to −Z
    // (v = 1), matching the SSS JPEG packing. The Milky Way stays on the
    // texture equator either way, which is why a pole/longitude flip is easy
    // to miss if you only check the band.
    return rotate_z(longitude_offset_deg) * rotate_y(180.0) * galactic_from_equatorial() *
           rotate_x(kMeanObliquityDeg);
}

glm::dvec3 ecliptic_direction(double ra_deg, double dec_deg) {
    return ecliptic_from_equatorial(equatorial_direction(ra_deg, dec_deg));
}

std::vector<glm::dvec3> directional_circle(const glm::dvec3& dir, double distance,
                                           double angular_radius_rad, std::size_t sample_count) {
    if (distance <= 0.0 || angular_radius_rad <= 0.0 || sample_count < 3) {
        return {};
    }
    const double dir_len2 = glm::dot(dir, dir);
    if (dir_len2 <= 0.0) {
        return {};
    }

    const glm::dvec3 n = dir / std::sqrt(dir_len2);
    const glm::dvec3 helper =
        (std::abs(n.z) < 0.9) ? glm::dvec3{0.0, 0.0, 1.0} : glm::dvec3{1.0, 0.0, 0.0};
    const glm::dvec3 e1 = glm::normalize(glm::cross(helper, n));
    const glm::dvec3 e2 = glm::cross(n, e1);
    const double cos_a = std::cos(angular_radius_rad);
    const double sin_a = std::sin(angular_radius_rad);

    std::vector<glm::dvec3> points;
    points.reserve(sample_count);
    for (std::size_t i = 0; i < sample_count; ++i) {
        const double theta = kTwoPi * static_cast<double>(i) / static_cast<double>(sample_count);
        const glm::dvec3 spoke = std::cos(theta) * e1 + std::sin(theta) * e2;
        points.push_back(distance * (n * cos_a + spoke * sin_a));
    }
    return points;
}

} // namespace solar::core
