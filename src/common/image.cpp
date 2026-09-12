#include "image.hpp"

#include <array>
#include <fstream>
#include <iterator>

namespace egypt::common {
namespace {

std::uint16_t read_le16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
}

std::size_t cache_index(std::uint16_t value) {
    const unsigned r = (value >> 11U) & 31U;
    const unsigned g = (value >> 5U) & 63U;
    const unsigned b = value & 31U;
    return (r * 3U + g * 5U + b * 7U) & 63U;
}

Color expand_rgb565(std::uint16_t value) {
    const std::uint8_t r5 = static_cast<std::uint8_t>((value >> 11U) & 31U);
    const std::uint8_t g6 = static_cast<std::uint8_t>((value >> 5U) & 63U);
    const std::uint8_t b5 = static_cast<std::uint8_t>(value & 31U);
    return {
        static_cast<std::uint8_t>((r5 << 3U) | (r5 >> 2U)),
        static_cast<std::uint8_t>((g6 << 2U) | (g6 >> 4U)),
        static_cast<std::uint8_t>((b5 << 3U) | (b5 >> 2U))
    };
}

} // namespace

Image load_e16(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return {};

    std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    if (bytes.size() < 8 ||
        bytes[0] != 'E' || bytes[1] != '1' || bytes[2] != '6' || bytes[3] != 'A') {
        return {};
    }

    const int width = read_le16(bytes, 4);
    const int height = read_le16(bytes, 6);
    if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return {};

    const std::size_t pixel_count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (pixel_count > 64U * 1024U * 1024U) return {};

    std::array<std::uint16_t, 64> cache{};
    std::uint16_t previous = 0;
    std::size_t pos = 8;

    Image image;
    image.width = width;
    image.height = height;
    image.pixels.reserve(pixel_count);

    while (image.pixels.size() < pixel_count && pos < bytes.size()) {
        const std::uint8_t op = bytes[pos++];
        const std::uint8_t tag = op >> 6U;

        if (tag == 0U) {
            previous = cache[op & 63U];
            image.pixels.push_back(expand_rgb565(previous));
            continue;
        }

        if (tag == 1U) {
            const int pr = (previous >> 11U) & 31U;
            const int pg = (previous >> 5U) & 63U;
            const int pb = previous & 31U;
            const int dr = static_cast<int>((op >> 4U) & 3U) - 2;
            const int dg = static_cast<int>((op >> 2U) & 3U) - 2;
            const int db = static_cast<int>(op & 3U) - 2;
            const int r = pr + dr;
            const int g = pg + dg;
            const int b = pb + db;
            if (r < 0 || r > 31 || g < 0 || g > 63 || b < 0 || b > 31) return {};
            previous = static_cast<std::uint16_t>((r << 11U) | (g << 5U) | b);
            cache[cache_index(previous)] = previous;
            image.pixels.push_back(expand_rgb565(previous));
            continue;
        }

        if (tag == 2U) {
            const std::size_t run = static_cast<std::size_t>(op & 63U) + 1U;
            if (image.pixels.size() + run > pixel_count) return {};
            const Color color = expand_rgb565(previous);
            image.pixels.insert(image.pixels.end(), run, color);
            continue;
        }

        if (pos + 1U >= bytes.size()) return {};
        previous = static_cast<std::uint16_t>(bytes[pos]) |
                   static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[pos + 1U]) << 8U);
        pos += 2U;
        cache[cache_index(previous)] = previous;
        image.pixels.push_back(expand_rgb565(previous));
    }

    if (image.pixels.size() != pixel_count) return {};
    return image;
}

} // namespace egypt::common
