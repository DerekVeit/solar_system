#include "app/files.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace solar::app {

std::filesystem::path asset_path(const std::string& relative) {
    const std::filesystem::path executable =
        std::filesystem::canonical("/proc/self/exe").parent_path();
    return executable / "assets" / relative;
}

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream in_file(path);
    if (!in_file) {
        throw std::runtime_error("failed to open " + path.string());
    }

    std::stringstream ss;
    ss << in_file.rdbuf();
    if (!in_file && !in_file.eof()) {
        throw std::runtime_error("failed reading " + path.string());
    }

    return ss.str();
}

} // namespace solar::app
