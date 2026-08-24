#include "app/scene/star_catalog_loader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
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
        if (!entry.contains("ra_deg") || !entry.contains("dec_deg")) {
            throw std::runtime_error(path_message(context, "ra_deg and dec_deg are required"));
        }

        Star star{};
        if (entry.contains("hip")) {
            star.hip = entry.at("hip").get<int>();
            if (star.hip <= 0) {
                throw std::runtime_error(path_message(context, "hip must be positive"));
            }
        }
        if (entry.contains("name")) {
            star.name = entry.at("name").get<std::string>();
        }
        if (star.hip == 0 && star.name.empty()) {
            throw std::runtime_error(path_message(context, "hip or a non-empty name is required"));
        }
        star.ra_deg = entry.at("ra_deg").get<double>();
        star.dec_deg = entry.at("dec_deg").get<double>();
        if (entry.contains("marker_radius_deg")) {
            star.marker_radius_deg = entry.at("marker_radius_deg").get<float>();
        }
        catalog.stars.push_back(std::move(star));
    }

    std::unordered_set<int> hips;
    hips.reserve(catalog.stars.size());
    for (std::size_t i = 0; i < catalog.stars.size(); ++i) {
        const int hip = catalog.stars[i].hip;
        if (hip == 0) {
            continue;
        }
        if (!hips.insert(hip).second) {
            throw std::runtime_error(path.string() + ": duplicate hip " + std::to_string(hip) +
                                     " at stars[" + std::to_string(i) + "]");
        }
    }
    return catalog;
}

} // namespace solar::app
