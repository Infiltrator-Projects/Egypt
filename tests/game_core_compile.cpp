#include "game.hpp"

#include <type_traits>

static_assert(std::is_class_v<egypt::Game>);
static_assert(std::is_class_v<egypt::Framebuffer>);
static_assert(std::is_enum_v<egypt::Key>);
static_assert(std::is_enum_v<egypt::MouseButton>);

int main() {
    return 0;
}
