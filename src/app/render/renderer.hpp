#pragma once

#include "app/render/types.hpp"

namespace solar::app {

/// Backend-agnostic draw API for the solar system view.
class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual bool init(const RenderCapacity& capacity) = 0;

    virtual void draw(const DrawBatch& batch) = 0;
};

} // namespace solar::app