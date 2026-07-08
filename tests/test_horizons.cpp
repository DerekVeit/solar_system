#include "core/constants.hpp"
#include "core/json_loader.hpp"
#include "core/kepler_ephemeris.hpp"
#include "core/types.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using Catch::Approx;

namespace {

struct HorizonsBodyReference {
    std::string name;
    double x_au{};
    double y_au{};
    double z_au{};
    double tolerance_au{};
    double tolerance_lon_deg{};
};

struct HorizonsEpochReference {
    std::string label;
    double jd{};
    std::vector<HorizonsBodyReference> bodies;
};

double heliocentric_longitude_deg(double x_au, double y_au) {
    return std::atan2(y_au, x_au) * solar::core::kRadToDeg;
}

double angular_difference_deg(double a_deg, double b_deg) {
    double delta = std::fmod(a_deg - b_deg + 180.0, 360.0);
    if (delta < 0.0) {
        delta += 360.0;
    }
    return delta - 180.0;
}

double position_error_au(const solar::core::Displacement& position,
                         const HorizonsBodyReference& reference) {
    const double dx = position.km.x / solar::core::kAuKm - reference.x_au;
    const double dy = position.km.y / solar::core::kAuKm - reference.y_au;
    const double dz = position.km.z / solar::core::kAuKm - reference.z_au;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

std::vector<HorizonsEpochReference> load_horizons_reference(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) {
        throw std::runtime_error("failed to open Horizons reference data: " + path.string());
    }

    const nlohmann::json json = nlohmann::json::parse(input);
    std::vector<HorizonsEpochReference> cases;
    for (const auto& epoch_case : json.at("cases")) {
        HorizonsEpochReference entry{
            .label = epoch_case.at("label").get<std::string>(),
            .jd = epoch_case.at("jd").get<double>(),
        };

        for (const auto& body : epoch_case.at("bodies")) {
            entry.bodies.push_back(HorizonsBodyReference{
                .name = body.at("name").get<std::string>(),
                .x_au = body.at("x_au").get<double>(),
                .y_au = body.at("y_au").get<double>(),
                .z_au = body.at("z_au").get<double>(),
                .tolerance_au = body.at("tolerance_au").get<double>(),
                .tolerance_lon_deg = body.at("tolerance_lon_deg").get<double>(),
            });
        }

        cases.push_back(std::move(entry));
    }

    return cases;
}

} // namespace

TEST_CASE("KeplerEphemeris agrees with JPL Horizons reference ephemeris", "[horizons][kepler]") {
    const auto reference_cases = load_horizons_reference("tests/data/horizons_reference.json");
    const auto bodies = solar::core::load_bodies("assets/data/bodies.json");
    const solar::core::KeplerEphemeris ephemeris(bodies);

    for (const HorizonsEpochReference& epoch_case : reference_cases) {
        INFO(epoch_case.label);
        const solar::core::Epoch epoch{epoch_case.jd};

        for (const HorizonsBodyReference& body_reference : epoch_case.bodies) {
            INFO(body_reference.name);

            const solar::core::StateVector state = ephemeris.state(body_reference.name, epoch);
            const double error_au = position_error_au(state.position, body_reference);

            const double reference_lon =
                heliocentric_longitude_deg(body_reference.x_au, body_reference.y_au);
            const double simulated_lon = heliocentric_longitude_deg(
                state.position.km.x / solar::core::kAuKm, state.position.km.y / solar::core::kAuKm);
            const double lon_error_deg =
                std::abs(angular_difference_deg(simulated_lon, reference_lon));

            CHECK(error_au == Approx(0.0).margin(body_reference.tolerance_au));
            CHECK(lon_error_deg == Approx(0.0).margin(body_reference.tolerance_lon_deg));
        }
    }
}