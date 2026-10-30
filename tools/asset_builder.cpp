// SPDX-License-Identifier: GPL-3.0-or-later
// Native, deterministic unpacking of Egypt's authored artwork.
// Build-only tool. No Python, subprocesses, or image rendering.
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <lzma.h>
#include <openssl/evp.h>
#include <zlib.h>

namespace fs = std::filesystem;
namespace {
constexpr size_t max_artwork_bytes = 16u * 1024u * 1024u;
constexpr std::string_view b85_alphabet =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_\x60{|}~";

[[noreturn]] void fail(const std::string& reason) { throw std::runtime_error(reason); }

std::string read_ascii(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) fail("Cannot read " + path.string());
    std::string encoded((std::istreambuf_iterator<char>(input)), {});
    std::string clean;
    clean.reserve(encoded.size());
    for (unsigned char c : encoded) {
        if (std::isspace(c)) continue;
        if (c >= 128) fail("Non-ASCII artwork payload in " + path.string());
        clean.push_back(static_cast<char>(c));
    }
    return clean;
}

std::vector<uint8_t> decode_base64(std::string_view text) {
    constexpr std::string_view digits =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (text.size() % 4 != 0) fail("Invalid Base64 length");
    std::vector<uint8_t> result;
    result.reserve(text.size() / 4 * 3);
    for (size_t i = 0; i < text.size(); i += 4) {
        unsigned values[4]{};
        size_t padding = 0;
        for (size_t j = 0; j < 4; ++j) {
            const char c = text[i + j];
            if (c == '=') {
                if (j < 2 || i + 4 != text.size()) fail("Invalid Base64 padding");
                ++padding;
            } else {
                if (padding) fail("Invalid Base64 padding order");
                const size_t position = digits.find(c);
                if (position == std::string_view::npos) fail("Invalid Base64 character");
                values[j] = static_cast<unsigned>(position);
            }
        }
        if (padding > 2) fail("Invalid Base64 padding size");
        if ((padding == 1 && (values[2] & 3u)) ||
            (padding == 2 && (values[1] & 15u))) fail("Invalid Base64 padding bits");
        const uint32_t value = (values[0] << 18) | (values[1] << 12) |
                               (values[2] << 6) | values[3];
        result.push_back(static_cast<uint8_t>(value >> 16));
        if (padding < 2) result.push_back(static_cast<uint8_t>(value >> 8));
        if (padding == 0) result.push_back(static_cast<uint8_t>(value));
    }
    return result;
}

std::vector<uint8_t> decode_base85(std::string_view text) {
    std::array<int, 256> lookup;
    lookup.fill(-1);
    for (size_t i = 0; i < b85_alphabet.size(); ++i)
        lookup[static_cast<unsigned char>(b85_alphabet[i])] = static_cast<int>(i);
    std::vector<uint8_t> result;
    result.reserve((text.size() / 5 + 1) * 4);
    for (size_t offset = 0; offset < text.size(); offset += 5) {
        const size_t count = std::min<size_t>(5, text.size() - offset);
        if (count == 1) fail("Invalid Base85 trailing group");
        uint64_t value = 0;
        for (size_t j = 0; j < 5; ++j) {
            const unsigned char c = static_cast<unsigned char>(j < count ? text[offset + j] : '~');
            if (lookup[c] < 0) fail("Invalid Base85 character");
            value = value * 85u + static_cast<unsigned>(lookup[c]);
        }
        if (value > std::numeric_limits<uint32_t>::max()) fail("Base85 group overflow");
        for (size_t j = 0; j < count - 1; ++j)
            result.push_back(static_cast<uint8_t>(value >> (24 - j * 8)));
    }
    return result;
}

std::vector<uint8_t> inflate_zlib(const std::vector<uint8_t>& encoded) {
    z_stream stream{};
    if (inflateInit(&stream) != Z_OK) fail("Cannot initialise zlib decoder");
    std::vector<uint8_t> result;
    std::array<uint8_t, 16384> buffer{};
    if (encoded.size() > std::numeric_limits<uInt>::max()) {
        inflateEnd(&stream);
        fail("Zlib input too large");
    }
    stream.next_in = const_cast<Bytef*>(encoded.data());
    stream.avail_in = static_cast<uInt>(encoded.size());
    int status = Z_OK;
    do {
        stream.next_out = buffer.data();
        stream.avail_out = static_cast<uInt>(buffer.size());
        status = inflate(&stream, Z_NO_FLUSH);
        const size_t produced = buffer.size() - stream.avail_out;
        if (result.size() + produced > max_artwork_bytes) {
            inflateEnd(&stream);
            fail("Zlib artwork exceeds safety limit");
        }
        result.insert(result.end(), buffer.begin(), buffer.begin() + produced);
        if (status != Z_OK && status != Z_STREAM_END) {
            inflateEnd(&stream);
            fail("Malformed zlib artwork stream");
        }
        if (status == Z_OK && produced == 0 && stream.avail_in == 0) {
            inflateEnd(&stream);
            fail("Truncated zlib artwork stream");
        }
    } while (status != Z_STREAM_END);
    const bool has_trailing = stream.avail_in != 0;
    inflateEnd(&stream);
    if (has_trailing) fail("Unexpected trailing zlib bytes");
    return result;
}

