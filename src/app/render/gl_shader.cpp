#include "app/render/gl_shader.hpp"

#include <glad/gl.h>

#include <stdexcept>
#include <string>

namespace solar::app {

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

} // namespace solar::app