#pragma once

#include <cstdint>
#include <filesystem>

namespace solar::app {

struct TextureImage {
    int width{0};
    int height{0};
    int channels{0};
    std::uint8_t* pixels{nullptr};

    TextureImage() = default;
    TextureImage(int w, int h, int c, std::uint8_t* p)
        : width(w)
        , height(h)
        , channels(c)
        , pixels(p) {}

    ~TextureImage() { reset(); }

    TextureImage(const TextureImage&) = delete;
    TextureImage& operator=(const TextureImage&) = delete;

    TextureImage(TextureImage&& other) noexcept
        : width(other.width)
        , height(other.height)
        , channels(other.channels)
        , pixels(other.pixels) {
        other.pixels = nullptr;
    }

    TextureImage& operator=(TextureImage&& other) noexcept {
        if (this != &other) {
            reset();
            width = other.width;
            height = other.height;
            channels = other.channels;
            pixels = other.pixels;
            other.pixels = nullptr;
        }
        return *this;
    }

    void reset();

    [[nodiscard]] bool valid() const { return pixels != nullptr && width > 0 && height > 0; }
};

TextureImage image_from_path(std::filesystem::path path);

} // namespace solar::app
