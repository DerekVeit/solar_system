#pragma once

#include "app/render/renderer.hpp"

namespace solar::app {

class GlRenderer final : public IRenderer {
  public:
    bool init(std::size_t max_points) override;
    void draw_points(std::span<const PointInstance> points) override;

  private:
    unsigned int program_{0};
    unsigned int vao_{0};
    int u_pos_location_{-1};
    int u_color_location_{-1};
    int u_point_size_location_{-1};
    std::size_t max_points_{0};
};

} // namespace solar::app