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
    if (world.immigrants_in_transit() != 0) fail("initial immigrant count must be zero");
    if (world.treasury() != 5000) fail("initial treasury must be 5000");

    // Roads alone must never conjure residents into a house.
    if (!world.place(Structure::Road, 20, 10)) fail("initial road placement failed");
    if (!world.place(Structure::House, 21, 10)) fail("initial house placement failed");
    for (int i = 0; i < 12; ++i) world.tick();
    if (world.tile(21, 10).population != 0) fail("house filled without food");

    // Build the local food chain first, but deliberately do NOT connect it to
    // a map edge yet. The house may receive food, but nobody can immigrate.
    for (int y = 10; y <= 14; ++y) {
        if (!world.place(Structure::Road, 29, y)) fail("food-chain road placement failed");
    }
    if (!world.place(Structure::Farm, 28, 10)) fail("farm placement failed");
    if (!world.place(Structure::Granary, 30, 11)) fail("granary placement failed");
    if (!world.place(Structure::Market, 30, 12)) fail("market placement failed");
    if (!world.place(Structure::House, 30, 13)) fail("fed house placement failed");

    for (int i = 0; i < 12; ++i) world.tick();
    if (world.tile(30, 13).food_stock == 0) fail("food did not reach house");
    if (world.tile(30, 13).population != 0) fail("people appeared without a kingdom-road connection");
    if (world.immigrants_in_transit() != 0) fail("immigrants spawned without a map-edge route");

    // Now connect the settlement road to the west edge. This is the kingdom
    // road: immigration should become visible before residents reach houses.
    for (int x = 0; x <= 28; ++x) {
        if (!world.place(Structure::Road, x, 11)) fail("kingdom-road placement failed");
    }

    bool saw_immigrants = false;
    bool saw_population = false;
    for (int i = 0; i < 48; ++i) {
        world.tick();
        if (world.immigrants_in_transit() > 0) saw_immigrants = true;
        if (world.tile(30, 13).population > 0) {
            saw_population = true;
            break;
        }
    }

    if (!saw_immigrants) fail("no visible immigrant traffic was created");
    if (!saw_population) fail("immigrants never reached the fed house");
    if (world.population() == 0) fail("city population did not respond to arriving immigrants");

    // Cutting the target's road access should make residents decline rather
    // than allowing invisible replacement population to appear.
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

    const IsoPoint origin = camera.project(0, 0);
    const IsoPoint plus_x = camera.project(1, 0);
    const IsoPoint plus_y = camera.project(0, 1);
    if (!(plus_x.x < origin.x && plus_x.y > origin.y)) fail("map +X must project down-left in Pharaoh orientation");
    if (!(plus_y.x > origin.x && plus_y.y > origin.y)) fail("map +Y must project down-right in Pharaoh orientation");

    std::cout << "Egypt world smoke test: OK\n";
    return 0;
}
