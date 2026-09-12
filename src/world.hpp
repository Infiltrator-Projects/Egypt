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

enum class Resource : std::uint8_t { Food, Clay, Pottery };

enum class WorkerRole : std::uint8_t {
    Hunter,
    Farmer,
    ClayWorker,
    Potter,
    GranaryWorker,
    MarketWorker
};

enum class WorkerState : std::uint8_t {
    CommutingToJob,
    WorkingAtJob,
    HunterOutbound,
    Hunting,
    HunterReturning,
    CommutingHome,
    RestingAtHome
};

struct Tile {
    Terrain terrain = Terrain::Desert;
    Structure structure = Structure::Empty;
    std::uint8_t population = 0;
    std::uint8_t employed = 0;
    std::uint8_t housing_level = 0;
    std::uint8_t housing_service_ticks = 0;
    std::uint8_t housing_goods_ticks = 0;
    std::uint16_t food_stock = 0;
    std::uint16_t clay_stock = 0;
    std::uint16_t pottery_stock = 0;
    std::uint16_t road_traffic = 0;

    // Houses use an authoritative residence identity. Ordinary houses point
    // to themselves; every tile in a merged 2x2 residence points to the same
    // anchor. Population, employment and household goods live on the anchor.
    std::int16_t residence_anchor_x = -1;
    std::int16_t residence_anchor_y = -1;
    std::uint8_t residence_tiles = 1;
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

struct WorkerAgent {
    int x = 0;
    int y = 0;
    int home_x = 0;
    int home_y = 0;
    int job_x = 0;
    int job_y = 0;
    WorkerRole role = WorkerRole::Hunter;
    WorkerState state = WorkerState::CommutingToJob;
    std::vector<std::size_t> path;
    std::size_t path_position = 0;
    std::uint8_t work_ticks = 0;
    std::uint8_t payload_food = 0;
};

struct GoodsAgent {
    int x = 0;
    int y = 0;
    int source_x = 0;
    int source_y = 0;
    int target_x = 0;
    int target_y = 0;
    Resource resource = Resource::Clay;
    std::uint8_t amount = 0;
    std::vector<std::size_t> road_path;
    std::size_t path_position = 0;
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
    [[nodiscard]] bool has_well_service(int x, int y) const;

    [[nodiscard]] int residence_anchor_x(int x, int y) const;
    [[nodiscard]] int residence_anchor_y(int x, int y) const;
    [[nodiscard]] bool is_residence_anchor(int x, int y) const;
    [[nodiscard]] int residence_tiles(int x, int y) const;
    [[nodiscard]] int residence_population(int x, int y) const;
    [[nodiscard]] int residence_employed(int x, int y) const;
    [[nodiscard]] int residence_food(int x, int y) const;
    [[nodiscard]] int residence_pottery(int x, int y) const;
    [[nodiscard]] const char* residence_name(int x, int y) const;

    [[nodiscard]] int house_capacity(int x, int y) const;
    [[nodiscard]] int house_desirability(int x, int y) const;
    [[nodiscard]] int road_level(int x, int y) const;
    [[nodiscard]] const char* house_evolution_status(int x, int y) const;
    [[nodiscard]] int workers_assigned(int x, int y) const;
    [[nodiscard]] int workers_active(int x, int y) const;
    [[nodiscard]] int worker_capacity(int x, int y) const;
    [[nodiscard]] int population() const;
    [[nodiscard]] int employed_population() const;
    [[nodiscard]] int total_food() const;
    [[nodiscard]] int total_clay() const;
    [[nodiscard]] int total_pottery() const;
    [[nodiscard]] int immigrants_in_transit() const;
    [[nodiscard]] const std::vector<ImmigrantAgent>& immigrants() const { return immigrants_; }
    [[nodiscard]] const std::vector<WorkerAgent>& workers() const { return workers_; }
    [[nodiscard]] const std::vector<GoodsAgent>& goods_agents() const { return goods_agents_; }
    [[nodiscard]] int treasury() const { return treasury_; }
    [[nodiscard]] std::uint64_t simulation_ticks() const { return ticks_; }
    [[nodiscard]] int month_index() const;
    [[nodiscard]] int year_bc() const;
    [[nodiscard]] const char* month_name() const;

    [[nodiscard]] static int cost(Structure structure);
    [[nodiscard]] static const char* terrain_name(Terrain terrain);
    [[nodiscard]] static const char* structure_name(Structure structure);
    [[nodiscard]] static const char* worker_state_name(WorkerState state);
    [[nodiscard]] static const char* worker_role_name(WorkerRole role);
    [[nodiscard]] static const char* resource_name(Resource resource);
    [[nodiscard]] static const char* housing_name(std::uint8_t level);
    [[nodiscard]] static const char* road_level_name(int level);

private:
    std::vector<Tile> tiles_;
    std::vector<ImmigrantAgent> immigrants_;
    std::vector<WorkerAgent> workers_;
    std::vector<GoodsAgent> goods_agents_;
    int treasury_ = 5000;
    std::uint64_t ticks_ = 0;

    [[nodiscard]] std::size_t index(int x, int y) const;
    [[nodiscard]] bool canonical_house(int x, int y, int& anchor_x, int& anchor_y) const;
    [[nodiscard]] int base_house_capacity(std::uint8_t level) const;
    [[nodiscard]] int residence_extent(int anchor_x, int anchor_y) const;
    [[nodiscard]] bool can_merge_residence(int anchor_x, int anchor_y) const;
    void merge_residence(int anchor_x, int anchor_y);
    void update_residence_merges();
    void sync_residence_members(int anchor_x, int anchor_y);

    [[nodiscard]] std::vector<std::size_t> adjacent_roads(int x, int y) const;
    [[nodiscard]] bool within_delivery_range(int ax, int ay, int bx, int by) const;
    [[nodiscard]] std::vector<std::size_t> road_path_between(int ax, int ay, int bx, int by) const;
    [[nodiscard]] std::vector<std::size_t> immigration_path_to(int house_x, int house_y) const;
    [[nodiscard]] int pending_immigrants_for(int house_x, int house_y) const;
    [[nodiscard]] int assigned_workers_for_job(int job_x, int job_y) const;
    [[nodiscard]] int active_workers_for_job(int job_x, int job_y) const;
    [[nodiscard]] int desired_workers_for(Structure structure) const;
    [[nodiscard]] WorkerRole role_for(Structure structure) const;
    [[nodiscard]] bool is_workplace(Structure structure) const;
    [[nodiscard]] std::vector<std::size_t> hunting_path_from(int job_x, int job_y) const;
    [[nodiscard]] int pending_goods_for(int target_x, int target_y, Resource resource) const;

    void generate();
    void produce_food();
    void produce_clay();
    void produce_pottery();
    void move_food_to_granaries();
    void move_food_to_markets();
    void feed_houses();
    void update_housing();
    void create_immigration();
    void move_immigrants();
    void recruit_workers();
    void move_workers();
    void release_worker(const WorkerAgent& worker);
    void start_commute_home(WorkerAgent& worker);
    void queue_goods_deliveries();
    void move_goods();
    void consume_household_goods();
    bool spawn_goods_agent(int source_x, int source_y, int target_x, int target_y, Resource resource, int amount);
    void record_road_traffic(int x, int y, int amount = 1);
};

} // namespace egypt
