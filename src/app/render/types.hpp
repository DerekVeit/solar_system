#pragma once

#include "app/color.hpp"

#include <vector>

namespace solar::app {

/// Point in camera-relative km (world minus camera origin).
struct PointInstance {
    float x_km{0.0f};
    float y_km{0.0f};
    float z_km{0.0f};
    Color color{};
    float point_size{1.0f};
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
    std::vector<PointInstance> points;
    std::vector<LinePrimitive> lines;
    std::vector<LinePrimitive> line_trails;
    std::vector<LinePrimitive> line_loops;
};

struct RenderCapacity {
    std::size_t max_points{0};
    std::size_t max_line_vertices{0};
    std::size_t max_line_trail_vertices{0};
    std::size_t max_line_loop_vertices{0};
};

} // namespace solar::app
