#include "core/sky_orientation.hpp"

#include "core/constants.hpp"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace solar::core {

namespace {

glm::dmat3 rotate_x(double angle_deg) {
    return glm::rotate(glm::dmat4{1.0}, angle_deg * kDegToRad, {1, 0, 0});
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
    return rotate_z(longitude_offset_deg) * galactic_from_equatorial() *
           rotate_x(kMeanObliquityDeg);
}

} // namespace solar::core
