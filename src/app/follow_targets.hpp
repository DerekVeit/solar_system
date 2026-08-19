#pragma once

#include <array>
#include <string_view>

namespace solar::app {

/// Follow targets keyed by main-keyboard digits 0-9 (Sun through Pluto).
/// M / , / . walk satellites and siblings from the current follow target.
inline constexpr std::array<std::string_view, 10> kFollowTargets = {
    "Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto",
};

} // namespace solar::app