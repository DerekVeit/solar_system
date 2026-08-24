#include "app/scene/sky_figure_loader.hpp"

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

[[nodiscard]] SkyFigureKind parse_kind(const nlohmann::json& json, const std::string& context) {
    const auto kind = json.get<std::string>();
    if (kind == "constellation") {
        return SkyFigureKind::constellation;
    }
    if (kind == "asterism") {
        return SkyFigureKind::asterism;
    }
    throw std::runtime_error(path_message(context, "kind must be constellation or asterism"));
}

void validate_hip(int hip, const StarCatalog& stars, const std::string& context) {
    if (hip <= 0) {
        throw std::runtime_error(path_message(context, "hip must be positive"));
    }
    if (star_by_hip(stars, hip) == nullptr) {
        throw std::runtime_error(path_message(context, "unknown hip " + std::to_string(hip)));
    }
}

} // namespace

SkyFigureCatalog load_sky_figures(const std::filesystem::path& path, const StarCatalog& stars) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open " + path.string());
    }

    const nlohmann::json root = nlohmann::json::parse(input);
    if (!root.is_object()) {
        throw std::runtime_error(path.string() + ": root must be a JSON object");
    }
    if (!root.contains("figures") || !root.at("figures").is_array()) {
        throw std::runtime_error(path.string() + ": figures must be an array");
    }

    SkyFigureCatalog catalog{};
    if (root.contains("visible")) {
        catalog.visible = root.at("visible").get<bool>();
    }
    if (root.contains("line_gap_deg")) {
        catalog.line_gap_deg = root.at("line_gap_deg").get<float>();
        if (catalog.line_gap_deg < 0.0f) {
            throw std::runtime_error(path.string() + ": line_gap_deg must be non-negative");
        }
    }
    if (root.contains("color")) {
        catalog.color = parse_color(root.at("color"), path.string() + ": color");
    }

    const auto& figures = root.at("figures");
    std::unordered_set<std::string> ids;
    for (std::size_t i = 0; i < figures.size(); ++i) {
        const std::string context = path.string() + ": figures[" + std::to_string(i) + "]";
        const nlohmann::json& entry = figures.at(i);
        if (!entry.is_object()) {
            throw std::runtime_error(path_message(context, "expected a JSON object"));
        }
        if (!entry.contains("id") || !entry.contains("name") || !entry.contains("kind") ||
            !entry.contains("polylines")) {
            throw std::runtime_error(
                path_message(context, "id, name, kind, and polylines are required"));
        }

        SkyFigure figure{};
        figure.id = entry.at("id").get<std::string>();
        figure.name = entry.at("name").get<std::string>();
        if (figure.id.empty() || figure.name.empty()) {
            throw std::runtime_error(path_message(context, "id and name must be non-empty"));
        }
        if (!ids.insert(figure.id).second) {
            throw std::runtime_error(path_message(context, "duplicate id " + figure.id));
        }
        figure.kind = parse_kind(entry.at("kind"), context + ": kind");
        if (entry.contains("visible")) {
            figure.visible = entry.at("visible").get<bool>();
        }

        const nlohmann::json& polylines = entry.at("polylines");
        if (!polylines.is_array() || polylines.empty()) {
            throw std::runtime_error(path_message(context, "polylines must be a non-empty array"));
        }
        for (std::size_t p = 0; p < polylines.size(); ++p) {
            const std::string poly_context = context + ": polylines[" + std::to_string(p) + "]";
            const auto hips = polylines.at(p).get<std::vector<int>>();
            if (hips.size() < 2) {
                throw std::runtime_error(
                    path_message(poly_context, "polyline must list at least two stars"));
            }
            for (std::size_t s = 0; s < hips.size(); ++s) {
                validate_hip(hips[s], stars, poly_context + "[" + std::to_string(s) + "]");
            }
            figure.polylines.push_back(hips);
        }
        catalog.figures.push_back(std::move(figure));
    }
    return catalog;
}

} // namespace solar::app
