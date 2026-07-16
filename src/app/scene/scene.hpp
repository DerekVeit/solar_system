#pragma once

#include "app/render/renderer.hpp"
#include "app/scene/body_visual.hpp"
#include "sim/solar_system.hpp"

#include <memory>
#include <vector>

namespace solar::app {

class Scene {
  public:
    explicit Scene(std::unique_ptr<IRenderer> renderer);

    void add_body(BodyVisual body);

    bool init();

    void render(const sim::SolarSystem& simulation, float aspect_ratio, int framebuffer_height);

    float scale();

    void set_scale(float factor);

  private:
    std::unique_ptr<IRenderer> renderer_;
    std::vector<BodyVisual> bodies_;
    float view_half_extent_au_{2.0f};
};

} // namespace solar::app
