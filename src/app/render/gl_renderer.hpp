#pragma once

#include "app/render/renderer.hpp"

#include <span>
#include <unordered_map>

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
    void upload_texture(const std::string& path, const TextureImage& image) override;

  private:
    void destroy();
    void set_camera_uniforms(unsigned int program, int view_loc, int projection_loc,
                             const glm::mat4& view, const glm::mat4& projection) const;
    void draw_spheres(std::span<const SphereInstance> spheres, const glm::mat4& view,
                      const glm::mat4& projection);
    void draw_rings(std::span<const RingInstance> rings, const glm::mat4& view,
                    const glm::mat4& projection);
    void draw_line_primitives(unsigned int mode, std::span<const LinePrimitive> primitives,
                              std::size_t max_vertices, const glm::mat4& view,
                              const glm::mat4& projection);

    unsigned int sphere_program_{0};
    unsigned int line_program_{0};
    unsigned int sphere_vao_{0};
    unsigned int sphere_vbo_{0};
    unsigned int sphere_ebo_{0};
    unsigned int line_vao_{0};
    unsigned int line_vbo_{0};
    int sphere_view_loc_{-1};
    int sphere_projection_loc_{-1};
    int sphere_model_loc_{-1};
    int sphere_color_loc_{-1};
    int sphere_ambient_loc_{-1};
    int sphere_emission_loc_{-1};
    int sphere_light_dir_loc_{-1};
    int sphere_texture_offset_loc_{-1};
    int sphere_diffuse_loc_{-1};
    int sphere_night_loc_{-1};
    int sphere_use_diffuse_loc_{-1};
    int sphere_use_night_loc_{-1};
    int sphere_show_graticules_loc_{-1};
    int line_view_loc_{-1};
    int line_projection_loc_{-1};
    int sphere_index_count_{0};
    std::size_t max_spheres_{0};
    std::size_t max_line_vertices_{0};
    std::size_t max_line_trail_vertices_{0};
    std::size_t max_line_loop_vertices_{0};
    std::unordered_map<std::string, unsigned int> textures_;

    unsigned int ring_program_{0};
    unsigned int ring_vao_{0};
    unsigned int ring_vbo_{0};
    unsigned int ring_ebo_{0};
    int ring_view_loc_{-1};
    int ring_projection_loc_{-1};
    int ring_model_loc_{-1};
    int ring_light_dir_loc_{-1};
    int ring_map_loc_{-1};
    int ring_use_map_loc_{-1};
    int ring_inner_fraction_loc_{-1};
    int ring_index_count_{0};
};

} // namespace solar::app
