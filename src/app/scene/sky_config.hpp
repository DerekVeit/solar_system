#pragma once

#include <string>

namespace solar::app {

/// Presentation settings for the directional star background.
struct SkySpec {
    std::string texture{};
    /// Extra rotation about the map +Z after the SSS galactic packing (degrees).
    float longitude_offset_deg{0.0f};
    float brightness{2.0f};
    bool visible{true};
};

} // namespace solar::app
