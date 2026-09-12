#include "game.hpp"
#include "platform.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        egypt::Framebuffer framebuffer(1280, 720);
        egypt::Game game(framebuffer);
        return egypt::platform::run(game, framebuffer);
    } catch (const std::exception& error) {
        std::cerr << "Egypt failed to start: " << error.what() << '\n';
        return 1;
    }
}
