#pragma once

#include "app/render/mesh_gen.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>
#include <glm/ext/matrix_float4x4.hpp>

#include <span>

namespace solar::app {

struct GlProgram {
    unsigned program{0};
    unsigned vao{0}, vbo{0};
    int view_loc{-1}, projection_loc{-1};
    void set_camera_uniforms(const glm::mat4& view, const glm::mat4& projection) const;
    void destroy_program();
};

struct StreamPipeline : GlProgram {
    int capacity; // max vertices

    void create(int capacity);
    void cache_uniforms();
    void destroy();
};

struct GlMeshProgram : GlProgram {
    unsigned ebo{0};
    int index_count{0};
    int model_loc{-1};

    void destroy_mesh_and_program();
};

struct SpherePipeline : GlMeshProgram {
    int color_loc{-1}, ambient_loc{-1}, emission_loc{-1}, light_dir_loc{-1};
    int diffuse_loc{-1}, use_diffuse_loc{-1}, night_loc{-1}, use_night_loc{-1};
    int texture_offset_loc{-1}, show_graticules_loc{-1};

    void create();
    void cache_uniforms();
    void upload_mesh(const MeshData& mesh);
    void destroy();
    void draw(std::span<const SphereInstance> spheres, std::size_t max_spheres,
              std::unordered_map<std::string, unsigned int>& textures, const glm::mat4& view,
              const glm::mat4& projection);
};

struct RingPipeline : GlMeshProgram {
    int light_dir_loc{-1};
    int map_loc{-1};
    int use_map_loc{-1};
    int inner_fraction_loc{-1};

    void create();
    void cache_uniforms();
    void upload_mesh(const MeshData& mesh);
    void destroy();
    void draw(std::span<const RingInstance> rings,
              std::unordered_map<std::string, unsigned int>& textures, const glm::mat4& view,
              const glm::mat4& projection);
};

} // namespace solar::app
