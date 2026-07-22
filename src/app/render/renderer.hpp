#pragma once

#include "app/render/types.hpp"
#include "app/scene/texture.hpp"

#include <glm/mat4x4.hpp>

namespace solar::app {

/// Backend-agnostic draw API for the solar system view.
class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual bool init(const RenderCapacity& capacity) = 0;

    /// Draw camera-relative geometry with view and projection matrices.
    virtual void draw(const DrawBatch& batch, const glm::mat4& view,
                      const glm::mat4& projection) = 0;

    virtual void upload_texture(const std::string& path, const TextureImage& image) = 0;
};

} // namespace solar::app
