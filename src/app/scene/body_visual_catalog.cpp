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

void warn_about_visual_config_mismatches(const std::vector<core::BodyDefinition>& catalog,
                                         const BodyVisualConfig& config) {
    for (const auto& [name, spec] : config.by_name) {
        (void)spec;
        if (!catalog_contains(catalog, name)) {
            log("body visual entry has no matching catalog body: {}", name);
        }
    }

    for (const core::BodyDefinition& body : catalog) {
        if (!config.by_name.contains(body.name)) {
            log("catalog body has no visual entry; using defaults: {}", body.name);
        }
    }
}

} // namespace

BodyVisual make_body_visual(const core::BodyDefinition& body, const BodyVisualSpec& spec) {
    const bool draws_orbit_trails = body.elements.semi_major_axis_km > 0.0;
    return BodyVisual{body.name, spec, body.radius_km, draws_orbit_trails};
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const BodyVisualConfig& config) {
    warn_about_visual_config_mismatches(catalog, config);

    for (const core::BodyDefinition& body : catalog) {
        const BodyVisualSpec* spec = nullptr;
        const auto it = config.by_name.find(body.name);
        if (it != config.by_name.end()) {
            spec = &it->second;
        } else {
            spec = &config.defaults;
        }
        if (!spec->visible) {
            continue;
        }
        scene.add_body(make_body_visual(body, *spec));
    }
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog,
                    const std::filesystem::path& visual_config_path) {
    populate_scene(scene, catalog, load_body_visual_config(visual_config_path));
}

} // namespace solar::app
