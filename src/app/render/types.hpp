#pragma once

#include "app/color.hpp"
#include "app/scene/body_visual_config.hpp"

#include <glad/gl.h>
#include <glm/ext/matrix_double3x3.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace solar::app {

/// Sphere in camera-relative km (center = world minus camera origin).
struct SphereInstance {
    float x_km{0.0f};
    float y_km{0.0f};
    float z_km{0.0f};
    float radius_km{1.0f};
    glm::mat3 rotation{1.0};
    BodySurface surface{};
    glm::vec3 light_dir{0.0f, 0.0f, 1.0f};
    bool show_graticules{true};
};

struct RingInstance {
    float x_km{0.0f};
    float y_km{0.0f};
    float z_km{0.0f};
    float inner_radius_km{1.0f};
    float outer_radius_km{1.0f};
    glm::mat3 rotation{1.0};
    std::string map{};
    glm::vec3 light_dir{0.0f, 0.0f, 1.0f};
};

/// Line vertex in camera-relative km.
struct LineVertex {
    float x_km{0.0f};
    float y_km{0.0f};
    float z_km{0.0f};
    Color color{};
};

struct LinePrimitive {
    std::vector<LineVertex> vertices;
};

/// Per-frame geometry collected by the scene and consumed by the renderer.
struct DrawBatch {
    std::vector<SphereInstance> spheres;
    std::vector<RingInstance> rings;
    std::vector<LinePrimitive> lines;
    std::vector<LinePrimitive> line_trails;
    std::vector<LinePrimitive> line_loops;
};

struct RenderCapacity {
    std::size_t max_spheres{0};
    std::size_t max_line_vertices{0};
    std::size_t max_line_trail_vertices{0};
    std::size_t max_line_loop_vertices{0};
};

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

} // namespace solar::app
