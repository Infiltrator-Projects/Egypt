#include "raw_image.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

namespace egypt::common {
namespace {

std::uint16_t read_le16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           static_cast<std::uint16_t>(bytes[offset + 1] << 8U);
}

} // namespace

IndexedRgbaImage load_e8pa(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};

    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), {});
    constexpr std::size_t header_size = 8;
    constexpr std::size_t palette_bytes = 256 * 4;
    if (bytes.size() < header_size + palette_bytes ||
        bytes[0] != 'E' || bytes[1] != '8' || bytes[2] != 'P' || bytes[3] != 'A') {
        return {};
    }

    const int width = read_le16(bytes, 4);
    const int height = read_le16(bytes, 6);
    if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return {};

    const std::size_t pixel_count =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    const std::size_t expected = header_size + palette_bytes + pixel_count;
    if (bytes.size() != expected) return {};

    IndexedRgbaImage image;
    image.width = width;
    image.height = height;

    std::size_t offset = header_size;
    for (std::size_t i = 0; i < 256; ++i) {
        image.palette[i] = {bytes[offset], bytes[offset + 1], bytes[offset + 2]};
        image.alpha[i] = bytes[offset + 3];
        offset += 4;
    }

    image.indices.assign(bytes.begin() + static_cast<std::ptrdiff_t>(offset), bytes.end());
    return image;
}

} // namespace egypt::common
