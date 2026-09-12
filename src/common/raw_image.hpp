#pragma once

#include "image.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace egypt::common {

struct IndexedRgbaImage {
    int width = 0;
    int height = 0;
    std::array<Color, 256> palette{};
    std::array<std::uint8_t, 256> alpha{};
    std::vector<std::uint8_t> indices;

    [[nodiscard]] bool valid() const {
        return width > 0 && height > 0 &&
               indices.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    }
};

// Egypt-native 8-bit indexed RGBA artwork.
// File layout: "E8PA", little-endian u16 width/height, 256 RGBA palette
// entries, then width*height palette indices.
IndexedRgbaImage load_e8pa(const std::string& path);

} // namespace egypt::common
