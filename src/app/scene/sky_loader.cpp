#include "app/scene/sky_loader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

namespace solar::app {

SkySpec load_sky_config(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open " + path.string());
    }

    const nlohmann::json root = nlohmann::json::parse(input);
    if (!root.is_object()) {
        throw std::runtime_error(path.string() + ": root must be a JSON object");
    }
    if (!root.contains("texture")) {
        throw std::runtime_error(path.string() + ": texture is required");
    }

    SkySpec spec{};
    spec.texture = root.at("texture").get<std::string>();
    if (spec.texture.empty()) {
        throw std::runtime_error(path.string() + ": texture must be non-empty");
    }
    if (root.contains("longitude_offset_deg")) {
        spec.longitude_offset_deg = root.at("longitude_offset_deg").get<float>();
    }
    if (root.contains("brightness")) {
        spec.brightness = root.at("brightness").get<float>();
    }
    if (root.contains("visible")) {
        spec.visible = root.at("visible").get<bool>();
    }
    return spec;
}

} // namespace solar::app
