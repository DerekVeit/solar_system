#pragma once

#include "app/render/types.hpp"
#include "app/scene/sky_figure.hpp"
#include "app/scene/star_catalog.hpp"

namespace solar::app {

/// Append faint stick-figure segments on the eye-relative sky sphere. Each edge
/// stops `line_gap_deg` short of its stars. Reticles and labels are not drawn.
void append_sky_figures(const SkyFigureCatalog& figures, const StarCatalog& stars,
                        double distance_km, DrawBatch& batch);

/// Largest `GL_LINES` vertex count of any one figure (renderer capacity).
[[nodiscard]] std::size_t sky_figure_line_vertex_capacity(const SkyFigureCatalog& figures);

} // namespace solar::app
