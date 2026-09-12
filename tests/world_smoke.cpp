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
    if (world.total_food() != 0) fail("initial food stock must be zero");
    if (world.treasury() != 5000) fail("initial treasury must be 5000");

    // A house with a road but no food must remain empty. Roads are access,
    // not an immigration spell.
    if (!world.place(Structure::Road, 20, 10)) fail("initial road placement failed");
    if (!world.place(Structure::House, 21, 10)) fail("initial house placement failed");
    for (int i = 0; i < 12; ++i) world.tick();
    if (world.tile(21, 10).population != 0) fail("house filled without food");

    // Build a compact connected food chain on the west floodplain:
    // farm -> road -> granary -> market -> house.
    constexpr int farm_x = 28;
    constexpr int farm_y = 10;
    if (world.tile(farm_x, farm_y).terrain != Terrain::Floodplain) {
        fail("known test farm tile is not floodplain");
    }

    for (int y = 10; y <= 14; ++y) {
        if (!world.place(Structure::Road, 29, y)) fail("food-chain road placement failed");
    }
    if (!world.place(Structure::Farm, 28, 10)) fail("farm placement failed");
    if (!world.place(Structure::Granary, 30, 11)) fail("granary placement failed");
    if (!world.place(Structure::Market, 30, 12)) fail("market placement failed");
    if (!world.place(Structure::House, 30, 13)) fail("fed house placement failed");

    if (!world.road_connected(28, 10, 30, 11)) fail("farm and granary should share a road network");
    if (!world.road_connected(30, 11, 30, 12)) fail("granary and market should share a road network");
    if (!world.road_connected(30, 12, 30, 13)) fail("market and house should share a road network");

    for (int i = 0; i < 40; ++i) world.tick();

    if (world.total_food() <= 0) fail("food chain produced no stored food");
    if (world.tile(30, 13).population == 0) fail("fed road-access house attracted nobody");
    if (world.population() == 0) fail("city population did not respond to food");

    // Cutting road access must make the house decline even when it had food.
    if (!world.bulldoze(29, 13)) fail("road bulldoze failed");
    const auto before = world.tile(30, 13).population;
    world.tick();
    if (world.tile(30, 13).population >= before) fail("house should decline without road access");

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

    // Lock the Pharaoh-style handedness: +X goes down-left and +Y goes
    // down-right. This prevents an accidental horizontal mirror regression.
    const IsoPoint origin = camera.project(10, 10);
    const IsoPoint plus_x = camera.project(11, 10);
    const IsoPoint plus_y = camera.project(10, 11);
    if (!(plus_x.x < origin.x && plus_x.y > origin.y)) {
        fail("map +X must project down-left");
    }
    if (!(plus_y.x > origin.x && plus_y.y > origin.y)) {
        fail("map +Y must project down-right");
    }

    std::cout << "Egypt world smoke test: OK\n";
    return 0;
}
