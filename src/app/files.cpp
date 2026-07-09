#include "app/files.hpp"

#include <filesystem>
#include <string>

namespace solar::app {

std::filesystem::path asset_path(const std::string& relative) {
    const std::filesystem::path executable =
        std::filesystem::canonical("/proc/self/exe").parent_path();
    return executable / "assets" / relative;
}

} // namespace solar::app
