#include "app/render/gl_renderer.hpp"

#include "app/render/gl_shader.hpp"

#include <glad/gl.h>

#include <string_view>

namespace solar::app {

namespace {

constexpr std::string_view kVertexShader = R"(#version 460 core
uniform vec2 u_pos;
uniform float u_point_size;
void main() {
    gl_Position = vec4(u_pos, 0.0, 1.0);
    gl_PointSize = u_point_size;
}
)";

constexpr std::string_view kFragmentShader = R"(#version 460 core
uniform vec4 u_color;
out vec4 frag_color;
void main() {
    frag_color = u_color;
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

        u_pos_location_ = glGetUniformLocation(program_, "u_pos");
        u_color_location_ = glGetUniformLocation(program_, "u_color");
        u_point_size_location_ = glGetUniformLocation(program_, "u_point_size");

        if (u_pos_location_ < 0 || u_color_location_ < 0 || u_point_size_location_ < 0) {
            return false;
        }

        glGenVertexArrays(1, &vao_);
        glEnable(GL_PROGRAM_POINT_SIZE);

        max_points_ = max_points;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void GlRenderer::draw_points(std::span<const PointInstance> points) {
    if (program_ == 0 || vao_ == 0) {
        return;
    }

    glUseProgram(program_);
    glBindVertexArray(vao_);

    const std::size_t count = points.size() < max_points_ ? points.size() : max_points_;
    for (std::size_t i = 0; i < count; ++i) {
        const PointInstance& point = points[i];
        glUniform2f(u_pos_location_, point.x_ndc, point.y_ndc);
        glUniform4f(u_color_location_, point.color.r, point.color.g, point.color.b, point.color.a);
        glUniform1f(u_point_size_location_, point.point_size);
        glDrawArrays(GL_POINTS, 0, 1);
    }
}

} // namespace solar::app