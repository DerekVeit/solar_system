#include "app/render/gl_pipeline.hpp"

#include "app/files.hpp"
#include "app/render/gl_shader.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

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

void GlProgram::set_camera_uniforms(const glm::mat4& view, const glm::mat4& projection) const {
    glUseProgram(program);
    if (view_loc >= 0) {
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    }
    if (projection_loc >= 0) {
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

void StreamPipeline::create(int vertex_capacity) {
    capacity = vertex_capacity;
    link_program_from_files(program, "line.vert", "line.frag");
    cache_uniforms();
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    prepare_buffer(vbo, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW,
                   static_cast<GLsizeiptr>(capacity * sizeof(LineVertex)), nullptr);
    prepare_vertex_array_attribs(sizeof(LineVertex), {floats(3), floats(4)});
}

void StreamPipeline::cache_uniforms() {
    cache_uniform(program, view_loc, "u_view");
    cache_uniform(program, projection_loc, "u_projection");
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

void SpherePipeline::draw(std::span<const SphereInstance> spheres, std::size_t max_spheres,
                          std::unordered_map<std::string, unsigned int>& textures,
                          const glm::mat4& view, const glm::mat4& projection) {
    if (program == 0 || vao == 0 || index_count <= 0) {
        return;
    }

    const std::size_t count = spheres.size() < max_spheres ? spheres.size() : max_spheres;
    if (count == 0) {
        return;
    }

    set_camera_uniforms(view, projection);
    glBindVertexArray(vao);

    for (std::size_t i = 0; i < count; ++i) {
        const SphereInstance& sphere = spheres[i];
        if (sphere.radius_km <= 0.0f) {
            continue;
        }

        const BodySurface& surface = sphere.surface;

        const glm::mat4 T =
            glm::translate(glm::mat4{1.0f}, glm::vec3{sphere.x_km, sphere.y_km, sphere.z_km});
        const glm::mat4 R = glm::mat4(sphere.rotation);
        const glm::mat4 S = glm::scale(glm::mat4{1.0f}, glm::vec3{sphere.radius_km});
        const glm::mat4 model = T * R * S;

        if (model_loc >= 0) {
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        }

        if (texture_offset_loc >= 0) {
            glUniform1f(texture_offset_loc, surface.textures.longitude_offset_deg);
        }

        const bool using_diffuse_texture =
            (!surface.textures.diffuse.empty() && textures.contains(surface.textures.diffuse) &&
             diffuse_loc >= 0 && use_diffuse_loc >= 0);

        const bool using_night_texture =
            (!surface.textures.night.empty() && textures.contains(surface.textures.night) &&
             night_loc >= 0 && use_night_loc >= 0);

        if (color_loc >= 0) {
            glUniform4f(color_loc, surface.color.r, surface.color.g, surface.color.b,
                        surface.color.a);
        }

        if (using_diffuse_texture) {
            const auto diffuse_id = textures[surface.textures.diffuse];
            glUniform1i(use_diffuse_loc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_id);
            glUniform1i(diffuse_loc, 0);
        } else {
            glUniform1i(use_diffuse_loc, 0);
        }

        if (using_night_texture) {
            const auto night_id = textures[surface.textures.night];
            glUniform1i(use_night_loc, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, night_id);
            glUniform1i(night_loc, 1);
        } else {
            glUniform1i(use_night_loc, 0);
        }

        if (show_graticules_loc >= 0) {
            glUniform1i(show_graticules_loc, static_cast<int>(sphere.show_graticules));
        }

        if (ambient_loc >= 0) {
            glUniform1f(ambient_loc, surface.ambient);
        }
        if (emission_loc >= 0) {
            glUniform1f(emission_loc, surface.emission);
        }
        if (light_dir_loc >= 0) {
            glUniform3fv(light_dir_loc, 1, glm::value_ptr(sphere.light_dir));
        }
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    }
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

void RingPipeline::draw(std::span<const RingInstance> rings,
                        std::unordered_map<std::string, unsigned int>& textures,
                        const glm::mat4& view, const glm::mat4& projection) {
    if (program == 0 || vao == 0 || index_count <= 0) {
        return;
    }
    if (rings.empty()) {
        return;
    }

    set_camera_uniforms(view, projection);
    glBindVertexArray(vao);

    const GLboolean was_blend = glIsEnabled(GL_BLEND);
    const GLboolean was_cull = glIsEnabled(GL_CULL_FACE);
    GLboolean depth_mask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    for (const RingInstance& ring : rings) {
        if (ring.outer_radius_km <= 0.0f) {
            continue;
        }

        const glm::mat4 T =
            glm::translate(glm::mat4{1.0f}, glm::vec3{ring.x_km, ring.y_km, ring.z_km});
        const glm::mat4 R = glm::mat4(ring.rotation);
        const float s = ring.outer_radius_km;
        const glm::mat4 S = glm::scale(glm::mat4{1.0f}, glm::vec3{s, s, s});
        const glm::mat4 model = T * R * S;

        if (model_loc >= 0) {
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if (light_dir_loc >= 0) {
            glUniform3fv(light_dir_loc, 1, glm::value_ptr(ring.light_dir));
        }

        const bool using_map = !ring.map.empty() && textures.contains(ring.map) && map_loc >= 0;
        if (using_map) {
            glUniform1i(use_map_loc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[ring.map]);
            glUniform1i(map_loc, 0);
        } else if (use_map_loc >= 0) {
            glUniform1i(use_map_loc, 0);
        }

        if (inner_fraction_loc >= 0) {
            glUniform1f(inner_fraction_loc,
                        static_cast<float>(ring.inner_radius_km / ring.outer_radius_km));
        }

        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    }

    glDepthMask(depth_mask);
    if (!was_blend) {
        glDisable(GL_BLEND);
    }
    if (was_cull) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void RingPipeline::destroy() {
    destroy_mesh_and_program();
    light_dir_loc = -1;
    map_loc = -1;
    use_map_loc = -1;
    inner_fraction_loc = -1;
}

} // namespace solar::app
