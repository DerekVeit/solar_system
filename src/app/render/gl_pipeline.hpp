#pragma once

#include "app/render/mesh_gen.hpp"

#include <glad/gl.h>

namespace solar::app {

struct GlProgram {
    unsigned program{0};
    unsigned vao{0}, vbo{0};
    int view_loc{-1}, projection_loc{-1};
    void destroy_program();
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
};

} // namespace solar::app
