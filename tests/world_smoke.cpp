#include "world.hpp"
#include "isometric.hpp"

#include <cstdlib>
#include <iostream>

namespace {
[[noreturn]] void fail(const char* message) {
    std::cerr << "world smoke test failed: " << message << '\n';
    std::exit(1);
}
}

int main() {
    using namespace egypt;

    World world;

    if (world.population() != 0) fail("initial population must be zero");
    if (world.treasury() != 5000) fail("initial treasury must be 5000");

    int road_x = -1;
    int road_y = -1;
    int house_x = -1;
    int house_y = -1;

    for (int y = 1; y < World::kHeight - 1 && road_x < 0; ++y) {
        for (int x = 1; x < World::kWidth - 2; ++x) {
            if (!world.can_place(Structure::Road, x, y)) continue;
            if (!world.can_place(Structure::House, x + 1, y)) continue;
            road_x = x;
            road_y = y;
            house_x = x + 1;
            house_y = y;
            break;
        }
    }

    if (road_x < 0) fail("could not find valid adjacent road/house tiles");
    if (!world.place(Structure::Road, road_x, road_y)) fail("road placement failed");
    if (!world.place(Structure::House, house_x, house_y)) fail("house placement failed");
    if (!world.has_road_access(house_x, house_y)) fail("house should have road access");
    if (world.treasury() != 4988) fail("placement costs are incorrect");

    for (int i = 0; i < 8; ++i) world.tick();
    if (world.tile(house_x, house_y).population != 8) fail("road-access house did not fill");
    if (world.population() != 8) fail("population total is incorrect");

    if (!world.bulldoze(road_x, road_y)) fail("road bulldoze failed");
    world.tick();
    if (world.tile(house_x, house_y).population != 7) fail("house should decline without road access");

    IsoCamera camera;
    camera.origin_x = 640;
    camera.origin_y = 160;
    camera.pan_x = -120;
    camera.pan_y = 35;
    camera.zoom_percent = 110;

    constexpr int tx = 17;
    constexpr int ty = 9;
    const IsoPoint screen = camera.project(tx, ty);
    int picked_x = -1;
    int picked_y = -1;
    if (!camera.pick(screen.x, screen.y, picked_x, picked_y)) fail("isometric pick failed");
    if (picked_x != tx || picked_y != ty) fail("isometric projection/pick round-trip failed");

    std::cout << "Egypt world smoke test: OK\n";
    return 0;
}
