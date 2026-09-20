#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

#ifndef EGYPT_BUILD_ASSET_DIR
#define EGYPT_BUILD_ASSET_DIR "build/assets"
#endif
#ifndef EGYPT_INSTALL_ASSET_DIR
#define EGYPT_INSTALL_ASSET_DIR "/usr/local/share/egypt/assets"
#endif

namespace egypt::common {

inline std::string runtime_asset_path(const char* filename) {
    if (const char* override_dir = std::getenv("EGYPT_ASSET_DIR");
        override_dir != nullptr && *override_dir != '\0') {
        return (std::filesystem::path(override_dir) / filename).string();
    }

    const std::filesystem::path build_path =
        std::filesystem::path(EGYPT_BUILD_ASSET_DIR) / filename;
    if (std::filesystem::exists(build_path)) {
        return build_path.string();
    }

    return (std::filesystem::path(EGYPT_INSTALL_ASSET_DIR) / filename).string();
}

} // namespace egypt::common
