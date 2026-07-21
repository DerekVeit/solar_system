#include "app/scene/body_visual_loader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace solar::app {

namespace {

[[nodiscard]] std::string path_message(const std::string& context, const std::string& detail) {
    return context + ": " + detail;
}

Color parse_color(const nlohmann::json& json, const std::string& context) {
    if (!json.is_array()) {
        throw std::runtime_error(path_message(context, "color must be an array [r, g, b, a]"));
    }
    const auto components = json.get<std::vector<float>>();
    if (components.size() != 4) {
        throw std::runtime_error(
            path_message(context, "color must have four components [r, g, b, a]"));
    }
    return Color{components[0], components[1], components[2], components[3]};
}

void require_object(const nlohmann::json& json, const std::string& context) {
    if (!json.is_object()) {
        throw std::runtime_error(path_message(context, "expected a JSON object"));
    }
}

void apply_surface_partial(const nlohmann::json& json, BodySurface& surface,
                           const std::string& context) {
    require_object(json, context);
    if (json.contains("color")) {
        surface.color = parse_color(json.at("color"), context + ".color");
    }
    if (json.contains("ambient")) {
        surface.ambient = json.at("ambient").get<float>();
    }
    if (json.contains("emission")) {
        surface.emission = json.at("emission").get<float>();
    }
}

BodySurface parse_surface_required(const nlohmann::json& json, const std::string& context) {
    require_object(json, context);
    for (const char* key : {"color", "ambient", "emission"}) {
        if (!json.contains(key)) {
            throw std::runtime_error(path_message(context, std::string(key) + " is required"));
        }
    }
    BodySurface surface{};
    apply_surface_partial(json, surface, context);
    return surface;
}

void apply_spec_partial(const nlohmann::json& json, BodyVisualSpec& spec,
                        const std::string& context) {
    require_object(json, context);
    if (json.contains("surface")) {
        apply_surface_partial(json.at("surface"), spec.surface, context + ".surface");
    }
    if (json.contains("tail_duration_days")) {
        spec.tail_duration_days = json.at("tail_duration_days").get<double>();
    }
    if (json.contains("display_size_factor")) {
        spec.display_size_factor = json.at("display_size_factor").get<float>();
    }
    if (json.contains("visible")) {
        spec.visible = json.at("visible").get<bool>();
    }
}

BodyVisualSpec parse_spec_required(const nlohmann::json& json, const std::string& context) {
    require_object(json, context);
    for (const char* key : {"surface", "tail_duration_days", "display_size_factor", "visible"}) {
        if (!json.contains(key)) {
            throw std::runtime_error(path_message(context, std::string(key) + " is required"));
        }
    }
    BodyVisualSpec spec{};
    spec.surface = parse_surface_required(json.at("surface"), context + ".surface");
    spec.tail_duration_days = json.at("tail_duration_days").get<double>();
    spec.display_size_factor = json.at("display_size_factor").get<float>();
    spec.visible = json.at("visible").get<bool>();
    return spec;
}

} // namespace

BodyVisualConfig load_body_visual_config(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open body visual config: " + path.string());
    }

    const nlohmann::json root = nlohmann::json::parse(input);
    if (!root.is_object()) {
        throw std::runtime_error(path.string() + ": root must be a JSON object");
    }
    if (!root.contains("defaults")) {
        throw std::runtime_error(path.string() + ": defaults is required");
    }
    if (!root.contains("bodies")) {
        throw std::runtime_error(path.string() + ": bodies is required");
    }
    if (!root.at("bodies").is_array()) {
        throw std::runtime_error(path.string() + ": bodies must be an array");
    }

    BodyVisualConfig config{};
    config.defaults = parse_spec_required(root.at("defaults"), path.string() + ": defaults");

    const auto& bodies = root.at("bodies");
    for (std::size_t i = 0; i < bodies.size(); ++i) {
        const std::string entry_context = path.string() + ": bodies[" + std::to_string(i) + "]";
        const nlohmann::json& entry = bodies.at(i);
        require_object(entry, entry_context);
        if (!entry.contains("name")) {
            throw std::runtime_error(path_message(entry_context, "name is required"));
        }

        const std::string name = entry.at("name").get<std::string>();
        if (name.empty()) {
            throw std::runtime_error(path_message(entry_context, "name must be non-empty"));
        }
        if (config.by_name.contains(name)) {
            throw std::runtime_error(
                path_message(entry_context, "duplicate body entry \"" + name + "\""));
        }

        BodyVisualSpec spec = config.defaults;
        apply_spec_partial(entry, spec, entry_context);
        config.by_name.emplace(name, std::move(spec));
    }

    return config;
}

} // namespace solar::app
