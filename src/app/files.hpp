#pragma once

#include <filesystem>

namespace solar::app {

std::filesystem::path asset_path(const std::string& relative);

} // namespace solar::app
