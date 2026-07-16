#include "app/render/gl_renderer.hpp"

#include "app/render/gl_shader.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <string_view>

namespace solar::app {

namespace {

constexpr std::string_view kPointVertexShader = R"(#version 460 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec4 a_color;
layout(location = 2) in float a_point_size;

out vec4 v_color;

void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);
    gl_PointSize = a_point_size;
    v_color = a_color;
}
)";

constexpr std::string_view kPointFragmentShader = R"(#version 460 core
in vec4 v_color;
out vec4 frag_color;
void main() {
    const vec2 coord = gl_PointCoord - vec2(0.5);
    if (dot(coord, coord) > 0.25) {
        discard;
    }
    frag_color = v_color;
}
)";

constexpr std::string_view kLineVertexShader = R"(#version 460 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec4 a_color;

out vec4 v_color;

void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);
    v_color = a_color;
}
)";

constexpr std::string_view kLineFragmentShader = R"(#version 460 core
in vec4 v_color;
out vec4 frag_color;
void main() {
    frag_color = v_color;
}
)";

void setup_line_vertex_layout() {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                          reinterpret_cast<void*>(offsetof(LineVertex, x_ndc)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                          reinterpret_cast<void*>(offsetof(LineVertex, color)));
}

} // namespace

bool GlRenderer::init(const RenderCapacity& capacity) {
    if (capacity.max_points == 0) {
        return false;
    }

    try {
        const unsigned int point_vertex_shader =
            compile_shader(GL_VERTEX_SHADER, kPointVertexShader);
        const unsigned int point_fragment_shader =
            compile_shader(GL_FRAGMENT_SHADER, kPointFragmentShader);
        point_program_ = link_program(point_vertex_shader, point_fragment_shader);
        glDeleteShader(point_vertex_shader);
        glDeleteShader(point_fragment_shader);

        const unsigned int line_vertex_shader = compile_shader(GL_VERTEX_SHADER, kLineVertexShader);
        const unsigned int line_fragment_shader =
            compile_shader(GL_FRAGMENT_SHADER, kLineFragmentShader);
        line_program_ = link_program(line_vertex_shader, line_fragment_shader);
        glDeleteShader(line_vertex_shader);
        glDeleteShader(line_fragment_shader);

        glGenVertexArrays(1, &point_vao_);
        glGenBuffers(1, &point_vbo_);
        glBindVertexArray(point_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, point_vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(capacity.max_points * sizeof(PointInstance)), nullptr,
                     GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointInstance),
                              reinterpret_cast<void*>(offsetof(PointInstance, x_ndc)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PointInstance),
                              reinterpret_cast<void*>(offsetof(PointInstance, color)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(PointInstance),
                              reinterpret_cast<void*>(offsetof(PointInstance, point_size)));

        glGenVertexArrays(1, &line_vao_);
        glGenBuffers(1, &line_vbo_);
        glBindVertexArray(line_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);
        const std::size_t line_buffer_capacity = capacity.max_line_vertices +
                                                 capacity.max_line_trail_vertices +
                                                 capacity.max_line_loop_vertices;
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(line_buffer_capacity * sizeof(LineVertex)), nullptr,
                     GL_DYNAMIC_DRAW);
        setup_line_vertex_layout();

        glBindVertexArray(0);

        glEnable(GL_PROGRAM_POINT_SIZE);

        max_points_ = capacity.max_points;
        max_line_vertices_ = capacity.max_line_vertices;
        max_line_trail_vertices_ = capacity.max_line_trail_vertices;
        max_line_loop_vertices_ = capacity.max_line_loop_vertices;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void GlRenderer::draw_points(std::span<const PointInstance> points) {
    if (point_program_ == 0 || point_vao_ == 0 || point_vbo_ == 0) {
        return;
    }

    const std::size_t count = points.size() < max_points_ ? points.size() : max_points_;
    if (count == 0) {
        return;
    }

    glUseProgram(point_program_);
    glBindVertexArray(point_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, point_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(PointInstance)),
                    points.data());
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
}

void GlRenderer::draw_line_primitives(unsigned int mode, std::span<const LinePrimitive> primitives,
                                      std::size_t max_vertices) {
    if (line_program_ == 0 || line_vao_ == 0 || line_vbo_ == 0) {
        return;
    }

    glUseProgram(line_program_);
    glBindVertexArray(line_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);

    for (const LinePrimitive& primitive : primitives) {
        const std::size_t count =
            primitive.vertices.size() < max_vertices ? primitive.vertices.size() : max_vertices;
        if (count < 2) {
            continue;
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(LineVertex)),
                        primitive.vertices.data());
        glDrawArrays(mode, 0, static_cast<GLsizei>(count));
    }
}

void GlRenderer::draw(const DrawBatch& batch) {
    draw_line_primitives(GL_LINE_STRIP, batch.line_trails, max_line_trail_vertices_);
    draw_line_primitives(GL_LINE_LOOP, batch.line_loops, max_line_loop_vertices_);
    draw_line_primitives(GL_LINES, batch.lines, max_line_vertices_);
    draw_points(batch.points);
}

} // namespace solar::app