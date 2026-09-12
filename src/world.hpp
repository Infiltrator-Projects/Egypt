#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace egypt {

enum class Terrain : std::uint8_t { Desert, Floodplain, Water, Clay, Reeds };
enum class Structure : std::uint8_t {
    Empty,
    Road,
    House,
    ClayPit,
    Potter,
    Farm,
    Granary,
    Market,
    Well,
    HuntingLodge
};

struct Tile {
    Terrain terrain = Terrain::Desert;
    Structure structure = Structure::Empty;
    std::uint8_t population = 0;
    std::uint8_t housing_level = 0;
    std::uint16_t food_stock = 0;
};

struct ImmigrantAgent {
    int x = 0;
    int y = 0;
    int target_x = 0;
    int target_y = 0;
    std::uint8_t group_size = 1;
    std::vector<std::size_t> road_path;
    std::size_t path_position = 0;
};

enum class WorkerState : std::uint8_t {
    ToLodge,
    Hunting,
    ReturningToLodge,
    GoingHome
};

struct WorkerAgent {
    int x = 0;
    int y = 0;
    int home_x = 0;
    int home_y = 0;
    int work_x = 0;
    int work_y = 0;
    int hunt_x = 0;
    int hunt_y = 0;
    WorkerState state = WorkerState::ToLodge;
    std::vector<std::size_t> road_path;
    std::size_t path_position = 0;
    std::uint8_t hunt_ticks = 0;
    bool carrying_food = false;
};

class World {
public:
    static constexpr int kWidth = 64;
    static constexpr int kHeight = 36;

    World();

    [[nodiscard]] bool in_bounds(int x, int y) const;
    [[nodiscard]] const Tile& tile(int x, int y) const;
    [[nodiscard]] Tile& tile(int x, int y);

    [[nodiscard]] bool can_place(Structure structure, int x, int y) const;
    bool place(Structure structure, int x, int y);
    bool bulldoze(int x, int y);

    void tick();

    [[nodiscard]] bool has_road_access(int x, int y) const;
    [[nodiscard]] bool road_connected(int ax, int ay, int bx, int by) const;
    [[nodiscard]] bool has_water_service(int x, int y) const;
    [[nodiscard]] int house_capacity(int x, int y) const;
    [[nodiscard]] int population() const;
    [[nodiscard]] int total_food() const;
    [[nodiscard]] int immigrants_in_transit() const;
    [[nodiscard]] int workers_in_transit() const { return static_cast<int>(workers_.size()); }
    [[nodiscard]] int hunting_jobs() const;
    [[nodiscard]] int hunting_jobs_filled() const;
    [[nodiscard]] const std::vector<ImmigrantAgent>& immigrants() const { return immigrants_; }
    [[nodiscard]] const std::vector<WorkerAgent>& workers() const { return workers_; }
    [[nodiscard]] int treasury() const { return treasury_; }
    [[nodiscard]] std::uint64_t simulation_ticks() const { return ticks_; }

    [[nodiscard]] static int cost(Structure structure);
    [[nodiscard]] static const char* terrain_name(Terrain terrain);
    [[nodiscard]] static const char* structure_name(Structure structure);

private:
    std::vector<Tile> tiles_;
    std::vector<ImmigrantAgent> immigrants_;
    std::vector<WorkerAgent> workers_;
    int treasury_ = 5000;
    std::uint64_t ticks_ = 0;

    [[nodiscard]] std::size_t index(int x, int y) const;
    [[nodiscard]] std::vector<std::size_t> adjacent_roads(int x, int y) const;
    [[nodiscard]] bool within_delivery_range(int ax, int ay, int bx, int by) const;
    [[nodiscard]] std::vector<std::size_t> road_path_between(int ax, int ay, int bx, int by) const;
    [[nodiscard]] std::vector<std::size_t> immigration_path_to(int house_x, int house_y) const;
    [[nodiscard]] int pending_immigrants_for(int house_x, int house_y) const;
    [[nodiscard]] int active_workers_from(int house_x, int house_y) const;
    [[nodiscard]] int active_workers_at(int work_x, int work_y) const;
    [[nodiscard]] bool find_hunt_target(int lodge_x, int lodge_y, int& target_x, int& target_y) const;
    void generate();
    void produce_food();
    void move_food_to_granaries();
    void move_food_to_markets();
    void feed_houses();
    void update_housing();
    void create_immigration();
    void move_immigrants();
    void recruit_workers();
    void move_workers();
};

} // namespace egypt
