#include "core/json_loader.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "core/constants.hpp"

namespace solar::core {

namespace {

double degrees_to_radians(double degrees) { return degrees * kDegToRad; }

KeplerianElements parse_elements(const nlohmann::json& json) {
    const auto& kepler = json.at("kepler");
    return KeplerianElements{
        .semi_major_axis_km = kepler.at("a_km").get<double>(),
        .eccentricity = kepler.at("e").get<double>(),
        .inclination_rad = degrees_to_radians(kepler.at("i_deg").get<double>()),
        .longitude_ascending_node_rad = degrees_to_radians(kepler.at("Omega_deg").get<double>()),
        .argument_periapsis_rad = degrees_to_radians(kepler.at("omega_deg").get<double>()),
        .mean_anomaly_at_epoch_rad = degrees_to_radians(kepler.at("M0_deg").get<double>()),
        .epoch = Epoch{kepler.at("epoch_jd").get<double>()},
    };
}

}  // namespace

std::vector<BodyDefinition> load_bodies(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open body data: " + path.string());
    }

    const nlohmann::json root = nlohmann::json::parse(input);
    std::vector<BodyDefinition> bodies;
    bodies.reserve(root.at("bodies").size());

    for (const auto& entry : root.at("bodies")) {
        bodies.push_back(BodyDefinition{
            .name = entry.at("name").get<std::string>(),
            .gravitational_parameter_km3_s2 = entry.at("mu_km3_s2").get<double>(),
            .radius_km = entry.at("radius_km").get<double>(),
            .elements = parse_elements(entry),
        });
    }

    return bodies;
}

}  // namespace solar::core