std::vector<uint8_t> inflate_xz(const std::vector<uint8_t>& encoded) {
    lzma_stream stream = LZMA_STREAM_INIT;
    if (lzma_stream_decoder(&stream, 128ull * 1024 * 1024, LZMA_CONCATENATED) != LZMA_OK)
        fail("Cannot initialise XZ decoder");
    std::vector<uint8_t> result;
    std::array<uint8_t, 16384> buffer{};
    stream.next_in = encoded.data();
    stream.avail_in = encoded.size();
    lzma_ret status = LZMA_OK;
    do {
        stream.next_out = buffer.data();
        stream.avail_out = buffer.size();
        status = lzma_code(&stream, LZMA_FINISH);
        const size_t produced = buffer.size() - stream.avail_out;
        if (result.size() + produced > max_artwork_bytes) {
            lzma_end(&stream);
            fail("XZ artwork exceeds safety limit");
        }
        result.insert(result.end(), buffer.begin(), buffer.begin() + produced);
        if (status != LZMA_OK && status != LZMA_STREAM_END) {
            lzma_end(&stream);
            fail("Malformed or truncated XZ artwork stream");
        }
        if (status == LZMA_OK && produced == 0 && stream.avail_in == 0) {
            lzma_end(&stream);
            fail("Truncated XZ artwork stream");
        }
    } while (status != LZMA_STREAM_END);
    lzma_end(&stream);
    return result;
}

std::string sha256(const std::vector<uint8_t>& payload) {
    unsigned char digest[EVP_MAX_MD_SIZE]{};
    unsigned int length = 0;
    if (EVP_Digest(payload.data(), payload.size(), digest, &length, EVP_sha256(), nullptr) != 1 || length != 32)
        fail("SHA-256 calculation failed");
    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (unsigned i = 0; i < length; ++i) output << std::setw(2) << static_cast<unsigned>(digest[i]);
    return output.str();
}

void write_payload(const fs::path& destination, const std::vector<uint8_t>& payload) {
    fs::create_directories(destination.parent_path());
    // Write to a sibling first to avoid replacing valid art with a partial file.
    const fs::path staging = destination.string() + ".tmp";
    try {
        std::ofstream output(staging, std::ios::binary | std::ios::trunc);
        if (!output) fail("Cannot create " + staging.string());
        output.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
        output.close();
        if (!output) fail("Could not write complete artwork " + staging.string());
        fs::rename(staging, destination);
    } catch (...) {
        std::error_code ignored;
        fs::remove(staging, ignored);
        throw;
    }
}

void menu(const fs::path& root, const fs::path& destination) {
    std::vector<fs::path> parts;
    for (const auto& item : fs::directory_iterator(root / "assets")) {
        const std::string filename = item.path().filename().string();
        if (item.is_regular_file() && filename.starts_with("menu_q20_xz_") &&
            filename.ends_with(".b64")) parts.push_back(item.path());
    }
    std::sort(parts.begin(), parts.end());
    if (parts.size() != 29) fail("Expected 29 authored menu parts; found " + std::to_string(parts.size()));
    std::string encoded;
    for (const fs::path& part : parts) encoded += read_ascii(part);
    const auto payload = inflate_xz(decode_base64(encoded));
    if (payload.size() != 35948) fail("Authored menu artwork size mismatch");
    if (payload.size() < 4 || std::string_view(reinterpret_cast<const char*>(payload.data()), 4) != "EJ8A")
        fail("Authored menu artwork signature mismatch");
    if (sha256(payload) != "e22ea3d3689495f83b6bd31437806ff897fff530f3d8420579f142bd3a1337c9")
        fail("Authored menu artwork SHA-256 mismatch");
    write_payload(destination, payload);
}

void hud(const fs::path& root, const fs::path& directory) {
    constexpr std::array<std::pair<std::string_view, std::string_view>, 3> assets{{
        {"hud_chrome_authored.e8p.b85", "hud_chrome.e8p"},
        {"hud_glyphs_authored.e8p.b85", "hud_glyphs.e8p"},
        {"tool_icons_authored.e8p.b85", "tool_icons.e8p"},
    }};
    for (const auto& [source, target] : assets) {
        const auto payload = inflate_zlib(decode_base85(read_ascii(root / "assets" / source)));
        if (payload.size() < 4 || std::string_view(reinterpret_cast<const char*>(payload.data()), 4) != "E8PA")
            fail("Authored HUD artwork signature mismatch: " + std::string(source));
        write_payload(directory / target, payload);
    }
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 4) fail("Usage: egypt-asset-builder <menu|hud> <source-repo-root> <destination-path>");
        const std::string mode(argv[1]);
        if (mode == "menu") menu(argv[2], argv[3]);
        else if (mode == "hud") hud(argv[2], argv[3]);
        else fail("Unknown artwork mode: " + mode);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Egypt artwork builder: " << e.what() << '\n';
        return 1;
    }
}
