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

// Egypt-native EJ8A menu-image decoder. The public name is retained for the
// current renderer call site while the bootstrap code is being consolidated.
Image load_e16(const std::string& path);

} // namespace egypt::common
