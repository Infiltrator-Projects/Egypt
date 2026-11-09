// SPDX-License-Identifier: GPL-3.0-or-later
#include "common/image.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

namespace {
constexpr const char* kFixture = "egypt-image-decoder-test.e16";

void write_length(std::vector<std::uint8_t>& bytes, std::size_t offset,
                  std::uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) {
        bytes[offset + i] = static_cast<std::uint8_t>(value >> (8U * i));
    }
}

bool write_image(const std::vector<std::uint8_t>& y,
                 const std::vector<std::uint8_t>& cb,
                 const std::vector<std::uint8_t>& cr,
                 int width = 8)
{
    std::vector<std::uint8_t> bytes(22U, 0U);
    bytes[0] = 'E'; bytes[1] = 'J'; bytes[2] = '8'; bytes[3] = 'A';
    bytes[4] = static_cast<std::uint8_t>(width);
    bytes[6] = 8U;
    bytes[8] = 50U;
    write_length(bytes, 10U, static_cast<std::uint32_t>(y.size()));
    write_length(bytes, 14U, static_cast<std::uint32_t>(cb.size()));
    write_length(bytes, 18U, static_cast<std::uint32_t>(cr.size()));
    bytes.insert(bytes.end(), y.begin(), y.end());
    bytes.insert(bytes.end(), cb.begin(), cb.end());
    bytes.insert(bytes.end(), cr.begin(), cr.end());
    std::ofstream output(kFixture, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    return output.good();
}

bool valid_image(const std::vector<std::uint8_t>& y,
                 const std::vector<std::uint8_t>& cb,
                 const std::vector<std::uint8_t>& cr,
                 int width = 8)
{
    return write_image(y, cb, cr, width) &&
           egypt::common::load_e16(kFixture).valid();
}
}

int main()
{
    const std::vector<std::uint8_t> zero_block{0x00U, 0xffU};
    const std::vector<std::uint8_t> oversized_varint{
        0x80U, 0x80U, 0x80U, 0x80U, 0x10U, 0xffU
    };
    // 0xfffffffe zigzag-decodes to INT32_MAX. Dequantization must not
    // multiply signed 32-bit integers before converting to floating point.
    const std::vector<std::uint8_t> maximal_dc{
        0xfeU, 0xffU, 0xffU, 0xffU, 0x0fU, 0xffU
    };
    const std::vector<std::uint8_t> two_extreme_blocks{
        0xfeU, 0xffU, 0xffU, 0xffU, 0x0fU, 0xffU,
        0xfeU, 0xffU, 0xffU, 0xffU, 0x0fU, 0xffU
    };

    const bool ok =
        valid_image(zero_block, zero_block, zero_block) &&
        !valid_image(oversized_varint, zero_block, zero_block) &&
        valid_image(maximal_dc, zero_block, zero_block) &&
        valid_image(two_extreme_blocks, zero_block, zero_block, 16);
    std::remove(kFixture);
    if (!ok) {
        std::cerr << "Egypt image decoder rejected a valid image or "
                     "accepted an oversized varint\n";
        return 1;
    }
    std::cout << "Egypt image decoder boundaries: OK\n";
    return 0;
}
