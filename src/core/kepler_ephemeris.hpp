#pragma once

#include "core/ephemeris.hpp"

namespace solar::core {

class KeplerEphemeris final : public EphemerisProvider {
  public:
    explicit KeplerEphemeris(std::vector<BodyDefinition> bodies);

    [[nodiscard]] StateVector state(const std::string& body_name, Epoch epoch) const override;
    [[nodiscard]] const std::vector<BodyDefinition>& bodies() const override { return bodies_; }

  private:
    std::vector<BodyDefinition> bodies_;
};

}  // namespace solar::core