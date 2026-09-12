#include "world.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <queue>
#include <stdexcept>

namespace egypt {
namespace {
constexpr std::array<const char*, 12> kMonths{{
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
}};
constexpr std::uint64_t kTicksPerMonth = 48;
constexpr std::uint64_t kTicksPerYear = kTicksPerMonth * 12;
constexpr int kStartYearBc = 3500;
}

World::World() : tiles_(static_cast<std::size_t>(kWidth * kHeight)) { generate(); }

std::size_t World::index(int x, int y) const { return static_cast<std::size_t>(y * kWidth + x); }
bool World::in_bounds(int x, int y) const { return x >= 0 && y >= 0 && x < kWidth && y < kHeight; }

const Tile& World::tile(int x, int y) const {
    if (!in_bounds(x, y)) throw std::out_of_range("tile");
    return tiles_[index(x, y)];
}

Tile& World::tile(int x, int y) {
    if (!in_bounds(x, y)) throw std::out_of_range("tile");
    return tiles_[index(x, y)];
}

void World::generate() {
    immigrants_.clear();
    workers_.clear();
    goods_agents_.clear();
    for (int y = 0; y < kHeight; ++y) {
        const int center = 34 + ((y / 5) % 3) - 1 + ((y / 13) % 2);
        for (int x = 0; x < kWidth; ++x) {
            Tile& t = tiles_[index(x, y)];
            t = {};
            const int d = x - center;
            if (d >= -2 && d <= 2) t.terrain = Terrain::Water;
            else if (d >= -7 && d <= 7) t.terrain = Terrain::Floodplain;
            else t.terrain = Terrain::Desert;
            if ((d == -3 || d == 3) && ((x * 11 + y * 7) % 5 == 0)) t.terrain = Terrain::Reeds;
            if (d <= -8 && d >= -13 && ((x * 17 + y * 13) % 31 < 4)) t.terrain = Terrain::Clay;
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
        case Structure::Well: return 30;
        case Structure::HuntingLodge: return 70;
        default: return 0;
    }
}

bool World::can_place(Structure s, int x, int y) const {
    if (!in_bounds(x, y) || s == Structure::Empty) return false;
    const Tile& t = tile(x, y);
    if (t.structure != Structure::Empty) return false;
    if (s == Structure::Road) return t.terrain != Terrain::Water && t.terrain != Terrain::Reeds;
    if (s == Structure::ClayPit) return t.terrain == Terrain::Clay;
    if (s == Structure::Farm) return t.terrain == Terrain::Floodplain;
    if (s == Structure::House || s == Structure::Potter || s == Structure::Granary ||
        s == Structure::Market || s == Structure::Well || s == Structure::HuntingLodge) {
        return t.terrain == Terrain::Desert || t.terrain == Terrain::Floodplain;
    }
    return false;
}

bool World::place(Structure s, int x, int y) {
    if (!can_place(s, x, y)) return false;
    const int c = cost(s);
    if (treasury_ < c) return false;
    Tile& t = tile(x, y);
    const Terrain terrain = t.terrain;
    t = {};
    t.terrain = terrain;
    t.structure = s;
    treasury_ -= c;
    return true;
}

void World::release_worker(const WorkerAgent& worker) {
    if (!in_bounds(worker.home_x, worker.home_y)) return;
    Tile& home = tile(worker.home_x, worker.home_y);
    if (home.structure == Structure::House && home.employed > 0) --home.employed;
}

bool World::bulldoze(int x, int y) {
    if (!in_bounds(x, y)) return false;
    Tile& t = tile(x, y);
    if (t.structure == Structure::Empty) return false;

    for (std::size_t i = 0; i < workers_.size();) {
        const WorkerAgent& w = workers_[i];
        if ((w.home_x == x && w.home_y == y) || (w.job_x == x && w.job_y == y)) {
            release_worker(w);
            workers_.erase(workers_.begin() + static_cast<std::ptrdiff_t>(i));
        } else ++i;
    }
    for (std::size_t i = 0; i < goods_agents_.size();) {
        const GoodsAgent& g = goods_agents_[i];
        if ((g.source_x == x && g.source_y == y) || (g.target_x == x && g.target_y == y)) {
            goods_agents_.erase(goods_agents_.begin() + static_cast<std::ptrdiff_t>(i));
        } else ++i;
    }

    const Terrain terrain = t.terrain;
    t = {};
    t.terrain = terrain;
    return true;
}

bool World::has_road_access(int x, int y) const {
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; ++i) {
        const int nx = x + dx[i], ny = y + dy[i];
        if (in_bounds(nx, ny) && tile(nx, ny).structure == Structure::Road) return true;
    }
    return false;
}

std::vector<std::size_t> World::adjacent_roads(int x, int y) const {
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    std::vector<std::size_t> roads;
    for (int i = 0; i < 4; ++i) {
        const int nx = x + dx[i], ny = y + dy[i];
        if (in_bounds(nx, ny) && tile(nx, ny).structure == Structure::Road) roads.push_back(index(nx, ny));
    }
    return roads;
}

std::vector<std::size_t> World::road_path_between(int ax, int ay, int bx, int by) const {
    const auto starts = adjacent_roads(ax, ay);
    const auto goals = adjacent_roads(bx, by);
    if (starts.empty() || goals.empty()) return {};

    std::vector<std::uint8_t> goal_mask(tiles_.size(), 0), seen(tiles_.size(), 0);
    std::vector<int> previous(tiles_.size(), -1);
    std::queue<std::size_t> q;
    for (const auto g : goals) goal_mask[g] = 1;
    for (const auto s : starts) { seen[s] = 1; q.push(s); }

    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    std::size_t found = tiles_.size();
    while (!q.empty()) {
        const auto current = q.front(); q.pop();
        if (goal_mask[current]) { found = current; break; }
        const int x = static_cast<int>(current % kWidth), y = static_cast<int>(current / kWidth);
        for (int i = 0; i < 4; ++i) {
            const int nx = x + dx[i], ny = y + dy[i];
            if (!in_bounds(nx, ny)) continue;
            const auto ni = index(nx, ny);
            if (seen[ni] || tile(nx, ny).structure != Structure::Road) continue;
            seen[ni] = 1;
            previous[ni] = static_cast<int>(current);
            q.push(ni);
        }
    }
    if (found == tiles_.size()) return {};

    std::vector<std::size_t> reverse;
    for (std::size_t cur = found;;) {
        reverse.push_back(cur);
        const int p = previous[cur];
        if (p < 0) break;
        cur = static_cast<std::size_t>(p);
    }
    std::reverse(reverse.begin(), reverse.end());
    return reverse;
}

bool World::road_connected(int ax, int ay, int bx, int by) const { return !road_path_between(ax, ay, bx, by).empty(); }
bool World::within_delivery_range(int ax, int ay, int bx, int by) const { return std::abs(ax - bx) + std::abs(ay - by) <= 14; }

void World::record_road_traffic(int x, int y, int amount) {
    if (!in_bounds(x, y) || amount <= 0) return;
    Tile& t = tile(x, y);
    if (t.structure != Structure::Road) return;
    const unsigned next = static_cast<unsigned>(t.road_traffic) + static_cast<unsigned>(amount);
    t.road_traffic = static_cast<std::uint16_t>(std::min<unsigned>(next, std::numeric_limits<std::uint16_t>::max()));
}

int World::road_level(int x, int y) const {
    if (!in_bounds(x, y) || tile(x, y).structure != Structure::Road) return -1;
    const auto traffic = tile(x, y).road_traffic;
    if (traffic >= 64) return 2;
    if (traffic >= 16) return 1;
    return 0;
}

const char* World::road_level_name(int level) {
    switch (level) {
        case 0: return "NEW TRACK";
        case 1: return "WORN ROAD";
        case 2: return "ESTABLISHED ROAD";
        default: return "NO ROAD";
    }
}

bool World::has_well_service(int x, int y) const {
    if (!in_bounds(x, y)) return false;
    for (int wy = std::max(0, y - 4); wy <= std::min(kHeight - 1, y + 4); ++wy) {
        for (int wx = std::max(0, x - 4); wx <= std::min(kWidth - 1, x + 4); ++wx) {
            if (std::abs(wx - x) + std::abs(wy - y) > 4) continue;
            if (tile(wx, wy).structure == Structure::Well) return true;
        }
    }
    return false;
}

int World::house_capacity(int x, int y) const {
    if (!in_bounds(x, y) || tile(x, y).structure != Structure::House) return 0;
    switch (tile(x, y).housing_level) {
        case 0: return 8;
        case 1: return 12;
        case 2: return 16;
        default: return 20;
    }
}

int World::house_desirability(int x, int y) const {
    if (!in_bounds(x, y) || tile(x, y).structure != Structure::House) return 0;
    int score = 0;
    for (int yy = std::max(0, y - 4); yy <= std::min(kHeight - 1, y + 4); ++yy) {
        for (int xx = std::max(0, x - 4); xx <= std::min(kWidth - 1, x + 4); ++xx) {
            const int distance = std::abs(xx - x) + std::abs(yy - y);
            if (distance == 0 || distance > 4) continue;
            const Tile& t = tile(xx, yy);
            if (t.terrain == Terrain::Water || t.terrain == Terrain::Reeds) ++score;
            switch (t.structure) {
                case Structure::Well: score += 2; break;
                case Structure::Market: score += 1; break;
                case Structure::ClayPit: score -= 4; break;
                case Structure::Potter: score -= 3; break;
                case Structure::HuntingLodge: score -= 2; break;
                case Structure::Granary: score -= 1; break;
                default: break;
            }
        }
    }
    return std::clamp(score, -20, 20);
}

const char* World::housing_name(std::uint8_t level) {
    switch (level) {
        case 0: return "HUT";
        case 1: return "WATERED HOME";
        case 2: return "ESTABLISHED RESIDENCE";
        default: return "COURTYARD HOME";
    }
}

const char* World::house_evolution_status(int x, int y) const {
    if (!in_bounds(x, y) || tile(x, y).structure != Structure::House) return "NOT A HOUSE";
    const Tile& h = tile(x, y);
    if (!has_road_access(x, y)) return "NEEDS ROAD ACCESS";
    if (h.food_stock == 0) return "NEEDS A RELIABLE FOOD SUPPLY";
    if (!has_well_service(x, y)) return "NEEDS WATER FROM A NEARBY WELL";
    if (h.housing_level == 0 && h.housing_service_ticks < 4) return "FOOD AND WATER ARE STABILISING";
    if (h.housing_level == 1 && h.housing_service_ticks < 12) return "SUSTAIN FOOD AND WATER TO EVOLVE";
    if (h.housing_level == 2 && h.pottery_stock == 0) return "NEEDS POTTERY FROM A MARKET";
    if (h.housing_level == 2 && h.housing_goods_ticks < 6) return "POTTERY SUPPLY IS STABILISING";
    if (h.housing_level >= 3 && house_desirability(x, y) < 3) return "NEEDS A MORE DESIRABLE NEIGHBOURHOOD";
    if (h.housing_level >= 3) return "NEEDS MORE GOODS AND SERVICES FOR NEXT LEVEL";
    return "READY TO EVOLVE";
}

int World::month_index() const { return static_cast<int>((ticks_ / kTicksPerMonth) % 12U); }
int World::year_bc() const { return std::max(1, kStartYearBc - static_cast<int>(ticks_ / kTicksPerYear)); }
const char* World::month_name() const { return kMonths[static_cast<std::size_t>(month_index())]; }

void World::produce_food() {
    if ((ticks_ % 4U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        Tile& t = tile(x, y);
        if (t.structure == Structure::Farm && has_road_access(x, y)) {
            t.food_stock = static_cast<std::uint16_t>(std::min<int>(32, t.food_stock + 2));
        }
    }
}

void World::produce_clay() {
    if ((ticks_ % 4U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        Tile& t = tile(x, y);
        if (t.structure == Structure::ClayPit && has_road_access(x, y)) {
            t.clay_stock = static_cast<std::uint16_t>(std::min<int>(32, t.clay_stock + 2));
        }
    }
}

void World::produce_pottery() {
    if ((ticks_ % 4U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        Tile& t = tile(x, y);
        if (t.structure != Structure::Potter || !has_road_access(x, y)) continue;
        if (t.clay_stock >= 2 && t.pottery_stock <= 22) {
            t.clay_stock = static_cast<std::uint16_t>(t.clay_stock - 2);
            t.pottery_stock = static_cast<std::uint16_t>(t.pottery_stock + 2);
        }
    }
}

void World::move_food_to_granaries() {
    for (int gy = 0; gy < kHeight; ++gy) for (int gx = 0; gx < kWidth; ++gx) {
        Tile& granary = tile(gx, gy);
        if (granary.structure != Structure::Granary || !has_road_access(gx, gy)) continue;
        const int incoming = pending_goods_for(gx, gy, Resource::Food);
        const int space = 96 - static_cast<int>(granary.food_stock) - incoming;
        if (space <= 0) continue;
        bool dispatched = false;
        for (int sy = 0; sy < kHeight && !dispatched; ++sy) for (int sx = 0; sx < kWidth && !dispatched; ++sx) {
            Tile& source = tile(sx, sy);
            if ((source.structure != Structure::Farm && source.structure != Structure::HuntingLodge) || source.food_stock == 0) continue;
            const int amount = std::min<int>({6, source.food_stock, space});
            dispatched = spawn_goods_agent(sx, sy, gx, gy, Resource::Food, amount);
        }
    }
}

void World::move_food_to_markets() {
    for (int my = 0; my < kHeight; ++my) for (int mx = 0; mx < kWidth; ++mx) {
        Tile& market = tile(mx, my);
        if (market.structure != Structure::Market || !has_road_access(mx, my)) continue;
        const int incoming = pending_goods_for(mx, my, Resource::Food);
        const int space = 32 - static_cast<int>(market.food_stock) - incoming;
        if (space <= 0) continue;
        bool dispatched = false;
        for (int gy = 0; gy < kHeight && !dispatched; ++gy) for (int gx = 0; gx < kWidth && !dispatched; ++gx) {
            Tile& granary = tile(gx, gy);
            if (granary.structure != Structure::Granary || granary.food_stock == 0) continue;
            const int amount = std::min<int>({4, granary.food_stock, space});
            dispatched = spawn_goods_agent(gx, gy, mx, my, Resource::Food, amount);
        }
    }
}

void World::feed_houses() {
    for (int hy = 0; hy < kHeight; ++hy) for (int hx = 0; hx < kWidth; ++hx) {
        Tile& house = tile(hx, hy);
        if (house.structure != Structure::House) continue;
        const bool road = has_road_access(hx, hy);
        const int incoming = pending_goods_for(hx, hy, Resource::Food);
        const int space = 12 - static_cast<int>(house.food_stock) - incoming;
        if (road && space > 0) {
            bool dispatched = false;
            for (int my = 0; my < kHeight && !dispatched; ++my) for (int mx = 0; mx < kWidth && !dispatched; ++mx) {
                Tile& market = tile(mx, my);
                if (market.structure != Structure::Market || market.food_stock == 0) continue;
                if (!within_delivery_range(hx, hy, mx, my)) continue;
                dispatched = spawn_goods_agent(mx, my, hx, hy, Resource::Food, std::min<int>({2, market.food_stock, space}));
            }
        }

        if ((ticks_ % 4U) == 0U && house.population > 0 && house.food_stock > 0) --house.food_stock;
        if (!road) {
            if (house.population > 0) --house.population;
            if (house.employed > house.population) house.employed = house.population;
            continue;
        }
        if (house.food_stock == 0 && pending_goods_for(hx, hy, Resource::Food) == 0 &&
            (ticks_ % 4U) == 0U && house.population > 0) {
            --house.population;
            if (house.employed > house.population) house.employed = house.population;
        }
    }
}

void World::consume_household_goods() {
    if ((ticks_ % 24U) != 0U) return;
    for (auto& t : tiles_) {
        if (t.structure == Structure::House && t.population > 0 && t.pottery_stock > 0) --t.pottery_stock;
    }
}

void World::update_housing() {
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        Tile& house = tile(x, y);
        if (house.structure != Structure::House) continue;
        const bool supported = has_road_access(x, y) && house.food_stock > 0 && has_well_service(x, y);
        if (supported) {
            if (house.housing_service_ticks < 24) ++house.housing_service_ticks;
            if (house.housing_level == 0 && house.housing_service_ticks >= 4) house.housing_level = 1;
            if (house.housing_level == 1 && house.housing_service_ticks >= 12) house.housing_level = 2;
            if (house.housing_level >= 2 && house.pottery_stock > 0) {
                if (house.housing_goods_ticks < 12) ++house.housing_goods_ticks;
                if (house.housing_level == 2 && house.housing_goods_ticks >= 6) house.housing_level = 3;
            } else if (house.housing_goods_ticks > 0) {
                --house.housing_goods_ticks;
            }
        } else {
            if (house.housing_service_ticks > 0) --house.housing_service_ticks;
            if (house.housing_goods_ticks > 0) --house.housing_goods_ticks;
            if ((ticks_ % 8U) == 0U && house.housing_service_ticks == 0 && house.housing_level > 0) --house.housing_level;
        }
        if (house.housing_level >= 3 && house.pottery_stock == 0 && house.housing_goods_ticks == 0 && (ticks_ % 16U) == 0U) {
            house.housing_level = 2;
        }
        const int capacity = house_capacity(x, y);
        if (house.population > capacity) --house.population;
        if (house.employed > house.population) house.employed = house.population;
    }
}

std::vector<std::size_t> World::immigration_path_to(int house_x, int house_y) const {
    const auto goals = adjacent_roads(house_x, house_y);
    if (goals.empty()) return {};
    std::vector<int> previous(tiles_.size(), -1);
    std::vector<std::uint8_t> seen(tiles_.size(), 0);
    std::queue<std::size_t> q;
    for (const auto goal : goals) { seen[goal] = 1; q.push(goal); }
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    std::size_t entrance = tiles_.size();
    while (!q.empty()) {
        const auto current = q.front(); q.pop();
        const int x = static_cast<int>(current % kWidth), y = static_cast<int>(current / kWidth);
        if (x == 0 || y == 0 || x == kWidth - 1 || y == kHeight - 1) { entrance = current; break; }
        for (int i = 0; i < 4; ++i) {
            const int nx = x + dx[i], ny = y + dy[i];
            if (!in_bounds(nx, ny)) continue;
            const auto ni = index(nx, ny);
            if (seen[ni] || tile(nx, ny).structure != Structure::Road) continue;
            seen[ni] = 1;
            previous[ni] = static_cast<int>(current);
            q.push(ni);
        }
    }
    if (entrance == tiles_.size()) return {};
    std::vector<std::size_t> path;
    for (std::size_t current = entrance;;) {
        path.push_back(current);
        const int p = previous[current];
        if (p < 0) break;
        current = static_cast<std::size_t>(p);
    }
    return path;
}

int World::pending_immigrants_for(int house_x, int house_y) const {
    int pending = 0;
    for (const auto& a : immigrants_) if (a.target_x == house_x && a.target_y == house_y) pending += a.group_size;
    return pending;
}

void World::create_immigration() {
    if ((ticks_ % 2U) != 0U || immigrants_.size() >= 32U) return;
    int groups = 0;
    for (int hy = 0; hy < kHeight && groups < 4; ++hy) for (int hx = 0; hx < kWidth && groups < 4; ++hx) {
        const Tile& house = tile(hx, hy);
        if (house.structure != Structure::House || house.food_stock == 0 || !has_road_access(hx, hy)) continue;
        const int vacancies = house_capacity(hx, hy) - static_cast<int>(house.population) - pending_immigrants_for(hx, hy);
        if (vacancies <= 0) continue;
        auto path = immigration_path_to(hx, hy);
        if (path.empty()) continue;
        const int group = std::min(3, vacancies);
        const auto start = path.front();
        ImmigrantAgent a;
        a.x = static_cast<int>(start % kWidth); a.y = static_cast<int>(start / kWidth);
        a.target_x = hx; a.target_y = hy; a.group_size = static_cast<std::uint8_t>(group);
        a.road_path = std::move(path); a.path_position = 0;
        record_road_traffic(a.x, a.y, group);
        immigrants_.push_back(std::move(a));
        ++groups;
    }
}

void World::move_immigrants() {
    for (std::size_t i = 0; i < immigrants_.size();) {
        ImmigrantAgent& a = immigrants_[i];
        bool remove = false;
        if (!in_bounds(a.target_x, a.target_y)) remove = true;
        else {
            const Tile& h = tile(a.target_x, a.target_y);
            if (h.structure != Structure::House || h.food_stock == 0 || !has_road_access(a.target_x, a.target_y)) remove = true;
        }
        if (!remove && a.path_position + 1 < a.road_path.size()) {
            const auto next = a.road_path[++a.path_position];
            const int nx = static_cast<int>(next % kWidth), ny = static_cast<int>(next / kWidth);
            if (tile(nx, ny).structure != Structure::Road) remove = true;
            else { a.x = nx; a.y = ny; record_road_traffic(nx, ny, a.group_size); }
        } else if (!remove) {
            Tile& h = tile(a.target_x, a.target_y);
            const int space = house_capacity(a.target_x, a.target_y) - static_cast<int>(h.population);
            if (space > 0) h.population = static_cast<std::uint8_t>(h.population + std::min<int>(space, a.group_size));
            remove = true;
        }
        if (remove) immigrants_.erase(immigrants_.begin() + static_cast<std::ptrdiff_t>(i)); else ++i;
    }
}

int World::assigned_workers_for_job(int job_x, int job_y) const {
    int count = 0;
    for (const auto& w : workers_) if (w.job_x == job_x && w.job_y == job_y) ++count;
    return count;
}

std::vector<std::size_t> World::hunting_path_from(int job_x, int job_y) const {
    static constexpr std::array<std::array<int, 2>, 8> dirs{{
        {{-1,0}}, {{0,1}}, {{0,-1}}, {{1,0}}, {{-1,1}}, {{-1,-1}}, {{1,1}}, {{1,-1}}
    }};
    for (const auto& d : dirs) {
        std::vector<std::size_t> path;
        int x = job_x, y = job_y;
        bool valid = true;
        for (int step = 0; step < 4; ++step) {
            x += d[0]; y += d[1];
            if (!in_bounds(x, y) || tile(x, y).terrain == Terrain::Water) { valid = false; break; }
            path.push_back(index(x, y));
        }
        if (valid) return path;
    }
    return {};
}

void World::recruit_workers() {
    if ((ticks_ % 4U) != 0U) return;
    for (int jy = 0; jy < kHeight; ++jy) for (int jx = 0; jx < kWidth; ++jx) {
        if (tile(jx, jy).structure != Structure::HuntingLodge || !has_road_access(jx, jy)) continue;
        if (assigned_workers_for_job(jx, jy) >= 2) continue;
        bool hired = false;
        for (int hy = 0; hy < kHeight && !hired; ++hy) for (int hx = 0; hx < kWidth && !hired; ++hx) {
            Tile& home = tile(hx, hy);
            if (home.structure != Structure::House || home.population <= home.employed || !has_road_access(hx, hy)) continue;
            auto path = road_path_between(hx, hy, jx, jy);
            if (path.empty()) continue;
            WorkerAgent w;
            w.home_x = hx; w.home_y = hy; w.job_x = jx; w.job_y = jy;
            w.state = WorkerState::CommutingToJob;
            w.path = std::move(path);
            const auto start = w.path.front();
            w.x = static_cast<int>(start % kWidth); w.y = static_cast<int>(start / kWidth);
            record_road_traffic(w.x, w.y);
            ++home.employed;
            workers_.push_back(std::move(w));
            hired = true;
        }
    }
}

void World::move_workers() {
    for (std::size_t i = 0; i < workers_.size();) {
        WorkerAgent& w = workers_[i];
        bool remove = false;
        if (!in_bounds(w.home_x, w.home_y) || !in_bounds(w.job_x, w.job_y) ||
            tile(w.home_x, w.home_y).structure != Structure::House ||
            tile(w.job_x, w.job_y).structure != Structure::HuntingLodge) remove = true;

        if (!remove) {
            switch (w.state) {
                case WorkerState::CommutingToJob:
                    if (w.path_position + 1 < w.path.size()) {
                        const auto next = w.path[++w.path_position];
                        const int nx = static_cast<int>(next % kWidth), ny = static_cast<int>(next / kWidth);
                        if (tile(nx, ny).structure != Structure::Road) { remove = true; break; }
                        w.x = nx; w.y = ny; record_road_traffic(nx, ny);
                    } else {
                        w.x = w.job_x; w.y = w.job_y;
                        w.path = hunting_path_from(w.job_x, w.job_y); w.path_position = 0;
                        if (w.path.empty()) { w.work_ticks = 2; w.state = WorkerState::Hunting; }
                        else w.state = WorkerState::HunterOutbound;
                    }
                    break;
                case WorkerState::HunterOutbound:
                    if (w.path_position < w.path.size()) {
                        const auto next = w.path[w.path_position++];
                        w.x = static_cast<int>(next % kWidth); w.y = static_cast<int>(next / kWidth);
                    } else { w.work_ticks = 3; w.state = WorkerState::Hunting; }
                    break;
                case WorkerState::Hunting:
                    if (w.work_ticks > 0) --w.work_ticks;
                    if (w.work_ticks == 0) { w.payload_food = 5; w.state = WorkerState::HunterReturning; }
                    break;
                case WorkerState::HunterReturning:
                    if (w.path_position > 0) {
                        const auto next = w.path[--w.path_position];
                        w.x = static_cast<int>(next % kWidth); w.y = static_cast<int>(next / kWidth);
                    } else {
                        w.x = w.job_x; w.y = w.job_y;
                        Tile& lodge = tile(w.job_x, w.job_y);
                        lodge.food_stock = static_cast<std::uint16_t>(std::min<int>(48, lodge.food_stock + w.payload_food));
                        w.payload_food = 0;
                        w.path = road_path_between(w.home_x, w.home_y, w.job_x, w.job_y);
                        std::reverse(w.path.begin(), w.path.end());
                        w.path_position = 0;
                        w.state = WorkerState::CommutingHome;
                    }
                    break;
                case WorkerState::CommutingHome:
                    if (w.path.empty()) { remove = true; break; }
                    if (w.path_position < w.path.size()) {
                        const auto next = w.path[w.path_position++];
                        const int nx = static_cast<int>(next % kWidth), ny = static_cast<int>(next / kWidth);
                        if (tile(nx, ny).structure != Structure::Road) { remove = true; break; }
                        w.x = nx; w.y = ny; record_road_traffic(nx, ny);
                    } else {
                        w.x = w.home_x; w.y = w.home_y; w.work_ticks = 2; w.state = WorkerState::RestingAtHome;
                    }
                    break;
                case WorkerState::RestingAtHome:
                    if (w.work_ticks > 0) --w.work_ticks;
                    if (w.work_ticks == 0) {
                        w.path = road_path_between(w.home_x, w.home_y, w.job_x, w.job_y);
                        if (w.path.empty()) { remove = true; break; }
                        w.path_position = 0;
                        const auto start = w.path.front();
                        w.x = static_cast<int>(start % kWidth); w.y = static_cast<int>(start / kWidth);
                        record_road_traffic(w.x, w.y);
                        w.state = WorkerState::CommutingToJob;
                    }
                    break;
            }
        }
        if (remove) {
            release_worker(w);
            workers_.erase(workers_.begin() + static_cast<std::ptrdiff_t>(i));
        } else ++i;
    }
}

int World::pending_goods_for(int target_x, int target_y, Resource resource) const {
    int total = 0;
    for (const auto& g : goods_agents_) {
        if (g.target_x == target_x && g.target_y == target_y && g.resource == resource) total += g.amount;
    }
    return total;
}

bool World::has_pending_delivery(int target_x, int target_y, Resource resource) const {
    return pending_goods_for(target_x, target_y, resource) > 0;
}

bool World::spawn_goods_agent(int source_x, int source_y, int target_x, int target_y, Resource resource, int amount) {
    if (amount <= 0 || goods_agents_.size() >= 128U) return false;
    auto path = road_path_between(source_x, source_y, target_x, target_y);
    if (path.empty()) return false;

    Tile& source = tile(source_x, source_y);
    int available = 0;
    if (resource == Resource::Clay) available = source.clay_stock;
    else if (resource == Resource::Pottery) available = source.pottery_stock;
    else available = source.food_stock;
    amount = std::min(amount, available);
    if (amount <= 0) return false;

    if (resource == Resource::Clay) source.clay_stock = static_cast<std::uint16_t>(source.clay_stock - amount);
    else if (resource == Resource::Pottery) source.pottery_stock = static_cast<std::uint16_t>(source.pottery_stock - amount);
    else source.food_stock = static_cast<std::uint16_t>(source.food_stock - amount);

    GoodsAgent g;
    g.source_x = source_x; g.source_y = source_y;
    g.target_x = target_x; g.target_y = target_y;
    g.resource = resource; g.amount = static_cast<std::uint8_t>(amount);
    g.road_path = std::move(path);
    const auto start = g.road_path.front();
    g.x = static_cast<int>(start % kWidth); g.y = static_cast<int>(start / kWidth);
    record_road_traffic(g.x, g.y);
    goods_agents_.push_back(std::move(g));
    return true;
}

void World::queue_goods_deliveries() {
    if (goods_agents_.size() >= 128U) return;

    for (int py = 0; py < kHeight; ++py) for (int px = 0; px < kWidth; ++px) {
        Tile& potter = tile(px, py);
        if (potter.structure != Structure::Potter || !has_road_access(px, py)) continue;
        const int incoming = pending_goods_for(px, py, Resource::Clay);
        if (static_cast<int>(potter.clay_stock) + incoming >= 12) continue;
        bool dispatched = false;
        for (int sy = 0; sy < kHeight && !dispatched; ++sy) for (int sx = 0; sx < kWidth && !dispatched; ++sx) {
            Tile& pit = tile(sx, sy);
            if (pit.structure != Structure::ClayPit || pit.clay_stock == 0) continue;
            dispatched = spawn_goods_agent(sx, sy, px, py, Resource::Clay,
                std::min<int>(4, 12 - static_cast<int>(potter.clay_stock) - incoming));
        }
    }

    for (int my = 0; my < kHeight; ++my) for (int mx = 0; mx < kWidth; ++mx) {
        Tile& market = tile(mx, my);
        if (market.structure != Structure::Market || !has_road_access(mx, my)) continue;
        const int incoming = pending_goods_for(mx, my, Resource::Pottery);
        if (static_cast<int>(market.pottery_stock) + incoming >= 16) continue;
        bool dispatched = false;
        for (int sy = 0; sy < kHeight && !dispatched; ++sy) for (int sx = 0; sx < kWidth && !dispatched; ++sx) {
            Tile& potter = tile(sx, sy);
            if (potter.structure != Structure::Potter || potter.pottery_stock == 0) continue;
            dispatched = spawn_goods_agent(sx, sy, mx, my, Resource::Pottery,
                std::min<int>(4, 16 - static_cast<int>(market.pottery_stock) - incoming));
        }
    }

    for (int hy = 0; hy < kHeight; ++hy) for (int hx = 0; hx < kWidth; ++hx) {
        Tile& house = tile(hx, hy);
        if (house.structure != Structure::House || !has_road_access(hx, hy)) continue;
        const int incoming = pending_goods_for(hx, hy, Resource::Pottery);
        if (static_cast<int>(house.pottery_stock) + incoming >= 4) continue;
        bool dispatched = false;
        for (int my = 0; my < kHeight && !dispatched; ++my) for (int mx = 0; mx < kWidth && !dispatched; ++mx) {
            Tile& market = tile(mx, my);
            if (market.structure != Structure::Market || market.pottery_stock == 0) continue;
            if (!within_delivery_range(hx, hy, mx, my)) continue;
            dispatched = spawn_goods_agent(mx, my, hx, hy, Resource::Pottery,
                std::min<int>(2, 4 - static_cast<int>(house.pottery_stock) - incoming));
        }
    }
}

void World::move_goods() {
    for (std::size_t i = 0; i < goods_agents_.size();) {
        GoodsAgent& g = goods_agents_[i];
        bool remove = false;
        if (!in_bounds(g.target_x, g.target_y) || g.amount == 0) remove = true;
        if (!remove && g.path_position + 1 < g.road_path.size()) {
            const auto next = g.road_path[++g.path_position];
            const int nx = static_cast<int>(next % kWidth), ny = static_cast<int>(next / kWidth);
            if (tile(nx, ny).structure != Structure::Road) remove = true;
            else { g.x = nx; g.y = ny; record_road_traffic(nx, ny); }
        } else if (!remove) {
            Tile& target = tile(g.target_x, g.target_y);
            if (g.resource == Resource::Food && target.structure == Structure::Granary) {
                const int space = 96 - target.food_stock;
                target.food_stock = static_cast<std::uint16_t>(target.food_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Food && target.structure == Structure::Market) {
                const int space = 32 - target.food_stock;
                target.food_stock = static_cast<std::uint16_t>(target.food_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Food && target.structure == Structure::House) {
                const int space = 12 - target.food_stock;
                target.food_stock = static_cast<std::uint16_t>(target.food_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Clay && target.structure == Structure::Potter) {
                const int space = 24 - target.clay_stock;
                target.clay_stock = static_cast<std::uint16_t>(target.clay_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Pottery && target.structure == Structure::Market) {
                const int space = 24 - target.pottery_stock;
                target.pottery_stock = static_cast<std::uint16_t>(target.pottery_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Pottery && target.structure == Structure::House) {
                const int space = 6 - target.pottery_stock;
                target.pottery_stock = static_cast<std::uint16_t>(target.pottery_stock + std::min<int>(space, g.amount));
            }
            remove = true;
        }
        if (remove) goods_agents_.erase(goods_agents_.begin() + static_cast<std::ptrdiff_t>(i)); else ++i;
    }
}

void World::tick() {
    ++ticks_;
    produce_food();
    produce_clay();
    move_workers();
    move_goods();
    produce_pottery();
    move_food_to_granaries();
    move_food_to_markets();
    feed_houses();
    consume_household_goods();
    update_housing();
    create_immigration();
    move_immigrants();
    recruit_workers();
    queue_goods_deliveries();
}

int World::population() const { int p = 0; for (const auto& t : tiles_) p += t.population; return p; }
int World::employed_population() const { int p = 0; for (const auto& t : tiles_) p += t.employed; return p; }
int World::total_food() const { int v = 0; for (const auto& t : tiles_) v += t.food_stock; for (const auto& g : goods_agents_) if (g.resource == Resource::Food) v += g.amount; return v; }
int World::total_clay() const { int v = 0; for (const auto& t : tiles_) v += t.clay_stock; for (const auto& g : goods_agents_) if (g.resource == Resource::Clay) v += g.amount; return v; }
int World::total_pottery() const { int v = 0; for (const auto& t : tiles_) v += t.pottery_stock; for (const auto& g : goods_agents_) if (g.resource == Resource::Pottery) v += g.amount; return v; }
int World::immigrants_in_transit() const { int n = 0; for (const auto& a : immigrants_) n += a.group_size; return n; }

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
        case Structure::Well: return "WELL";
        case Structure::HuntingLodge: return "HUNTING LODGE";
    }
    return "?";
}

const char* World::worker_state_name(WorkerState s) {
    switch (s) {
        case WorkerState::CommutingToJob: return "TO JOB";
        case WorkerState::HunterOutbound: return "HUNTER OUT";
        case WorkerState::Hunting: return "HUNTING";
        case WorkerState::HunterReturning: return "HUNTER RETURN";
        case WorkerState::CommutingHome: return "TO HOME";
        case WorkerState::RestingAtHome: return "HOME";
    }
    return "?";
}

const char* World::resource_name(Resource r) {
    switch (r) {
        case Resource::Food: return "FOOD";
        case Resource::Clay: return "CLAY";
        case Resource::Pottery: return "POTTERY";
    }
    return "?";
}

} // namespace egypt
