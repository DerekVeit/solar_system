#pragma once

#include "app/color.hpp"
#include "glm/ext/vector_float3.hpp"

#include <vector>

namespace solar::app {

/// Sphere in camera-relative km (center = world minus camera origin).
struct SphereInstance {
    float x_km{0.0f};
    float y_km{0.0f};
    float z_km{0.0f};
    float radius_km{1.0f};
    Color color{};
    float ambient{};
    float emission{};
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

} // namespace solar::app
