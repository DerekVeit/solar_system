#include "gl_pipeline.hpp"

#include "app/files.hpp"
#include "app/render/gl_shader.hpp"

#include <glad/gl.h>

#include <stdexcept>
#include <vector>

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

void prepare_buffer(unsigned& id, GLenum type, GLsizeiptr size_bytes, const void* start) {
    glGenBuffers(1, &id);
    glBindBuffer(type, id);
    glBufferData(type, size_bytes, start, GL_STATIC_DRAW);
}

struct VertItemSpec {
    int count{};     // components (1-4)
    int type{};      // e.g.GL_FLOAT
    int type_size{}; // bytes per component, e.g. sizeof(float)
};

constexpr VertItemSpec floats(int count) {
    return VertItemSpec{count, GL_FLOAT, static_cast<int>(sizeof(float))};
}

void prepare_vertex_array_attribs(int stride, std::vector<VertItemSpec> specs) {
    unsigned offset = 0;
    for (unsigned i = 0; i < specs.size(); ++i) {
        VertItemSpec spec = specs[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, spec.count, spec.type, GL_FALSE, stride,
                              reinterpret_cast<void*>(offset));
        offset += static_cast<unsigned>(spec.count * spec.type_size);
    }
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

void SpherePipeline::upload_mesh(const MeshData& mesh) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    prepare_buffer(vbo, GL_ARRAY_BUFFER, mesh.interleaved.size() * sizeof(float),
                   mesh.interleaved.data());

    prepare_buffer(ebo, GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned),
                   mesh.indices.data());

    prepare_vertex_array_attribs(mesh.stride, {floats(3), floats(2)});
}

} // namespace solar::app
