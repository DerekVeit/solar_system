#include "app/scene/scene.hpp"

namespace solar::app {

Scene::Scene(std::unique_ptr<IRenderer> renderer)
    : renderer_(std::move(renderer)) {}

void Scene::add_body(BodyVisual body) { bodies_.push_back(std::move(body)); }

bool Scene::init() {
    if (renderer_ == nullptr) {
        return false;
    }
    return renderer_->init(bodies_.size());
}

void Scene::render(const sim::SolarSystem& simulation) {
    if (renderer_ == nullptr) {
        return;
    }

    std::vector<PointInstance> points;
    points.reserve(bodies_.size());
    for (const BodyVisual& body : bodies_) {
        body.append_point(simulation, view_half_extent_au_, points);
    }
    renderer_->draw_points(points);
}

} // namespace solar::app