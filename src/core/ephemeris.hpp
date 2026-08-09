#pragma once

#include "core/types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace solar::core {

struct BodyDefinition {
    std::string name{};
    double gravitational_parameter_km3_s2{};
    double radius_km{};
    /// Temporary ecliptic tilt used by body_orientation_matrix until pole-based R is implemented.
    float obliquity_deg{};
    BodyPole pole{};
    BodyRotation rotation{};
    KeplerianElements elements{};
};

class EphemerisProvider {
  public:
    virtual ~EphemerisProvider() = default;

    [[nodiscard]] virtual StateVector state(const std::string& body_name, Epoch epoch) const = 0;
    [[nodiscard]] virtual double rotation_deg(const std::string& body_name, Epoch epoch) const = 0;
    [[nodiscard]] virtual const std::vector<BodyDefinition>& bodies() const = 0;
};

using EphemerisProviderPtr = std::unique_ptr<EphemerisProvider>;

[[nodiscard]] const BodyDefinition* find_body(const EphemerisProvider& ephemeris,
                                              const std::string& name);

[[nodiscard]] double central_gravitational_parameter(const EphemerisProvider& ephemeris);

} // namespace solar::core