#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace egypt::common {

struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

struct Image {
    int width = 0;
    int height = 0;
    std::vector<Color> pixels;

    [[nodiscard]] bool valid() const {
        return width > 0 && height > 0 &&
               pixels.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    }
};

// Egypt-native E16A image format.
// Header: "E16A", little-endian uint16 width, uint16 height.
// Stream: 2-bit opcode + compact RGB565 payload/delta/run/index operations.
Image load_e16(const std::string& path);

} // namespace egypt::common
