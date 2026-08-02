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

void cache_uniform(unsigned program, int& loc, const char* name) {
    loc = glGetUniformLocation(program, name);
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
    cache_uniforms();
    const MeshData mesh = generate_unit_sphere(48, 24);
    index_count = mesh.index_count;
    upload_mesh(mesh);
}

void SpherePipeline::cache_uniforms() {
    cache_uniform(program, view_loc, "u_view");
    cache_uniform(program, projection_loc, "u_projection");
    cache_uniform(program, model_loc, "u_model");
    cache_uniform(program, color_loc, "u_color");
    cache_uniform(program, ambient_loc, "u_ambient");
    cache_uniform(program, emission_loc, "u_emission");
    cache_uniform(program, light_dir_loc, "u_light_dir");
    cache_uniform(program, texture_offset_loc, "u_texture_offset");
    cache_uniform(program, diffuse_loc, "u_diffuse");
    cache_uniform(program, use_diffuse_loc, "u_use_diffuse");
    cache_uniform(program, night_loc, "u_night");
    cache_uniform(program, use_night_loc, "u_use_night");
    cache_uniform(program, show_graticules_loc, "u_show_graticules");
}

void SpherePipeline::upload_mesh(MeshData mesh) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.interleaved.size() * sizeof(float)),
                 mesh.interleaved.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(unsigned int)),
                 mesh.indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
}

} // namespace solar::app
