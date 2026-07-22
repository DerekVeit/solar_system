#include "app/scene/texture.hpp"

#include "app/files.hpp"

#include <stb/stb_image.h>

namespace solar::app {

void TextureImage::reset() {
    if (pixels != nullptr) {
        stbi_image_free(pixels);
        pixels = nullptr;
    }
    width = height = channels = 0;
}

TextureImage image_from_path(std::filesystem::path path) {
    if (path.is_relative()) {
        path = asset_path(path);
    }

    int width = 0, height = 0, channels_in_file = 0;
    auto* pixels = stbi_load(path.c_str(), &width, &height, &channels_in_file, 4);
    if (pixels == nullptr) {
        return {};
    }
    return TextureImage{width, height, 4, pixels};
}

} // namespace solar::app
