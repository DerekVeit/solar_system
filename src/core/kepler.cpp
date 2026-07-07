#include "core/kepler.hpp"

#include <cmath>

namespace solar::core {

namespace {

double normalize_angle(double radians) {
    radians = std::fmod(radians, kTwoPi);
    if (radians < 0.0) {
        radians += kTwoPi;
    }
    return radians;
}

glm::dmat3 rotation_matrix(const KeplerianElements& elements) {
    const double cos_i = std::cos(elements.inclination_rad);
    const double sin_i = std::sin(elements.inclination_rad);
    const double cos_O = std::cos(elements.longitude_ascending_node_rad);
    const double sin_O = std::sin(elements.longitude_ascending_node_rad);
    const double cos_w = std::cos(elements.argument_periapsis_rad);
    const double sin_w = std::sin(elements.argument_periapsis_rad);

    const glm::dvec3 p{
        cos_O * cos_w - sin_O * sin_w * cos_i,
        sin_O * cos_w + cos_O * sin_w * cos_i,
        sin_w * sin_i,
    };
    const glm::dvec3 q{
        -cos_O * sin_w - sin_O * cos_w * cos_i,
        -sin_O * sin_w + cos_O * cos_w * cos_i,
        cos_w * sin_i,
    };
    const glm::dvec3 w{
        // The perifocal z is always 0.
        0.0, // sin_O * sin_i,
        0.0, // -cos_O * sin_i,
        1.0, // cos_i,
    };

    return glm::dmat3{p, q, w};
}

double cube(double num) { return num * num * num; }

double mean_anomaly_at_epoch(GravitationalParameter mu, const KeplerianElements& elements,
                             Epoch epoch) {
    const Duration elapsed{(epoch.jd - elements.epoch.jd) * kSecondsPerDay};

    const double mean_motion = std::sqrt(mu / cube(elements.semi_major_axis_km));
    const double mean_anomaly =
        normalize_angle(elements.mean_anomaly_at_epoch_rad + mean_motion * elapsed.count());
    return mean_anomaly;
}

const glm::dvec3 perifocal_velocity_at_E(GravitationalParameter mu,
                                         const KeplerianElements& elements,
                                         const double eccentric_anomaly,
                                         const Displacement perifocal_position) {
    const double radius = perifocal_position.length();
    const double factor = std::sqrt(mu * elements.semi_major_axis_km) / radius; // km/s
    const glm::dvec3 perifocal_velocity{
        -std::sin(eccentric_anomaly) * factor,
        std::sqrt(1.0 - elements.eccentricity * elements.eccentricity) *
            std::cos(eccentric_anomaly) * factor,
        0.0,
    };
    return perifocal_velocity;
}

} // namespace

double solve_kepler(double mean_anomaly_rad, double eccentricity) {
    mean_anomaly_rad = normalize_angle(mean_anomaly_rad);

    double eccentric_anomaly = mean_anomaly_rad;
    if (eccentricity > 0.8) {
        eccentric_anomaly = kPi;
    }

    for (int iteration = 0; iteration < 30; ++iteration) {
        const double delta =
            (eccentric_anomaly - eccentricity * std::sin(eccentric_anomaly) - mean_anomaly_rad) /
            (1.0 - eccentricity * std::cos(eccentric_anomaly));
        eccentric_anomaly -= delta;
        if (std::abs(delta) < 1e-12) {
            break;
        }
    }

    return eccentric_anomaly;
}

Displacement position_in_orbital_plane(const KeplerianElements& elements,
                                       double eccentric_anomaly) {
    const double e = elements.eccentricity;
    const double a = elements.semi_major_axis_km;
    const double b = a * std::sqrt(1.0 - e * e);

    const double x = a * (std::cos(eccentric_anomaly) - e);
    const double y = b * std::sin(eccentric_anomaly);

    return Displacement{glm::dvec3{x, y, 0.0}};
}

StateVector state_from_kepler(GravitationalParameter mu, const KeplerianElements& elements,
                              Epoch epoch) {
    // Conceptually:
    // time (epoch) → M → E → (x,y) in orbital plane → rotate to 3D → position
    //                                               → velocity in plane → rotate → velocity

    const double mean_anomaly = mean_anomaly_at_epoch(mu, elements, epoch);
    const double eccentric_anomaly = solve_kepler(mean_anomaly, elements.eccentricity);
    const Displacement perifocal_position = position_in_orbital_plane(elements, eccentric_anomaly);

    const glm::dmat3 rotation = rotation_matrix(elements);
    const glm::dvec3 position_km = rotation * perifocal_position.km;

    const glm::dvec3 perifocal_velocity =
        perifocal_velocity_at_E(mu, elements, eccentric_anomaly, perifocal_position);
    const glm::dvec3 velocity_km_s = rotation * perifocal_velocity;

    return StateVector{
        Displacement{position_km},
        Velocity{velocity_km_s},
    };
}

} // namespace solar::core