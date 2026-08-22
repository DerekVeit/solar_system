#pragma once

#include <string>

namespace solar::app {

/// Presentation settings for the directional star background.
struct SkySpec {
    std::string texture{};
    /// Rotation about galactic +Z, degrees, so texture u=0 matches the packed meridian.
    float longitude_offset_deg{184.7f};
    float brightness{2.0f};
    bool visible{true};
};

} // namespace solar::app
