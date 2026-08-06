#include "app/render/gl_renderer.hpp"

#include "app/logging.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

#include <cstddef>

namespace solar::app {

GlRenderer::~GlRenderer() { destroy(); }

void GlRenderer::destroy() {
    sphere_.destroy();
    ring_.destroy();
    line_.destroy();
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

        const std::size_t line_buffer_capacity = capacity.max_line_vertices +
                                                 capacity.max_line_trail_vertices +
                                                 capacity.max_line_loop_vertices;
        line_.create(line_buffer_capacity);

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

void GlRenderer::draw_rings(std::span<const RingInstance> rings, const glm::mat4& view,
                            const glm::mat4& projection) {
    if (ring_.program == 0 || ring_.vao == 0 || ring_.index_count <= 0) {
        return;
    }
    if (rings.empty()) {
        return;
    }

    ring_.set_camera_uniforms(view, projection);
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
    if (line_.program == 0 || line_.vao == 0 || line_.vbo == 0) {
        return;
    }

    line_.set_camera_uniforms(view, projection);
    glBindVertexArray(line_.vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_.vbo);

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
    sphere_.draw(batch.spheres, max_spheres_, textures_, view, projection);
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
