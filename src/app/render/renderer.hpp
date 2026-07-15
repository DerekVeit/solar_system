#pragma once

#include "app/render/types.hpp"

#include <cstddef>
#include <span>

namespace solar::app {

/// Backend-agnostic draw API for the solar system view.
class IRenderer {
  public:
    virtual ~IRenderer() = default;

    /// Prepare GPU resources for up to max_points per frame.
    virtual bool init(std::size_t max_points) = 0;

    virtual void draw_points(std::span<const PointInstance> points) = 0;
};

} // namespace solar::app