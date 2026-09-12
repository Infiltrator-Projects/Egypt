#include "world.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <queue>
#include <stdexcept>

namespace egypt {

World::World() : tiles_(static_cast<std::size_t>(kWidth * kHeight)) { generate(); }

std::size_t World::index(int x, int y) const {
    return static_cast<std::size_t>(y * kWidth + x);
}

bool World::in_bounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < kWidth && y < kHeight;
}

const Tile& World::tile(int x, int y) const {
    if (!in_bounds(x, y)) throw std::out_of_range("tile");
    return tiles_[index(x, y)];
}

Tile& World::tile(int x, int y) {
    if (!in_bounds(x, y)) throw std::out_of_range("tile");
    return tiles_[index(x, y)];
}

void World::generate() {
    for (int y = 0; y < kHeight; ++y) {
        const int center = 34 + ((y / 5) % 3) - 1 + ((y / 13) % 2);
        for (int x = 0; x < kWidth; ++x) {
            Tile& t = tiles_[index(x, y)];
            t = {};
            const int d = x - center;
            if (d >= -2 && d <= 2) t.terrain = Terrain::Water;
            else if (d >= -7 && d <= 7) t.terrain = Terrain::Floodplain;
            else t.terrain = Terrain::Desert;

            if ((d == -3 || d == 3) && ((x * 11 + y * 7) % 5 == 0)) {
                t.terrain = Terrain::Reeds;
            }
            if (d <= -8 && d >= -13 && ((x * 17 + y * 13) % 31 < 4)) {
                t.terrain = Terrain::Clay;
            }
        }
    }
}

int World::cost(Structure s) {
    switch (s) {
        case Structure::Road: return 2;
        case Structure::House: return 10;
        case Structure::ClayPit: return 35;
        case Structure::Potter: return 55;
        case Structure::Farm: return 40;
        case Structure::Granary: return 80;
        case Structure::Market: return 65;
        default: return 0;
    }
}

bool World::can_place(Structure s, int x, int y) const {
    if (!in_bounds(x, y) || s == Structure::Empty) return false;
    const Tile& t = tile(x, y);
    if (t.structure != Structure::Empty) return false;

    if (s == Structure::Road) {
        return t.terrain != Terrain::Water && t.terrain != Terrain::Reeds;
    }
    if (s == Structure::ClayPit) return t.terrain == Terrain::Clay;
    if (s == Structure::Farm) return t.terrain == Terrain::Floodplain;
    if (s == Structure::House || s == Structure::Potter ||
        s == Structure::Granary || s == Structure::Market) {
        return t.terrain == Terrain::Desert || t.terrain == Terrain::Floodplain;
    }
    return false;
}

bool World::place(Structure s, int x, int y) {
    if (!can_place(s, x, y)) return false;
    const int c = cost(s);
    if (treasury_ < c) return false;
    tile(x, y).structure = s;
    treasury_ -= c;
    return true;
}

bool World::bulldoze(int x, int y) {
    if (!in_bounds(x, y)) return false;
    Tile& t = tile(x, y);
    if (t.structure == Structure::Empty) return false;
    t.structure = Structure::Empty;
    t.population = 0;
    t.food_stock = 0;
    return true;
}

bool World::has_road_access(int x, int y) const {
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; ++i) {
        const int nx = x + dx[i];
        const int ny = y + dy[i];
        if (in_bounds(nx, ny) && tile(nx, ny).structure == Structure::Road) return true;
    }
    return false;
}

std::vector<std::size_t> World::adjacent_roads(int x, int y) const {
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    std::vector<std::size_t> roads;
    for (int i = 0; i < 4; ++i) {
        const int nx = x + dx[i];
        const int ny = y + dy[i];
        if (in_bounds(nx, ny) && tile(nx, ny).structure == Structure::Road) {
            roads.push_back(index(nx, ny));
        }
    }
    return roads;
}

bool World::road_connected(int ax, int ay, int bx, int by) const {
    const auto starts = adjacent_roads(ax, ay);
    const auto goals = adjacent_roads(bx, by);
    if (starts.empty() || goals.empty()) return false;

    std::vector<std::uint8_t> goal_mask(tiles_.size(), 0);
    for (const auto g : goals) goal_mask[g] = 1;

    std::vector<std::uint8_t> seen(tiles_.size(), 0);
    std::queue<std::size_t> q;
    for (const auto s : starts) {
        seen[s] = 1;
        q.push(s);
    }

    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};

    while (!q.empty()) {
        const std::size_t current = q.front();
        q.pop();
        if (goal_mask[current]) return true;

        const int x = static_cast<int>(current % kWidth);
        const int y = static_cast<int>(current / kWidth);
        for (int i = 0; i < 4; ++i) {
            const int nx = x + dx[i];
            const int ny = y + dy[i];
            if (!in_bounds(nx, ny)) continue;
            const std::size_t ni = index(nx, ny);
            if (seen[ni] || tile(nx, ny).structure != Structure::Road) continue;
            seen[ni] = 1;
            q.push(ni);
        }
    }
    return false;
}

