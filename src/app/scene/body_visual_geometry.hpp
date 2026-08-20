#pragma once

#include "core/ephemeris.hpp"
#include "core/types.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace solar::app {

/// Stretch applied to a satellite's primary→body vector when drawing.
/// Planets (no primary) and unscaled views always use 1.
[[nodiscard]] constexpr double orbit_display_factor(bool body_scaling, bool has_primary,
                                                    double size_factor) {
    return (body_scaling && has_primary) ? size_factor : 1.0;
}

/// `primary + orbit_factor * relative` in km.
[[nodiscard]] core::Displacement offset_from_primary(const core::Displacement& primary,
                                                     const core::Displacement& relative,
                                                     double orbit_factor);

/// Heliocentric draw position: the body's Kepler offset from its primary, scaled
/// by `orbit_factor`. Pass 1 for a 1:1 layout.
[[nodiscard]] core::Displacement drawn_position(const core::EphemerisProvider& ephemeris,
                                                const std::string& body_name, core::Epoch epoch,
                                                double orbit_factor);

/// Closed orbit around the primary frozen at `epoch_now`. Relative Kepler is
/// sampled over one period so a satellite ellipse stays closed while the
/// primary moves. Empty when the body has no orbital period.
[[nodiscard]] std::vector<core::Displacement>
orbit_loop_positions(const core::EphemerisProvider& ephemeris, const core::BodyDefinition& body,
                     core::Epoch epoch_now, double orbit_factor, std::size_t sample_count);

/// Past heliocentric path, sampled uniformly in time (not mean anomaly).
/// Primary and relative are both evaluated at each sample epoch. Empty when
/// `tail_duration_seconds` is not positive or `sample_count` is fewer than 2.
[[nodiscard]] std::vector<core::Displacement>
tail_positions(const core::EphemerisProvider& ephemeris, const core::BodyDefinition& body,
               core::Epoch epoch_now, double orbit_factor, double tail_duration_seconds,
               std::size_t sample_count);

} // namespace solar::app
