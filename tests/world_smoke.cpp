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
    if (world.total_clay() != 0) fail("initial clay stock must be zero");
    if (world.total_pottery() != 0) fail("initial pottery stock must be zero");
    if (world.month_index() != 0 || world.year_bc() != 3500) fail("initial date incorrect");

    if (!world.place(Structure::Road, 20, 10)) fail("isolated road placement failed");
    if (!world.place(Structure::House, 21, 10)) fail("isolated house placement failed");
    for (int i = 0; i < 20; ++i) world.tick();
    if (world.tile(21, 10).population != 0) fail("isolated housing attracted settlers");

    for (int x = 0; x <= 29; ++x) {
        if (!world.place(Structure::Road, x, 11)) fail("kingdom road placement failed");
    }
    for (int y = 10; y <= 14; ++y) {
        if (y == 11) continue;
        if (!world.place(Structure::Road, 29, y)) fail("district road placement failed");
    }
    if (!world.place(Structure::House, 30, 13)) fail("district house placement failed");

    bool saw_immigrants = false;
    bool saw_bootstrap_population = false;
    for (int i = 0; i < 80; ++i) {
        world.tick();
        if (world.immigrants_in_transit() > 0) saw_immigrants = true;
        if (world.tile(30, 13).population > 0) saw_bootstrap_population = true;
    }
    if (!saw_immigrants) fail("no visible bootstrap immigration");
    if (!saw_bootstrap_population) fail("settlers never reached unsupplied housing");
    if (world.tile(30, 13).population > 4) fail("unsupplied house exceeded bootstrap attraction cap");

    if (!world.place(Structure::Farm, 28, 10)) fail("farm placement failed");
    if (!world.place(Structure::Granary, 30, 11)) fail("granary placement failed");
    if (!world.place(Structure::Market, 30, 12)) fail("market placement failed");

    bool saw_farmer = false;
    bool saw_granary_worker = false;
    bool saw_market_worker = false;
    bool saw_food_cart = false;
    bool house_received_food = false;
    for (int i = 0; i < 220; ++i) {
        world.tick();
        for (const auto& worker : world.workers()) {
            if (worker.role == WorkerRole::Farmer) saw_farmer = true;
            if (worker.role == WorkerRole::GranaryWorker) saw_granary_worker = true;
            if (worker.role == WorkerRole::MarketWorker) saw_market_worker = true;
        }
        for (const auto& goods : world.goods_agents()) if (goods.resource == Resource::Food) saw_food_cart = true;
        if (world.tile(30, 13).food_stock > 0) house_received_food = true;
    }
    if (!saw_farmer || !saw_granary_worker || !saw_market_worker) fail("food chain was not staffed by residents");
    if (!saw_food_cart) fail("food never moved as a physical logistics agent");
    if (!house_received_food) fail("physical food chain never reached housing");
    if (world.employed_population() < 3) fail("food jobs were not tied to resident employment");

    const int base_capacity = world.house_capacity(30, 13);
    if (base_capacity != 8) fail("base house capacity must be 8");
    if (!world.place(Structure::Well, 28, 14)) fail("well placement failed");
    bool housing_evolved = false;
    bool population_grew = false;
    const int bootstrap_population = world.tile(30, 13).population;
    for (int i = 0; i < 120; ++i) {
        world.tick();
        if (world.tile(30, 13).housing_level >= 1) housing_evolved = true;
        if (world.tile(30, 13).population > bootstrap_population) population_grew = true;
    }
    if (!world.has_well_service(30, 13)) fail("well did not serve house");
    if (!housing_evolved) {
        const Tile& h = world.tile(30, 13);
        std::cerr << "housing debug: level=" << int(h.housing_level)
                  << " service_ticks=" << int(h.housing_service_ticks)
                  << " food=" << h.food_stock
                  << " pop=" << int(h.population)
                  << " employed=" << int(h.employed)
                  << " total_food=" << world.total_food()
                  << " farm=" << world.workers_assigned(28,10) << "/" << world.workers_active(28,10)
                  << " granary=" << world.workers_assigned(30,11) << "/" << world.workers_active(30,11)
                  << " market=" << world.workers_assigned(30,12) << "/" << world.workers_active(30,12)
                  << " status=" << world.house_evolution_status(30,13) << '\n';
        fail("house did not evolve with sustained food and water");
    }
    if (world.house_capacity(30, 13) <= base_capacity) fail("housing evolution did not raise capacity");
    if (!population_grew) fail("new housing capacity did not attract more settlers");

    if (!world.place(Structure::Road, 29, 9)) fail("industry connector road failed");
    if (!world.place(Structure::ClayPit, 27, 10)) fail("clay pit placement failed");
    if (!world.place(Structure::Potter, 28, 9)) fail("potter placement failed");

    bool saw_clay_worker = false;
    bool saw_potter = false;
    bool saw_clay_cart = false;
    bool saw_pottery_cart = false;
    bool house_received_pottery = false;
    bool house_reached_goods_level = false;
    for (int i = 0; i < 260; ++i) {
        world.tick();
        for (const auto& worker : world.workers()) {
            if (worker.role == WorkerRole::ClayWorker) saw_clay_worker = true;
            if (worker.role == WorkerRole::Potter) saw_potter = true;
        }
        for (const auto& goods : world.goods_agents()) {
            if (goods.resource == Resource::Clay) saw_clay_cart = true;
            if (goods.resource == Resource::Pottery) saw_pottery_cart = true;
        }
        if (world.tile(30, 13).pottery_stock > 0) house_received_pottery = true;
        if (world.tile(30, 13).housing_level >= 3) house_reached_goods_level = true;
    }
    if (!saw_clay_worker || !saw_potter) fail("industry was not staffed by residents");
    if (!saw_clay_cart) fail("clay never moved as a physical logistics agent");
    if (!saw_pottery_cart) fail("pottery never moved as a physical logistics agent");
    if (!house_received_pottery) fail("pottery never reached the house");
    if (!house_reached_goods_level) fail("pottery did not unlock the goods housing level");

    if (!world.place(Structure::HuntingLodge, 28, 12)) fail("hunting lodge placement failed");
    bool saw_hunter = false;
    bool saw_hunter_role_state = false;
    bool saw_hunting_food = false;
    for (int i = 0; i < 180; ++i) {
        world.tick();
        for (const auto& worker : world.workers()) {
            if (worker.role == WorkerRole::Hunter) saw_hunter = true;
            if (worker.role == WorkerRole::Hunter &&
                (worker.state == WorkerState::HunterOutbound || worker.state == WorkerState::Hunting ||
                 worker.state == WorkerState::HunterReturning)) saw_hunter_role_state = true;
        }
        if (world.tile(28, 12).food_stock > 0) saw_hunting_food = true;
    }
    if (!saw_hunter) fail("hunting lodge recruited no resident worker");
    if (!saw_hunter_role_state) fail("hunter never entered specialist field work");
    if (!saw_hunting_food && world.total_food() == 0) fail("hunting produced no food");

    bool saw_used_road = false;
    for (int y = 0; y < World::kHeight; ++y) {
        for (int x = 0; x < World::kWidth; ++x) {
            if (world.tile(x, y).structure == Structure::Road && world.road_level(x, y) >= 1) saw_used_road = true;
        }
    }
    if (!saw_used_road) fail("real traffic never evolved a road");

    if (world.worker_capacity(28, 10) != 1) fail("farm staffing capacity incorrect");
    if (world.workers_assigned(28, 10) == 0) fail("farm lost its household worker assignment");
    const char* status = world.house_evolution_status(30, 13);
    if (status == nullptr || *status == '\0') fail("house diagnostic status missing");

    for (int i = 0; i < 600; ++i) world.tick();
    if (world.year_bc() >= 3500) fail("simulation calendar did not advance years");

    IsoCamera camera;
    const IsoPoint origin = camera.project(0, 0);
    const IsoPoint plus_x = camera.project(1, 0);
    const IsoPoint plus_y = camera.project(0, 1);
    if (!(plus_x.x < origin.x && plus_x.y > origin.y)) fail("map +X must project down-left");
    if (!(plus_y.x > origin.x && plus_y.y > origin.y)) fail("map +Y must project down-right");

    int picked_x = -1, picked_y = -1;
    const IsoPoint point = camera.project(17, 9);
    if (!camera.pick(point.x, point.y, picked_x, picked_y)) fail("isometric pick failed");
    if (picked_x != 17 || picked_y != 9) fail("projection/pick round-trip failed");

    std::cout << "Egypt world smoke test: OK\n";
    return 0;
}
