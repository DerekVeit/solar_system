#include "app/earth_renderer.hpp"

#include "core/constants.hpp"

#include <glad/gl.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace solar::app {

namespace {

unsigned int compile_shader(unsigned int type, std::string_view source) {
    const unsigned int shader = glCreateShader(type);
    const char* source_ptr = source.data();
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char log[512]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        throw std::runtime_error(std::string{"shader compile failed: "} + log);
    }
    return shader;
}

unsigned int link_program(unsigned int vertex_shader, unsigned int fragment_shader) {
    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        throw std::runtime_error(std::string{"program link failed: "} + log);
    }
    return program;
}

constexpr std::string_view kVertexShader = R"(#version 460 core
uniform vec2 u_pos;
void main() {
    gl_Position = vec4(u_pos, 0.0, 1.0);
    gl_PointSize = 14.0;
}
)";

constexpr std::string_view kFragmentShader = R"(#version 460 core
out vec4 frag_color;
void main() {
    frag_color = vec4(0.45, 0.75, 1.0, 1.0);
}
)";

} // namespace

bool EarthRenderer::init() {
    try {
        const unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, kVertexShader);
        const unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, kFragmentShader);
        program_ = link_program(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        u_pos_location_ = glGetUniformLocation(program_, "u_pos");

        glGenVertexArrays(1, &vao_);

        glEnable(GL_PROGRAM_POINT_SIZE);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void EarthRenderer::draw(const core::Displacement& earth_position) const {
    if (program_ == 0 || u_pos_location_ < 0) {
        return;
    }

    const float scale = 2.0f * static_cast<float>(core::kAuKm);
    const float x_ndc = static_cast<float>(earth_position.km.x) / scale;
    const float y_ndc = static_cast<float>(earth_position.km.y) / scale;

    glUseProgram(program_);
    glBindVertexArray(vao_);
    glUniform2f(u_pos_location_, x_ndc, y_ndc);
    glDrawArrays(GL_POINTS, 0, 1);
}

} // namespace solar::app