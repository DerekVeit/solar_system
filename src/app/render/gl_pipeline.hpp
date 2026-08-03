#pragma once

#include "app/render/mesh_gen.hpp"

#include <glad/gl.h>

namespace solar::app {

struct GlMeshProgram {
    unsigned program{0};
    unsigned vao{0}, vbo{0}, ebo{0};
    int index_count{0};
    int model_loc{-1}, view_loc{-1}, projection_loc{-1};
};

struct SpherePipeline : GlMeshProgram {
    int color_loc{-1}, ambient_loc{-1}, emission_loc{-1}, light_dir_loc{-1};
    int diffuse_loc{-1}, use_diffuse_loc{-1}, night_loc{-1}, use_night_loc{-1};
    int texture_offset_loc{-1}, show_graticules_loc{-1};
    void create();
    void cache_uniforms();
    void upload_mesh(const MeshData& mesh);
    void destroy() {
        if (program != 0) {
            glDeleteProgram(program);
            program = 0;
        }
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (ebo != 0) {
            glDeleteBuffers(1, &ebo);
            ebo = 0;
        }
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        view_loc = -1;
        projection_loc = -1;
        model_loc = -1;
        color_loc = -1;
        ambient_loc = -1;
        emission_loc = -1;
        light_dir_loc = -1;
        texture_offset_loc = -1;
        diffuse_loc = -1;
        night_loc = -1;
        use_diffuse_loc = -1;
        use_night_loc = -1;
        show_graticules_loc = -1;
        index_count = 0;
    }
};

struct RingPipeline : GlMeshProgram {
    int light_dir_loc_{-1}, map_loc{-1}, use_map_loc{-1}, inner_fraction_loc{-1};
};

} // namespace solar::app
