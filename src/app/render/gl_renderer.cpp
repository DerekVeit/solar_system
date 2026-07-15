#include "app/render/gl_renderer.hpp"

#include "app/render/gl_shader.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <string_view>

namespace solar::app {

namespace {

constexpr std::string_view kVertexShader = R"(#version 460 core
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

constexpr std::string_view kFragmentShader = R"(#version 460 core
in vec4 v_color;
out vec4 frag_color;
void main() {
    frag_color = v_color;
}
)";

} // namespace

bool GlRenderer::init(std::size_t max_points) {
    if (max_points == 0) {
        return false;
    }

    try {
        const unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, kVertexShader);
        const unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, kFragmentShader);
        program_ = link_program(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(max_points * sizeof(PointInstance)),
                     nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointInstance),
                              reinterpret_cast<void*>(offsetof(PointInstance, x_ndc)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PointInstance),
                              reinterpret_cast<void*>(offsetof(PointInstance, color)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(PointInstance),
                              reinterpret_cast<void*>(offsetof(PointInstance, point_size)));

        glBindVertexArray(0);

        glEnable(GL_PROGRAM_POINT_SIZE);

        max_points_ = max_points;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void GlRenderer::draw_points(std::span<const PointInstance> points) {
    if (program_ == 0 || vao_ == 0 || vbo_ == 0) {
        return;
    }

    const std::size_t count = points.size() < max_points_ ? points.size() : max_points_;
    if (count == 0) {
        return;
    }

    glUseProgram(program_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(PointInstance)),
                    points.data());
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(count));
}

} // namespace solar::app
