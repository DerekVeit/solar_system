#pragma once

#include "core/ephemeris.hpp"

#include <filesystem>
#include <vector>

namespace solar::core {

[[nodiscard]] std::vector<BodyDefinition> load_bodies(const std::filesystem::path& path);

} // namespace solar::core