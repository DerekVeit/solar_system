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

    void body_scaling(bool enabled);

    /// Pan the camera by fractions of the current view width and height.
    void pan_view_fraction(float delta_x_fraction, float delta_y_fraction);

    void reset_view_center();

  private:
    std::unique_ptr<IRenderer> renderer_;
    std::vector<BodyVisual> bodies_;
    float view_half_extent_au_{2.0f};
    bool body_scaling_{true};
    float view_center_x_au_{0.0f};
    float view_center_y_au_{0.0f};
    float last_aspect_ratio_{1.0f};
};

} // namespace solar::app
