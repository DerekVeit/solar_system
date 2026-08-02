#pragma once

#include <filesystem>
#include <string>

namespace solar::app {

std::filesystem::path asset_path(const std::string& relative);

std::string read_text_file(const std::filesystem::path& path);

} // namespace solar::app
