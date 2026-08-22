#pragma once

#include "app/render/types.hpp"
#include "app/scene/star_catalog.hpp"

namespace solar::app {

/// Append eye-relative line-loop reticles for catalog stars. `distance_km` is
/// the sphere radius from the camera (inside the far plane).
void append_star_markers(const StarCatalog& catalog, double distance_km, DrawBatch& batch);

} // namespace solar::app
