#pragma once

#include <cstdint>

namespace egypt {

enum class Key : std::uint8_t {
    Unknown,
    Escape,
    Left,
    Right,
    Up,
    Down,
    A,
    D,
    W,
    S,
    Plus,
    Minus,
    Home,
    Space,
    F
};

enum class MouseButton : std::uint8_t {
    Unknown,
    Left,
    Middle,
    Right,
    WheelUp,
    WheelDown
};

} // namespace egypt
