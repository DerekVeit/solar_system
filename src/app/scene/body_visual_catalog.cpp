#include "app/scene/body_visual_catalog.hpp"

#include "app/scene/body_visual_loader.hpp"

namespace solar::app {

namespace {

[[nodiscard]] const BodyVisualOverrideEntry* find_override(const BodyVisualConfig& config,
                                                           const std::string& name) {
    for (const BodyVisualOverrideEntry& override_entry : config.overrides) {
        if (override_entry.name == name) {
            return &override_entry;
        }
    }
    return nullptr;
}

} // namespace

BodyVisualSettings resolve_body_visual_settings(const BodyVisualDefaults& defaults,
                                                const BodyVisualOverrideEntry* override) {
    if (override == nullptr) {
        return BodyVisualSettings{defaults.color, defaults.tail_duration_days,
                                  defaults.display_size_factor};
    }

    return BodyVisualSettings{
        override->color.value_or(defaults.color),
        override->tail_duration_days.value_or(defaults.tail_duration_days),
        override->display_size_factor.value_or(defaults.display_size_factor),
    };
}

BodyVisual make_body_visual(const core::BodyDefinition& body, const BodyVisualSettings& settings) {
    return BodyVisual{body.name, settings.color, body.radius_km, settings.tail_duration_days,
                      settings.display_size_factor};
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const BodyVisualConfig& config) {
    for (const core::BodyDefinition& body : catalog) {
        const BodyVisualSettings settings =
            resolve_body_visual_settings(config.defaults, find_override(config, body.name));
        scene.add_body(make_body_visual(body, settings));
    }
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const std::filesystem::path& visual_config_path) {
    populate_scene(scene, catalog, load_body_visual_config(visual_config_path));
}

} // namespace solar::app