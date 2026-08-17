#pragma once

#include "core/ephemeris.hpp"

namespace solar::core {

class KeplerEphemeris final : public EphemerisProvider {
  public:
    explicit KeplerEphemeris(std::vector<BodyDefinition> bodies);

    [[nodiscard]] StateVector state(const std::string& body_name, Epoch epoch) const override;
    [[nodiscard]] StateVector relative_state(const std::string& body_name,
                                             Epoch epoch) const override;
    [[nodiscard]] double rotation_deg(const std::string& body_name, Epoch epoch) const override;
    [[nodiscard]] const std::vector<BodyDefinition>& bodies() const override { return bodies_; }

  private:
    [[nodiscard]] StateVector kepler_relative(const BodyDefinition& body, Epoch epoch) const;

    std::vector<BodyDefinition> bodies_;
};

} // namespace solar::core