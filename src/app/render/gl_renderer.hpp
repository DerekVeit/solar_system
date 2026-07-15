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
    unsigned int vbo_{0};
    std::size_t max_points_{0};
};

} // namespace solar::app
