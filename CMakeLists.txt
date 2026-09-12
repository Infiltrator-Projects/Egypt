cmake_minimum_required(VERSION 3.20)

project(Egypt VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)

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

find_package(X11 REQUIRED)

add_executable(egypt
    src/egypt.cpp
    src/world.cpp
    src/common/image.cpp
)

target_compile_features(egypt PRIVATE cxx_std_20)
set_target_properties(egypt PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(egypt PRIVATE InfiltratrCommon::Portable X11::X11)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(egypt PRIVATE -Wall -Wextra -Wpedantic)
endif()
