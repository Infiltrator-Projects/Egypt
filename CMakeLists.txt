# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.20)

project(Egypt VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
include(FetchContent)
include(GNUInstallDirs)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(EGYPT_BUILD_PROFILE "generic" CACHE STRING
    "Build profile: generic for distributable packages, native for this machine")
set_property(CACHE EGYPT_BUILD_PROFILE PROPERTY STRINGS generic native)
if(EGYPT_BUILD_PROFILE STREQUAL "native")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" OR
       CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-march=native -mtune=native)
    endif()
elseif(NOT EGYPT_BUILD_PROFILE STREQUAL "generic")
    message(FATAL_ERROR "EGYPT_BUILD_PROFILE must be 'generic' or 'native'")
endif()

# Use the canonical shared Infiltratr Common release. Pin the exact reviewed
# Common 1.16.0 commit so Egypt builds reproducibly and cannot silently drift.
set(INFILTRATR_COMMON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(INFILTRATR_COMMON_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(INFILTRATR_COMMON_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    InfiltratrCommon
    GIT_REPOSITORY https://github.com/Infiltrator-Projects/Infiltrator-Libraries.git
    GIT_TAG a9db06b11f493c4e6f42bf6c13cc5cc5c73e1fc4
)
FetchContent_MakeAvailable(InfiltratrCommon)

# Authored runtime artwork is reconstructed deterministically as part of the
# native build. The installed application owns these files under its data
# directory; it never depends on a repository-relative build/ path.
set(EGYPT_ASSET_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")
file(GLOB EGYPT_MENU_PARTS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/menu_q20_xz_*.b64")
set(EGYPT_HUD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/hud_chrome_authored.e8p.b85"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/hud_glyphs_authored.e8p.b85"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/tool_icons_authored.e8p.b85")

add_custom_command(
    OUTPUT "${EGYPT_ASSET_DIR}/menu.e16"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${EGYPT_ASSET_DIR}"
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_menu_asset.py"
            "${EGYPT_ASSET_DIR}/menu.e16"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_menu_asset.py"
        ${EGYPT_MENU_PARTS}
    VERBATIM)

add_custom_command(
    OUTPUT
        "${EGYPT_ASSET_DIR}/hud_chrome.e8p"
        "${EGYPT_ASSET_DIR}/hud_glyphs.e8p"
        "${EGYPT_ASSET_DIR}/tool_icons.e8p"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${EGYPT_ASSET_DIR}"
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_hud_assets.py"
            "${EGYPT_ASSET_DIR}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_hud_assets.py"
        ${EGYPT_HUD_SOURCES}
    VERBATIM)

add_custom_target(egypt-runtime-assets
    DEPENDS
        "${EGYPT_ASSET_DIR}/menu.e16"
        "${EGYPT_ASSET_DIR}/hud_chrome.e8p"
        "${EGYPT_ASSET_DIR}/hud_glyphs.e8p"
        "${EGYPT_ASSET_DIR}/tool_icons.e8p")

# Egypt itself is platform-neutral. A host backend is selected only for the
# outer executable shell. Set EGYPT_HOST_BACKEND=none to build/test the game
# core with no window-system dependency at all.
set(EGYPT_HOST_BACKEND "x11" CACHE STRING "Host adapter: x11 or none")
set_property(CACHE EGYPT_HOST_BACKEND PROPERTY STRINGS x11 none)

if(EGYPT_HOST_BACKEND STREQUAL "x11")
    find_package(X11 REQUIRED)
    add_executable(egypt
        src/main.cpp
        src/platform/x11_backend.cpp
        src/world.cpp
        src/common/image.cpp
        src/common/raw_image.cpp
    )
    add_dependencies(egypt egypt-runtime-assets)
    target_include_directories(egypt PRIVATE src)
    target_compile_features(egypt PRIVATE cxx_std_20)
    target_compile_definitions(egypt PRIVATE
        EGYPT_BUILD_ASSET_DIR="${EGYPT_ASSET_DIR}"
        EGYPT_INSTALL_ASSET_DIR="${CMAKE_INSTALL_FULL_DATADIR}/egypt/assets")
    set_target_properties(egypt PROPERTIES CXX_EXTENSIONS OFF)
    target_link_libraries(egypt PRIVATE InfiltratrCommon::Portable X11::X11)

    install(TARGETS egypt
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT Runtime)
    install(FILES
        "${EGYPT_ASSET_DIR}/menu.e16"
        "${EGYPT_ASSET_DIR}/hud_chrome.e8p"
        "${EGYPT_ASSET_DIR}/hud_glyphs.e8p"
        "${EGYPT_ASSET_DIR}/tool_icons.e8p"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/egypt/assets"
        COMPONENT Runtime)
    install(FILES README.md
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/egypt"
        COMPONENT Runtime)
elseif(EGYPT_HOST_BACKEND STREQUAL "none")
    message(STATUS "Egypt host backend disabled: building platform-neutral core/tests only")
else()
    message(FATAL_ERROR "Unsupported EGYPT_HOST_BACKEND='${EGYPT_HOST_BACKEND}'")
endif()

# This target deliberately has no X11 include directories or libraries. It is
# a compile-time guard against platform APIs leaking back into Game/UI code.
add_executable(egypt-game-core-compile tests/game_core_compile.cpp)
target_include_directories(egypt-game-core-compile PRIVATE src)
target_compile_features(egypt-game-core-compile PRIVATE cxx_std_20)
set_target_properties(egypt-game-core-compile PROPERTIES CXX_EXTENSIONS OFF)

if(BUILD_TESTING)
    add_executable(egypt-world-smoke
        tests/world_smoke.cpp
        src/world.cpp
    )
    target_include_directories(egypt-world-smoke PRIVATE src)
    target_compile_features(egypt-world-smoke PRIVATE cxx_std_20)
    set_target_properties(egypt-world-smoke PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME egypt-world-smoke COMMAND egypt-world-smoke)
    add_test(NAME egypt-game-core-compile COMMAND egypt-game-core-compile)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    if(TARGET egypt)
        target_compile_options(egypt PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    target_compile_options(egypt-game-core-compile PRIVATE -Wall -Wextra -Wpedantic)
    if(BUILD_TESTING)
        target_compile_options(egypt-world-smoke PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
