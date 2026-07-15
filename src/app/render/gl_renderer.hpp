#pragma once

#include "app/render/renderer.hpp"

#include <span>

namespace solar::app {

class GlRenderer final : public IRenderer {
  public:
    bool init(const RenderCapacity& capacity) override;
    void draw(const DrawBatch& batch) override;

  private:
    void draw_points(std::span<const PointInstance> points);
    void draw_line_primitives(unsigned int mode, std::span<const LinePrimitive> primitives,
                              std::size_t max_vertices);

    unsigned int point_program_{0};
    unsigned int line_program_{0};
    unsigned int point_vao_{0};
    unsigned int point_vbo_{0};
    unsigned int line_vao_{0};
    unsigned int line_vbo_{0};
    std::size_t max_points_{0};
    std::size_t max_line_vertices_{0};
    std::size_t max_line_trail_vertices_{0};
    std::size_t max_line_loop_vertices_{0};
};

} // namespace solar::app