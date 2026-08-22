#pragma once

#include <glm/ext/matrix_double3x3.hpp>
#include <glm/ext/vector_double2.hpp>
#include <glm/ext/vector_double3.hpp>

#include <cstddef>
#include <vector>

namespace solar::core {

/// IAU J2000 north galactic pole and galactic centre, equatorial degrees.
inline constexpr double kNorthGalacticPoleRaDeg = 192.85948;
inline constexpr double kNorthGalacticPoleDecDeg = 27.12825;
inline constexpr double kGalacticCenterRaDeg = 266.4051;
inline constexpr double kGalacticCenterDecDeg = -28.936175;

/// Mean obliquity of the ecliptic used by the draw frame (matches body orientation).
inline constexpr double kMeanObliquityDeg = 23.44;

/// Unit vector at right ascension / declination (ICRF / equatorial J2000).
[[nodiscard]] glm::dvec3 equatorial_direction(double ra_deg, double dec_deg);

/// Ecliptic J2000 from equatorial J2000: Rx(−ε).
[[nodiscard]] glm::dvec3 ecliptic_from_equatorial(const glm::dvec3& equatorial);

/// Galactic Cartesian from equatorial J2000. +X is the galactic centre, +Z the NGP.
[[nodiscard]] glm::dmat3 galactic_from_equatorial();

/// Equirectangular UV in [0, 1)×[0, 1] for a unit direction in the texture frame.
/// u = 0 on +X, increasing toward +Y; v = 0 at +Z (same as the globe mesh).
[[nodiscard]] glm::dvec2 equirectangular_uv(const glm::dvec3& dir);

/// Texture-frame from ecliptic J2000. `longitude_offset_deg` is a rotation about
/// galactic +Z so the map's u = 0 matches the packed prime meridian.
[[nodiscard]] glm::dmat3 tex_from_ecliptic(double longitude_offset_deg);

/// Ecliptic J2000 unit vector at ICRS right ascension / declination.
[[nodiscard]] glm::dvec3 ecliptic_direction(double ra_deg, double dec_deg);

/// Eye-relative ring of `sample_count` points at angular radius `angular_radius_rad`
/// around `dir`, on the sphere of radius `distance`. Empty if the inputs are degenerate.
[[nodiscard]] std::vector<glm::dvec3> directional_circle(const glm::dvec3& dir, double distance,
                                                         double angular_radius_rad,
                                                         std::size_t sample_count);

} // namespace solar::core
