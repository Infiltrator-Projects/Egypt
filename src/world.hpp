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
    Market
};

struct Tile {
    Terrain terrain = Terrain::Desert;
    Structure structure = Structure::Empty;
    std::uint8_t population = 0;
    std::uint16_t food_stock = 0;
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
    [[nodiscard]] int population() const;
    [[nodiscard]] int total_food() const;
    [[nodiscard]] int treasury() const { return treasury_; }
    [[nodiscard]] std::uint64_t simulation_ticks() const { return ticks_; }

    [[nodiscard]] static int cost(Structure structure);
    [[nodiscard]] static const char* terrain_name(Terrain terrain);
    [[nodiscard]] static const char* structure_name(Structure structure);

private:
    std::vector<Tile> tiles_;
    int treasury_ = 5000;
    std::uint64_t ticks_ = 0;

    [[nodiscard]] std::size_t index(int x, int y) const;
    [[nodiscard]] std::vector<std::size_t> adjacent_roads(int x, int y) const;
    [[nodiscard]] bool within_delivery_range(int ax, int ay, int bx, int by) const;
    void generate();
    void produce_food();
    void move_food_to_granaries();
    void move_food_to_markets();
    void feed_houses();
};

} // namespace egypt
