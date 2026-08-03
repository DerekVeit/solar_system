#include "app/render/gl_renderer.hpp"

#include "app/logging.hpp"
#include "app/render/gl_shader.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <string_view>

namespace solar::app {

namespace {

constexpr std::string_view kLineVertexShader = R"(#version 460 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 v_color;

void main() {
    gl_Position = u_projection * u_view * vec4(a_pos, 1.0);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                          reinterpret_cast<void*>(offsetof(LineVertex, x_km)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                          reinterpret_cast<void*>(offsetof(LineVertex, color)));
}

} // namespace

GlRenderer::~GlRenderer() { destroy(); }

void GlRenderer::destroy() {
    sphere_.destroy();
    ring_.destroy();
    if (line_program_ != 0) {
        glDeleteProgram(line_program_);
        line_program_ = 0;
    }
    if (line_vbo_ != 0) {
        glDeleteBuffers(1, &line_vbo_);
        line_vbo_ = 0;
    }
    if (line_vao_ != 0) {
        glDeleteVertexArrays(1, &line_vao_);
        line_vao_ = 0;
    }
    line_view_loc_ = -1;
    line_projection_loc_ = -1;
    max_spheres_ = 0;
    max_line_vertices_ = 0;
    max_line_trail_vertices_ = 0;
    max_line_loop_vertices_ = 0;
    for (auto item : textures_) {
        glDeleteTextures(1, &item.second);
    }
    textures_.clear();
}

bool GlRenderer::init(const RenderCapacity& capacity) {
    if (capacity.max_spheres == 0) {
        return false;
    }

    destroy();

    try {
        sphere_.create();
        ring_.create();

        const unsigned int line_vertex_shader = compile_shader(GL_VERTEX_SHADER, kLineVertexShader);
        const unsigned int line_fragment_shader =
            compile_shader(GL_FRAGMENT_SHADER, kLineFragmentShader);
        line_program_ = link_program(line_vertex_shader, line_fragment_shader);
        glDeleteShader(line_vertex_shader);
        glDeleteShader(line_fragment_shader);

        line_view_loc_ = glGetUniformLocation(line_program_, "u_view");
        line_projection_loc_ = glGetUniformLocation(line_program_, "u_projection");

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

        max_spheres_ = capacity.max_spheres;
        max_line_vertices_ = capacity.max_line_vertices;
        max_line_trail_vertices_ = capacity.max_line_trail_vertices;
        max_line_loop_vertices_ = capacity.max_line_loop_vertices;
        return true;
    } catch (const std::exception& e) {
        solar::app::log("GlRenderer::init failed: {}", e.what());
        destroy();
        return false;
    }
}

void GlRenderer::set_camera_uniforms(unsigned int program, int view_loc, int projection_loc,
                                     const glm::mat4& view, const glm::mat4& projection) const {
    glUseProgram(program);
    if (view_loc >= 0) {
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    }
    if (projection_loc >= 0) {
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

void GlRenderer::draw_spheres(std::span<const SphereInstance> spheres, const glm::mat4& view,
                              const glm::mat4& projection) {
    if (sphere_.program == 0 || sphere_.vao == 0 || sphere_.index_count <= 0) {
        return;
    }

    const std::size_t count = spheres.size() < max_spheres_ ? spheres.size() : max_spheres_;
    if (count == 0) {
        return;
    }

    set_camera_uniforms(sphere_.program, sphere_.view_loc, sphere_.projection_loc, view,
                        projection);
    glBindVertexArray(sphere_.vao);

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

        if (sphere_.model_loc >= 0) {
            glUniformMatrix4fv(sphere_.model_loc, 1, GL_FALSE, glm::value_ptr(model));
        }

        if (sphere_.texture_offset_loc >= 0) {
            glUniform1f(sphere_.texture_offset_loc, surface.textures.longitude_offset_deg);
        }

        const bool using_diffuse_texture =
            (!surface.textures.diffuse.empty() && textures_.contains(surface.textures.diffuse) &&
             sphere_.diffuse_loc >= 0 && sphere_.use_diffuse_loc >= 0);

        const bool using_night_texture =
            (!surface.textures.night.empty() && textures_.contains(surface.textures.night) &&
             sphere_.night_loc >= 0 && sphere_.use_night_loc >= 0);

        if (sphere_.color_loc >= 0) {
            glUniform4f(sphere_.color_loc, surface.color.r, surface.color.g, surface.color.b,
                        surface.color.a);
        }

        if (using_diffuse_texture) {
            const auto diffuse_id = textures_[surface.textures.diffuse];
            glUniform1i(sphere_.use_diffuse_loc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_id);
            glUniform1i(sphere_.diffuse_loc, 0);
        } else {
            glUniform1i(sphere_.use_diffuse_loc, 0);
        }

        if (using_night_texture) {
            const auto night_id = textures_[surface.textures.night];
            glUniform1i(sphere_.use_night_loc, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, night_id);
            glUniform1i(sphere_.night_loc, 1);
        } else {
            glUniform1i(sphere_.use_night_loc, 0);
        }

        if (sphere_.show_graticules_loc >= 0) {
            glUniform1i(sphere_.show_graticules_loc, static_cast<int>(sphere.show_graticules));
        }

        if (sphere_.ambient_loc >= 0) {
            glUniform1f(sphere_.ambient_loc, surface.ambient);
        }
        if (sphere_.emission_loc >= 0) {
            glUniform1f(sphere_.emission_loc, surface.emission);
        }
        if (sphere_.light_dir_loc >= 0) {
            glUniform3fv(sphere_.light_dir_loc, 1, glm::value_ptr(sphere.light_dir));
        }
        glDrawElements(GL_TRIANGLES, sphere_.index_count, GL_UNSIGNED_INT, nullptr);
    }
}

void GlRenderer::draw_rings(std::span<const RingInstance> rings, const glm::mat4& view,
                            const glm::mat4& projection) {
    if (ring_.program == 0 || ring_.vao == 0 || ring_.index_count <= 0) {
        return;
    }
    if (rings.empty()) {
        return;
    }

    set_camera_uniforms(ring_.program, ring_.view_loc, ring_.projection_loc, view, projection);
    glBindVertexArray(ring_.vao);

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

        if (ring_.model_loc >= 0) {
            glUniformMatrix4fv(ring_.model_loc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if (ring_.light_dir_loc >= 0) {
            glUniform3fv(ring_.light_dir_loc, 1, glm::value_ptr(ring.light_dir));
        }

        const bool using_map =
            !ring.map.empty() && textures_.contains(ring.map) && ring_.map_loc >= 0;
        if (using_map) {
            glUniform1i(ring_.use_map_loc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures_[ring.map]);
            glUniform1i(ring_.map_loc, 0);
        } else if (ring_.use_map_loc >= 0) {
            glUniform1i(ring_.use_map_loc, 0);
        }

        if (ring_.inner_fraction_loc >= 0) {
            glUniform1f(ring_.inner_fraction_loc,
                        static_cast<float>(ring.inner_radius_km / ring.outer_radius_km));
        }

        glDrawElements(GL_TRIANGLES, ring_.index_count, GL_UNSIGNED_INT, nullptr);
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

void GlRenderer::draw_line_primitives(unsigned int mode, std::span<const LinePrimitive> primitives,
                                      std::size_t max_vertices, const glm::mat4& view,
                                      const glm::mat4& projection) {
    if (line_program_ == 0 || line_vao_ == 0 || line_vbo_ == 0) {
        return;
    }

    set_camera_uniforms(line_program_, line_view_loc_, line_projection_loc_, view, projection);
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

void GlRenderer::draw(const DrawBatch& batch, const glm::mat4& view, const glm::mat4& projection) {
    draw_spheres(batch.spheres, view, projection);
    draw_rings(batch.rings, view, projection);
    draw_line_primitives(GL_LINE_STRIP, batch.line_trails, max_line_trail_vertices_, view,
                         projection);
    draw_line_primitives(GL_LINE_LOOP, batch.line_loops, max_line_loop_vertices_, view, projection);
    draw_line_primitives(GL_LINES, batch.lines, max_line_vertices_, view, projection);
}

void GlRenderer::upload_texture(const std::string& path, const TextureImage& image) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image.pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (textures_.contains(path)) {
        glDeleteTextures(1, &textures_[path]);
    }
    textures_[path] = id;
}

} // namespace solar::app
