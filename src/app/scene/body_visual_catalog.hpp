#pragma once

#include "app/scene/body_visual.hpp"
#include "app/scene/body_visual_config.hpp"
#include "app/scene/scene.hpp"
#include "core/ephemeris.hpp"

#include <filesystem>

namespace solar::app {

[[nodiscard]] BodyVisualSettings
resolve_body_visual_settings(const BodyVisualDefaults& defaults,
                             const BodyVisualOverrideEntry* override);

[[nodiscard]] BodyVisual make_body_visual(const core::BodyDefinition& body,
                                          const BodyVisualSettings& settings);

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const BodyVisualConfig& config);

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const std::filesystem::path& visual_config_path);

} // namespace solar::app