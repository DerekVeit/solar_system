#include "app/scene/body_visual_catalog.hpp"

#include "app/logging.hpp"
#include "app/scene/body_visual_loader.hpp"

namespace solar::app {

namespace {

[[nodiscard]] bool catalog_contains(const std::vector<core::BodyDefinition>& catalog,
                                    const std::string& name) {
    for (const core::BodyDefinition& body : catalog) {
        if (body.name == name) {
            return true;
        }
    }
    return false;
}

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
        return BodyVisualSettings{defaults.color,
                                  defaults.ambient,
                                  defaults.emission,
                                  defaults.tail_duration_days,
                                  defaults.display_size_factor,
                                  defaults.visible};
    }

    return BodyVisualSettings{
        override->color.value_or(defaults.color),
        override->ambient.value_or(defaults.ambient),
        override->emission.value_or(defaults.emission),
        override->tail_duration_days.value_or(defaults.tail_duration_days),
        override->display_size_factor.value_or(defaults.display_size_factor),
        override->visible.value_or(defaults.visible),
    };
}

BodyVisual make_body_visual(const core::BodyDefinition& body, const BodyVisualSettings& settings) {
    const bool draws_orbit_trails = body.elements.semi_major_axis_km > 0.0;
    return BodyVisual{body.name,
                      settings.color,
                      settings.ambient,
                      settings.emission,
                      body.radius_km,
                      settings.tail_duration_days,
                      settings.display_size_factor,
                      draws_orbit_trails};
}

void warn_about_visual_config_mismatches(const std::vector<core::BodyDefinition>& catalog,
                                         const BodyVisualConfig& config) {
    for (const BodyVisualOverrideEntry& override_entry : config.overrides) {
        if (!catalog_contains(catalog, override_entry.name)) {
            log("body visual override has no matching catalog entry: {}", override_entry.name);
        }
    }

    for (const core::BodyDefinition& body : catalog) {
        if (find_override(config, body.name) == nullptr) {
            log("catalog body has no visual override; using defaults: {}", body.name);
        }
    }
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const BodyVisualConfig& config) {
    warn_about_visual_config_mismatches(catalog, config);

    for (const core::BodyDefinition& body : catalog) {
        const BodyVisualSettings settings =
            resolve_body_visual_settings(config.defaults, find_override(config, body.name));
        if (!settings.visible) {
            continue;
        }
        scene.add_body(make_body_visual(body, settings));
    }
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const std::filesystem::path& visual_config_path) {
    populate_scene(scene, catalog, load_body_visual_config(visual_config_path));
}

} // namespace solar::app