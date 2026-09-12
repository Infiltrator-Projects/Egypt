#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace egypt {

enum class Terrain : std::uint8_t { Desert, Floodplain, Water, Clay, Reeds };
enum class Structure : std::uint8_t { Empty, Road, House, ClayPit, Potter };

struct Tile {
    Terrain terrain = Terrain::Desert;
    Structure structure = Structure::Empty;
    std::uint8_t population = 0;
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
    [[nodiscard]] int population() const;
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
    void generate();
};
}
