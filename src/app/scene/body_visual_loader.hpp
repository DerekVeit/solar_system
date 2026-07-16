#pragma once

#include "app/scene/body_visual_config.hpp"

#include <filesystem>

namespace solar::app {

[[nodiscard]] BodyVisualConfig load_body_visual_config(const std::filesystem::path& path);

} // namespace solar::app
