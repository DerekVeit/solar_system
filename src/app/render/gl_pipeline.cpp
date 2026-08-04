#include "app/render/gl_pipeline.hpp"

#include "app/files.hpp"
#include "app/render/gl_shader.hpp"

#include <glad/gl.h>

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace solar::app {

namespace {

std::string shader_source(std::string filename) {
    std::filesystem::path path = asset_path("shaders/" + filename);
    std::string src = read_text_file(path);
    if (src.empty() || !src.starts_with("#version ")) {
        throw std::runtime_error("failed to get the shader source of " + path.string());
    }
    return src;
}

void cache_uniform(unsigned program, int& loc, const char* name) {
    loc = glGetUniformLocation(program, name);
}

void prepare_buffer(unsigned& id, GLenum type, GLenum usage, GLsizeiptr size_bytes,
                    const void* start) {
    glGenBuffers(1, &id);
    glBindBuffer(type, id);
    glBufferData(type, size_bytes, start, usage);
}

struct VertItemSpec {
    int count{};     // components (1-4)
    GLenum type{};   // e.g. GL_FLOAT
    int type_size{}; // bytes per component, e.g. sizeof(float)
};

constexpr VertItemSpec floats(int count) {
    return VertItemSpec{count, GL_FLOAT, static_cast<int>(sizeof(float))};
}

void prepare_vertex_array_attribs(int stride, std::vector<VertItemSpec> specs) {
    unsigned offset = 0;
    for (unsigned i = 0; i < specs.size(); ++i) {
        const VertItemSpec spec = specs[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, spec.count, spec.type, GL_FALSE, stride,
                              reinterpret_cast<void*>(static_cast<uintptr_t>(offset)));
        offset += static_cast<unsigned>(spec.count * spec.type_size);
    }
}

void link_program_from_files(unsigned& program, const char* vert_name, const char* frag_name) {
    const std::string vs_src = shader_source(vert_name);
    const std::string fs_src = shader_source(frag_name);
    const unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vs_src);
    const unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    program = link_program(vertex_shader, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

} // namespace

void GlProgram::destroy_program() {
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    view_loc = -1;
    projection_loc = -1;
}

void StreamPipeline::create() {
    link_program_from_files(program, "line.vert", "line.frag");
}

void StreamPipeline::destroy() { destroy_program(); }

void GlMeshProgram::destroy_mesh_and_program() {
    destroy_program();
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
    model_loc = -1;
    index_count = 0;
}

void SpherePipeline::create() {
    link_program_from_files(program, "sphere.vert", "sphere.frag");
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

    prepare_buffer(vbo, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                   static_cast<GLsizeiptr>(mesh.interleaved.size() * sizeof(float)),
                   mesh.interleaved.data());

    prepare_buffer(ebo, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
                   static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(unsigned)),
                   mesh.indices.data());

    prepare_vertex_array_attribs(mesh.stride, {floats(3), floats(2)});
}

void SpherePipeline::destroy() {
    destroy_mesh_and_program();
    color_loc = -1;
    ambient_loc = -1;
    emission_loc = -1;
    light_dir_loc = -1;
    texture_offset_loc = -1;
    diffuse_loc = -1;
    night_loc = -1;
    use_diffuse_loc = -1;
    use_night_loc = -1;
    show_graticules_loc = -1;
}

void RingPipeline::create() {
    link_program_from_files(program, "ring.vert", "ring.frag");
    cache_uniforms();
    const MeshData mesh = generate_ring_disc(64);
    index_count = mesh.index_count;
    upload_mesh(mesh);
}

void RingPipeline::cache_uniforms() {
    cache_uniform(program, view_loc, "u_view");
    cache_uniform(program, projection_loc, "u_projection");
    cache_uniform(program, model_loc, "u_model");
    cache_uniform(program, light_dir_loc, "u_light_dir");
    cache_uniform(program, map_loc, "u_map");
    cache_uniform(program, use_map_loc, "u_use_map");
    cache_uniform(program, inner_fraction_loc, "u_inner_fraction");
}

void RingPipeline::upload_mesh(const MeshData& mesh) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    prepare_buffer(vbo, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                   static_cast<GLsizeiptr>(mesh.interleaved.size() * sizeof(float)),
                   mesh.interleaved.data());

    prepare_buffer(ebo, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
                   static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(unsigned)),
                   mesh.indices.data());

    prepare_vertex_array_attribs(mesh.stride, {floats(3), floats(1)});
}

void RingPipeline::destroy() {
    destroy_mesh_and_program();
    light_dir_loc = -1;
    map_loc = -1;
    use_map_loc = -1;
    inner_fraction_loc = -1;
}

} // namespace solar::app
