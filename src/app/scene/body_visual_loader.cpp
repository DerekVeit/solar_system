#include "app/scene/body_visual_loader.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace solar::app {

namespace {

Color parse_color(const nlohmann::json& json) {
    const auto components = json.get<std::vector<float>>();
    if (components.size() != 4) {
        throw std::runtime_error("color must have four components [r, g, b, a]");
    }
    return Color{components[0], components[1], components[2], components[3]};
}

BodyVisualDefaults parse_defaults(const nlohmann::json& json) {
    BodyVisualDefaults defaults;
    if (json.contains("color")) {
        defaults.color = parse_color(json.at("color"));
    }
    if (json.contains("tail_duration_days")) {
        defaults.tail_duration_days = json.at("tail_duration_days").get<double>();
    }
    if (json.contains("display_size_factor")) {
        defaults.display_size_factor = json.at("display_size_factor").get<float>();
    }
    if (json.contains("visible")) {
        defaults.visible = json.at("visible").get<bool>();
    }
    return defaults;
}

BodyVisualOverrideEntry parse_override(const nlohmann::json& json) {
    BodyVisualOverrideEntry override_entry{
        .name = json.at("name").get<std::string>(),
    };
    if (json.contains("color")) {
        override_entry.color = parse_color(json.at("color"));
    }
    if (json.contains("tail_duration_days")) {
        override_entry.tail_duration_days = json.at("tail_duration_days").get<double>();
    }
    if (json.contains("display_size_factor")) {
        override_entry.display_size_factor = json.at("display_size_factor").get<float>();
    }
    if (json.contains("visible")) {
        override_entry.visible = json.at("visible").get<bool>();
    }
    return override_entry;
}

} // namespace

BodyVisualConfig load_body_visual_config(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open body visual config: " + path.string());
    }

    const nlohmann::json root = nlohmann::json::parse(input);
    BodyVisualConfig config{
        .defaults = parse_defaults(root.at("defaults")),
    };

    for (const auto& entry : root.at("bodies")) {
        config.overrides.push_back(parse_override(entry));
    }

    return config;
}

} // namespace solar::app
