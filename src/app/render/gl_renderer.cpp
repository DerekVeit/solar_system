#include "app/render/gl_renderer.hpp"

#include "app/logging.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

namespace solar::app {

GlRenderer::~GlRenderer() { destroy(); }

void GlRenderer::destroy() {
    sphere_.destroy();
    ring_.destroy();
    line_.destroy();
    sky_.destroy();
    capacity_ = {};
    for (auto item : textures_) {
        glDeleteTextures(1, &item.second);
    }
    textures_.clear();
}

bool GlRenderer::init(const RenderCapacity& capacity) {
    destroy();

    capacity_ = capacity;
    if (capacity.max_spheres == 0) {
        return false;
    }

    try {
        sphere_.create();
        ring_.create();
        line_.create(capacity.max_line_vertices + capacity.max_line_trail_vertices +
                     capacity.max_line_loop_vertices);
        sky_.create();

        glBindVertexArray(0);

        return true;
    } catch (const std::exception& e) {
        solar::app::log("GlRenderer::init failed: {}", e.what());
        destroy();
        return false;
    }
}

void GlRenderer::draw(const DrawBatch& batch, const glm::mat4& view, const glm::mat4& projection) {
    sky_.draw(textures_, view, projection);
    sphere_.draw(batch.spheres, capacity_.max_spheres, textures_, view, projection);
    ring_.draw(batch.rings, textures_, view, projection);
    line_.draw(batch.line_trails, GL_LINE_STRIP, capacity_.max_line_trail_vertices, view,
               projection);
    line_.draw(batch.line_loops, GL_LINE_LOOP, capacity_.max_line_loop_vertices, view, projection);
    line_.draw(batch.lines, GL_LINES, capacity_.max_line_vertices, view, projection);
}

void GlRenderer::upload_texture(const std::string& path, const TextureImage& image,
                                TextureFilter filter) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (filter == TextureFilter::mipmapped) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image.pixels);
    if (filter == TextureFilter::mipmapped) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    if (textures_.contains(path)) {
        glDeleteTextures(1, &textures_[path]);
    }
    textures_[path] = id;
}

void GlRenderer::set_sky(const std::string& texture_path, const glm::mat3& tex_from_ecliptic,
                         float brightness) {
    sky_.set_sky(texture_path, tex_from_ecliptic, brightness);
}

} // namespace solar::app
