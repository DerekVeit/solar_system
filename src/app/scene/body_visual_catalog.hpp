#pragma once

#include "app/scene/body_visual_config.hpp"
#include "app/scene/scene.hpp"
#include "core/ephemeris.hpp"

#include <filesystem>

namespace solar::app {

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const BodyVisualConfig& config);

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const std::filesystem::path& visual_config_path);

} // namespace solar::app
