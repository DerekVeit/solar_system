#include "app/scene/body_visual.hpp"

namespace solar::app {

BodyVisual::BodyVisual(std::string name, Color color, float point_size)
    : name_(std::move(name))
    , color_(color)
    , point_size_(point_size) {}

void BodyVisual::append_point(const sim::SolarSystem& simulation, float view_half_extent_au,
                              std::vector<PointInstance>& points) const {
    if (name_ == "Sun") {
        points.push_back(PointInstance{0.0f, 0.0f, color_, point_size_});
        return;
    }

    const auto position = simulation.state(name_).position;
    const float scale = view_half_extent_au * static_cast<float>(core::kAuKm);
    points.push_back(PointInstance{static_cast<float>(position.km.x) / scale,
                                   static_cast<float>(position.km.y) / scale, color_, point_size_});
}

} // namespace solar::app