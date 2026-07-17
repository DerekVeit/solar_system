#pragma once

#include "app/render/renderer.hpp"
#include "app/scene/body_visual.hpp"
#include "app/scene/camera.hpp"
#include "sim/solar_system.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace solar::app {

class Scene {
  public:
    explicit Scene(std::unique_ptr<IRenderer> renderer);

    void add_body(BodyVisual body);

    bool init();

    void render(const sim::SolarSystem& simulation, float aspect_ratio, int framebuffer_height);

    /// Half the visible view height in AU (center to top edge).
    [[nodiscard]] float view_half_extent_au() const { return camera_.half_extent_au(); }

    void set_view_half_extent_au(float half_extent_au);

    void body_scaling(bool enabled);

    /// Pan the camera by fractions of the current view width and height.
    void pan_view_fraction(float delta_x_fraction, float delta_y_fraction);

    void reset_view_center();

    void set_follow_target(const sim::SolarSystem& simulation,
                           std::optional<std::string> body_name);

    std::optional<std::string> release_from_follow();

  private:
    void update_view_center_from_follow(const sim::SolarSystem& simulation);

    std::unique_ptr<IRenderer> renderer_;
    std::vector<BodyVisual> bodies_;
    Camera camera_;
    bool body_scaling_{true};
    std::optional<std::string> followed_body_;
    float follow_offset_x_au_{0.0f};
    float follow_offset_y_au_{0.0f};
};

} // namespace solar::app
