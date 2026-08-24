#pragma once

#include "app/render/types.hpp"
#include "app/scene/star_catalog.hpp"

#include <glm/ext/vector_double3.hpp>

namespace solar::app {

/// Append eye-relative line-loop reticles and stroke-font name labels for catalog
/// stars. `distance_km` is the sphere radius from the camera (inside the far plane).
/// `view_right` / `view_up` billboard the labels on the sky.
void append_star_markers(const StarCatalog& catalog, double distance_km,
                         const glm::dvec3& view_right, const glm::dvec3& view_up, DrawBatch& batch);

/// Largest `GL_LINES` vertex count of any one star label (renderer capacity).
[[nodiscard]] std::size_t star_marker_line_vertex_capacity(const StarCatalog& catalog);

} // namespace solar::app
