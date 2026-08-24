#pragma once

#include "app/scene/sky_figure.hpp"
#include "app/scene/star_catalog.hpp"

#include <filesystem>

namespace solar::app {

[[nodiscard]] SkyFigureCatalog load_sky_figures(const std::filesystem::path& path,
                                                const StarCatalog& stars);

} // namespace solar::app
