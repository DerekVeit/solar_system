#pragma once

#include "core/types.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace solar::core {

struct BodyDefinition {
    std::string name{};
    /// Body this one orbits. Empty means the Sun (heliocentric Kepler).
    std::string primary{};
    double gravitational_parameter_km3_s2{};
    double radius_km{};
    /// Temporary ecliptic tilt used by body_orientation_matrix until pole-based R is implemented.
    float obliquity_deg{};
    BodyPole pole{};
    BodyRotation rotation{};
    /// If true, Ẇ is the Keplerian mean motion of this orbit (same μ and a), so
    /// the prime meridian stays locked to the primary. IAU W0 is still used.
    bool tidally_locked{false};
    KeplerianElements elements{};
};

class EphemerisProvider {
  public:
    virtual ~EphemerisProvider() = default;

    /// Heliocentric (draw-frame) state: Kepler relative to the primary, plus the
    /// primary's own state. The Sun (or any body with a <= 0) is at the origin.
    [[nodiscard]] virtual StateVector state(const std::string& body_name, Epoch epoch) const = 0;
    /// Kepler state relative to the primary (Sun if primary is empty).
    [[nodiscard]] virtual StateVector relative_state(const std::string& body_name,
                                                     Epoch epoch) const = 0;
    [[nodiscard]] virtual double rotation_deg(const std::string& body_name, Epoch epoch) const = 0;
    [[nodiscard]] virtual const std::vector<BodyDefinition>& bodies() const = 0;
};

using EphemerisProviderPtr = std::unique_ptr<EphemerisProvider>;

[[nodiscard]] const BodyDefinition* find_body(const EphemerisProvider& ephemeris,
                                              const std::string& name);

[[nodiscard]] double central_gravitational_parameter(const EphemerisProvider& ephemeris);

/// The body this one orbits: named primary, or the Sun if primary is empty.
[[nodiscard]] const BodyDefinition* primary_body(const EphemerisProvider& ephemeris,
                                                 const BodyDefinition& body);

/// Gravitational parameter of the attracting center for this body's Kepler orbit.
[[nodiscard]] double orbital_mu(const EphemerisProvider& ephemeris, const BodyDefinition& body);

/// Bodies that orbit `primary_name`, sorted by semi-major axis.
/// Empty or "Sun" selects heliocentric bodies (empty primary), including the Sun.
[[nodiscard]] std::vector<const BodyDefinition*> bodies_orbiting(const EphemerisProvider& ephemeris,
                                                                 std::string_view primary_name);

/// Innermost satellite of `body` (smallest a > 0 that orbits it). Null if none.
[[nodiscard]] const BodyDefinition* innermost_satellite(const EphemerisProvider& ephemeris,
                                                        const BodyDefinition& body);

/// Sibling in the same orbital family, wrapping. Offset +1 is the next-farther body.
[[nodiscard]] const BodyDefinition* sibling_by_offset(const EphemerisProvider& ephemeris,
                                                      const BodyDefinition& body, int offset);

} // namespace solar::core