bool World::within_delivery_range(int ax, int ay, int bx, int by) const {
    return std::abs(ax - bx) + std::abs(ay - by) <= 14;
}

void World::produce_food() {
    if ((ticks_ % 4U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            Tile& t = tile(x, y);
            if (t.structure != Structure::Farm || !has_road_access(x, y)) continue;
            t.food_stock = static_cast<std::uint16_t>(std::min<int>(32, t.food_stock + 2));
        }
    }
}

void World::move_food_to_granaries() {
    for (int gy = 0; gy < kHeight; ++gy) {
        for (int gx = 0; gx < kWidth; ++gx) {
            Tile& granary = tile(gx, gy);
            if (granary.structure != Structure::Granary || !has_road_access(gx, gy)) continue;
            if (granary.food_stock >= 64) continue;

            for (int fy = 0; fy < kHeight && granary.food_stock < 64; ++fy) {
                for (int fx = 0; fx < kWidth && granary.food_stock < 64; ++fx) {
                    Tile& farm = tile(fx, fy);
                    if (farm.structure != Structure::Farm || farm.food_stock == 0) continue;
                    if (!road_connected(gx, gy, fx, fy)) continue;
                    const int amount = std::min<int>({4, farm.food_stock, 64 - granary.food_stock});
                    farm.food_stock = static_cast<std::uint16_t>(farm.food_stock - amount);
                    granary.food_stock = static_cast<std::uint16_t>(granary.food_stock + amount);
                }
            }
        }
    }
}

void World::move_food_to_markets() {
    for (int my = 0; my < kHeight; ++my) {
        for (int mx = 0; mx < kWidth; ++mx) {
            Tile& market = tile(mx, my);
            if (market.structure != Structure::Market || !has_road_access(mx, my)) continue;
            if (market.food_stock >= 24) continue;

            for (int gy = 0; gy < kHeight && market.food_stock < 24; ++gy) {
                for (int gx = 0; gx < kWidth && market.food_stock < 24; ++gx) {
                    Tile& granary = tile(gx, gy);
                    if (granary.structure != Structure::Granary || granary.food_stock == 0) continue;
                    if (!road_connected(mx, my, gx, gy)) continue;
                    const int amount = std::min<int>({4, granary.food_stock, 24 - market.food_stock});
                    granary.food_stock = static_cast<std::uint16_t>(granary.food_stock - amount);
                    market.food_stock = static_cast<std::uint16_t>(market.food_stock + amount);
                }
            }
        }
    }
}

void World::feed_houses() {
    for (int hy = 0; hy < kHeight; ++hy) {
        for (int hx = 0; hx < kWidth; ++hx) {
            Tile& house = tile(hx, hy);
            if (house.structure != Structure::House) continue;

            const bool road = has_road_access(hx, hy);
            if (road && house.food_stock < 8) {
                for (int my = 0; my < kHeight && house.food_stock < 8; ++my) {
                    for (int mx = 0; mx < kWidth && house.food_stock < 8; ++mx) {
                        Tile& market = tile(mx, my);
                        if (market.structure != Structure::Market || market.food_stock == 0) continue;
                        if (!within_delivery_range(hx, hy, mx, my)) continue;
                        if (!road_connected(hx, hy, mx, my)) continue;
                        const int amount = std::min<int>({2, market.food_stock, 8 - house.food_stock});
                        market.food_stock = static_cast<std::uint16_t>(market.food_stock - amount);
                        house.food_stock = static_cast<std::uint16_t>(house.food_stock + amount);
                    }
                }
            }

            if ((ticks_ % 4U) == 0U && house.population > 0 && house.food_stock > 0) {
                --house.food_stock;
            }

            if (!road) {
                if (house.population > 0) --house.population;
                continue;
            }

            if (house.food_stock > 0) {
                if ((ticks_ % 2U) == 0U && house.population < 8) ++house.population;
            } else if ((ticks_ % 2U) == 0U && house.population > 0) {
                --house.population;
            }
        }
    }
}

void World::tick() {
    ++ticks_;
    produce_food();
    move_food_to_granaries();
    move_food_to_markets();
    feed_houses();
}

int World::population() const {
    int p = 0;
    for (const Tile& t : tiles_) p += t.population;
    return p;
}

int World::total_food() const {
    int food = 0;
    for (const Tile& t : tiles_) food += t.food_stock;
    return food;
}

const char* World::terrain_name(Terrain t) {
    switch (t) {
        case Terrain::Desert: return "DESERT";
        case Terrain::Floodplain: return "FLOODPLAIN";
        case Terrain::Water: return "NILE";
        case Terrain::Clay: return "CLAY";
        case Terrain::Reeds: return "REEDS";
    }
    return "?";
}

const char* World::structure_name(Structure s) {
    switch (s) {
        case Structure::Empty: return "EMPTY";
        case Structure::Road: return "ROAD";
        case Structure::House: return "HOUSE";
        case Structure::ClayPit: return "CLAY PIT";
        case Structure::Potter: return "POTTER";
        case Structure::Farm: return "FARM";
        case Structure::Granary: return "GRANARY";
        case Structure::Market: return "MARKET";
    }
    return "?";
}

} // namespace egypt
