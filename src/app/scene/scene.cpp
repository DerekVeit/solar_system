#include "app/scene/scene.hpp"

namespace solar::app {

Scene::Scene(std::unique_ptr<IRenderer> renderer)
    : renderer_(std::move(renderer)) {}

void Scene::add_body(BodyVisual body) { bodies_.push_back(std::move(body)); }

bool Scene::init() {
    if (renderer_ == nullptr) {
        return false;
    }

    std::size_t trail_bodies = 0;
    for (const BodyVisual& body : bodies_) {
        if (body.draws_orbit_trails()) {
            ++trail_bodies;
        }
    }

    const RenderCapacity capacity{
        .max_points = bodies_.size() + trail_bodies * BodyVisual::kTailSamples,
        .max_line_vertices = 0,
        .max_line_trail_vertices = trail_bodies * BodyVisual::kTailSamples,
        .max_line_loop_vertices = trail_bodies * BodyVisual::kOrbitSamples,
    };
    return renderer_->init(capacity);
}

void Scene::render(const sim::SolarSystem& simulation, float aspect_ratio, int framebuffer_height) {
    if (renderer_ == nullptr) {
        return;
    }

    DrawBatch batch;
    batch.points.reserve(bodies_.size() + bodies_.size() * BodyVisual::kTailSamples);
    batch.line_loops.reserve(bodies_.size());
    batch.line_trails.reserve(bodies_.size());

    for (const BodyVisual& body : bodies_) {
        body.append_draw(simulation, view_half_extent_au_, aspect_ratio, framebuffer_height, batch);
    }
    renderer_->draw(batch);
}

float Scene::scale() { return view_half_extent_au_ / 2.0f; }

void Scene::set_scale(float factor) { view_half_extent_au_ = 2.0f * factor; }

} // namespace solar::app