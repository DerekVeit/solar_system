#pragma once

#include <filesystem>
#include <vector>

#include "core/ephemeris.hpp"

namespace solar::core {

[[nodiscard]] std::vector<BodyDefinition> load_bodies(const std::filesystem::path& path);

}  // namespace solar::core