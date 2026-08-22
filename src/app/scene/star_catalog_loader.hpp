#pragma once

#include "app/scene/star_catalog.hpp"

#include <filesystem>

namespace solar::app {

[[nodiscard]] StarCatalog load_star_catalog(const std::filesystem::path& path);

} // namespace solar::app
