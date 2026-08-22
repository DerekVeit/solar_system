#pragma once

#include "app/scene/sky_config.hpp"

#include <filesystem>

namespace solar::app {

[[nodiscard]] SkySpec load_sky_config(const std::filesystem::path& path);

} // namespace solar::app
