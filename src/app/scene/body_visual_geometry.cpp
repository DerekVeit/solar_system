#include "app/scene/body_visual_geometry.hpp"

#include "core/constants.hpp"
#include "core/kepler.hpp"

#include <stdexcept>

namespace solar::app {

core::Displacement offset_from_primary(const core::Displacement& primary,
                                       const core::Displacement& relative, double orbit_factor) {
    return {primary.km + orbit_factor * relative.km};
}

core::Displacement drawn_position(const core::EphemerisProvider& ephemeris,
                                  const std::string& body_name, core::Epoch epoch,
                                  double orbit_factor) {
    const core::BodyDefinition* body = core::find_body(ephemeris, body_name);
    if (body == nullptr) {
        throw std::invalid_argument("unknown body: " + body_name);
    }
    const core::Displacement relative = ephemeris.relative_state(body_name, epoch).position;
    const core::Displacement primary = body->primary.empty()
                                           ? core::Displacement{}
                                           : ephemeris.state(body->primary, epoch).position;
    return offset_from_primary(primary, relative, orbit_factor);
}

std::vector<core::Displacement> orbit_loop_positions(const core::EphemerisProvider& ephemeris,
                                                     const core::BodyDefinition& body,
                                                     core::Epoch epoch_now, double orbit_factor,
                                                     std::size_t sample_count) {
    const double mu = core::orbital_mu(ephemeris, body);
    const double period_seconds = core::orbital_period_seconds(mu, body.elements);
    if (period_seconds <= 0.0 || sample_count == 0) {
        return {};
    }

    const core::Displacement primary_now = body.primary.empty()
                                               ? core::Displacement{}
                                               : ephemeris.state(body.primary, epoch_now).position;

    std::vector<core::Displacement> positions;
    positions.reserve(sample_count);
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(sample_count);
        const double elapsed_seconds = period_seconds * fraction;
        const core::Epoch sample_epoch{epoch_now.jd -
                                       (period_seconds - elapsed_seconds) / core::kSecondsPerDay};
        const core::Displacement relative =
            ephemeris.relative_state(body.name, sample_epoch).position;
        positions.push_back(offset_from_primary(primary_now, relative, orbit_factor));
    }
    return positions;
}

std::vector<core::Displacement> tail_positions(const core::EphemerisProvider& ephemeris,
                                               const core::BodyDefinition& body,
                                               core::Epoch epoch_now, double orbit_factor,
                                               double tail_duration_seconds,
                                               std::size_t sample_count) {
    if (tail_duration_seconds <= 0.0 || sample_count < 2) {
        return {};
    }

    // Uniform in time. Mean-anomaly stepping wraps to one period, so a tail
    // longer than one revolution would fold onto the last orbit.
    std::vector<core::Displacement> positions;
    positions.reserve(sample_count);
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        const double fraction = static_cast<double>(sample) / static_cast<double>(sample_count - 1);
        const core::Epoch sample_epoch{epoch_now.jd -
                                       tail_duration_seconds * fraction / core::kSecondsPerDay};
        const core::Displacement relative =
            ephemeris.relative_state(body.name, sample_epoch).position;
        const core::Displacement primary =
            body.primary.empty() ? core::Displacement{}
                                 : ephemeris.state(body.primary, sample_epoch).position;
        positions.push_back(offset_from_primary(primary, relative, orbit_factor));
    }
    return positions;
}

} // namespace solar::app
