#include "gl_pipeline.hpp"

#include "app/files.hpp"
#include "app/render/gl_shader.hpp"
#include <stdexcept>

namespace solar::app {

namespace {

std::string shader_source(std::string filename) {
    std::string src = read_text_file(asset_path("shaders/" + filename));
    if (src.empty() || !src.starts_with("#version ")) {
        throw std::runtime_error("failed to get the shader source of " + filename);
    }
    return src;
}

} // namespace

void SpherePipeline::create() {
    std::string vs_src = shader_source("sphere.vert");
    std::string fs_src = shader_source("sphere.frag");
    const unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vs_src);
    const unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    program = link_program(vertex_shader, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

} // namespace solar::app
