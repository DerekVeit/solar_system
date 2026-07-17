#pragma once

#include "app/render/renderer.hpp"

#include <span>

namespace solar::app {

class GlRenderer final : public IRenderer {
  public:
    GlRenderer() = default;
    ~GlRenderer() override;

    GlRenderer(const GlRenderer&) = delete;
    GlRenderer& operator=(const GlRenderer&) = delete;
    GlRenderer(GlRenderer&&) = delete;
    GlRenderer& operator=(GlRenderer&&) = delete;

    bool init(const RenderCapacity& capacity) override;
    void draw(const DrawBatch& batch, const glm::mat4& view, const glm::mat4& projection) override;

  private:
    void destroy();
    void set_camera_uniforms(unsigned int program, int view_loc, int projection_loc,
                             const glm::mat4& view, const glm::mat4& projection) const;
    void draw_points(std::span<const PointInstance> points, const glm::mat4& view,
                     const glm::mat4& projection);
    void draw_line_primitives(unsigned int mode, std::span<const LinePrimitive> primitives,
                              std::size_t max_vertices, const glm::mat4& view,
                              const glm::mat4& projection);

    unsigned int point_program_{0};
    unsigned int line_program_{0};
    unsigned int point_vao_{0};
    unsigned int point_vbo_{0};
    unsigned int line_vao_{0};
    unsigned int line_vbo_{0};
    int point_view_loc_{-1};
    int point_projection_loc_{-1};
    int line_view_loc_{-1};
    int line_projection_loc_{-1};
    std::size_t max_points_{0};
    std::size_t max_line_vertices_{0};
    std::size_t max_line_trail_vertices_{0};
    std::size_t max_line_loop_vertices_{0};
};

} // namespace solar::app
