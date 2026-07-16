#include "app/scene/body_visual_catalog.hpp"

namespace solar::app {

namespace {

constexpr BodyVisualOverride kBodyVisualOverrides[] = {
    {.name = "Sun",
     .color = Color{1.0f, 0.9f, 0.3f, 1.0f},
     .tail_duration_days = 0.0,
     .display_size_factor = 5.0f},
    {.name = "Earth", .color = Color{0.45f, 0.75f, 1.0f, 1.0f}, .tail_duration_days = 45.0},
    {.name = "Venus", .color = Color{1.0f, 0.85f, 0.5f, 1.0f}},
    {.name = "Mars", .color = Color{1.0f, 0.4f, 0.3f, 1.0f}, .tail_duration_days = 60.0},
    {.name = "Jupiter", .color = Color{0.9f, 0.7f, 0.5f, 1.0f}, .tail_duration_days = 120.0},
};

[[nodiscard]] const BodyVisualOverride* find_override(std::string_view name) {
    for (const BodyVisualOverride& override_entry : kBodyVisualOverrides) {
        if (override_entry.name == name) {
            return &override_entry;
        }
    }
    return nullptr;
}

} // namespace

BodyVisualDefaults default_body_visual_defaults() { return BodyVisualDefaults{}; }

BodyVisualSettings resolve_body_visual_settings(const BodyVisualDefaults& defaults,
                                                const BodyVisualOverride* override) {
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

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog) {
    const BodyVisualDefaults defaults = default_body_visual_defaults();

    for (const core::BodyDefinition& body : catalog) {
        const BodyVisualSettings settings =
            resolve_body_visual_settings(defaults, find_override(body.name));
        scene.add_body(make_body_visual(body, settings));
    }
}

} // namespace solar::app
