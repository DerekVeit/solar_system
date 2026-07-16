#include "app/scene/body_visual_catalog.hpp"

namespace solar::app {

namespace {

[[nodiscard]] double body_radius_km(const std::vector<core::BodyDefinition>& catalog,
                                    std::string_view name) {
    for (const core::BodyDefinition& body : catalog) {
        if (body.name == name) {
            return body.radius_km;
        }
    }
    return 0.0;
}

constexpr BodyVisualSpec kBodyVisualSpecs[] = {
    {.name = "Sun",
     .color = {1.0f, 0.9f, 0.3f, 1.0f},
     .tail_duration_days = 0.0,
     .display_size_factor = 5.0f},
    {.name = "Earth", .color = {0.45f, 0.75f, 1.0f, 1.0f}, .tail_duration_days = 45.0},
    {.name = "Venus", .color = {1.0f, 0.85f, 0.5f, 1.0f}, .tail_duration_days = 30.0},
    {.name = "Mars", .color = {1.0f, 0.4f, 0.3f, 1.0f}, .tail_duration_days = 60.0},
    {.name = "Jupiter", .color = {0.9f, 0.7f, 0.5f, 1.0f}, .tail_duration_days = 120.0},
};

} // namespace

std::span<const BodyVisualSpec> default_body_visual_specs() { return kBodyVisualSpecs; }

BodyVisual make_body_visual(const BodyVisualSpec& spec,
                            const std::vector<core::BodyDefinition>& catalog) {
    return BodyVisual{std::string{spec.name}, spec.color, body_radius_km(catalog, spec.name),
                      spec.tail_duration_days, spec.display_size_factor};
}

void populate_scene(Scene& scene, const std::vector<core::BodyDefinition>& catalog) {
    for (const BodyVisualSpec& spec : default_body_visual_specs()) {
        scene.add_body(make_body_visual(spec, catalog));
    }
}

} // namespace solar::app
