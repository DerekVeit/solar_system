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
    KeplerianElements elements{};
};

class EphemerisProvider {
  public:
    virtual ~EphemerisProvider() = default;

    [[nodiscard]] virtual StateVector state(const std::string& body_name, Epoch epoch) const = 0;
    [[nodiscard]] virtual const std::vector<BodyDefinition>& bodies() const = 0;
};

using EphemerisProviderPtr = std::unique_ptr<EphemerisProvider>;

} // namespace solar::core