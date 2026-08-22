#include "app/scene/star_catalog_loader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

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

} // namespace

StarCatalog load_star_catalog(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open " + path.string());
    }

    const nlohmann::json root = nlohmann::json::parse(input);
    if (!root.is_object()) {
        throw std::runtime_error(path.string() + ": root must be a JSON object");
    }
    if (!root.contains("stars") || !root.at("stars").is_array()) {
        throw std::runtime_error(path.string() + ": stars must be an array");
    }

    StarCatalog catalog{};
    if (root.contains("visible")) {
        catalog.visible = root.at("visible").get<bool>();
    }
    if (root.contains("marker_radius_deg")) {
        catalog.marker_radius_deg = root.at("marker_radius_deg").get<float>();
    }
    if (root.contains("color")) {
        catalog.color = parse_color(root.at("color"), path.string() + ": color");
    }

    const auto& stars = root.at("stars");
    for (std::size_t i = 0; i < stars.size(); ++i) {
        const std::string context = path.string() + ": stars[" + std::to_string(i) + "]";
        const nlohmann::json& entry = stars.at(i);
        if (!entry.is_object()) {
            throw std::runtime_error(path_message(context, "expected a JSON object"));
        }
        if (!entry.contains("name") || !entry.contains("ra_deg") || !entry.contains("dec_deg")) {
            throw std::runtime_error(
                path_message(context, "name, ra_deg, and dec_deg are required"));
        }

        Star star{};
        star.name = entry.at("name").get<std::string>();
        if (star.name.empty()) {
            throw std::runtime_error(path_message(context, "name must be non-empty"));
        }
        star.ra_deg = entry.at("ra_deg").get<double>();
        star.dec_deg = entry.at("dec_deg").get<double>();
        if (entry.contains("marker_radius_deg")) {
            star.marker_radius_deg = entry.at("marker_radius_deg").get<float>();
        }
        catalog.stars.push_back(std::move(star));
    }
    return catalog;
}

} // namespace solar::app
