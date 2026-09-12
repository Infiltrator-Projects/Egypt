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
    wildlife_.clear();
    next_wildlife_id_ = 1;
    wildlife_harvests_ = 0;
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
    seed_wildlife();
}

void World::seed_wildlife() {
    wildlife_.clear();
    for (int y = 4; y < kHeight - 2 && wildlife_.size() < 28U; y += 4) {
        for (int x = 8; x < kWidth - 2 && wildlife_.size() < 28U; x += 6) {
            const Tile& t = tile(x, y);
            if (t.structure != Structure::Empty) continue;
            if (t.terrain != Terrain::Desert && t.terrain != Terrain::Floodplain) continue;
            WildlifeAgent animal;
            animal.id = next_wildlife_id_++;
            animal.x = x;
            animal.y = y;
            animal.kind = WildlifeKind::Gazelle;
            wildlife_.push_back(animal);
        }
    }
}

const WildlifeAgent* World::wildlife_by_id(int wildlife_id) const {
    for (const auto& animal : wildlife_) if (animal.id == wildlife_id) return &animal;
    return nullptr;
}

bool World::harvest_wildlife(int wildlife_id) {
    for (std::size_t i = 0; i < wildlife_.size(); ++i) {
        if (wildlife_[i].id != wildlife_id) continue;
        wildlife_.erase(wildlife_.begin() + static_cast<std::ptrdiff_t>(i));
        ++wildlife_harvests_;
        return true;
    }
    return false;
}

void World::move_wildlife() {
    if ((ticks_ % 8U) == 0U) {
        static constexpr int dx[4] = {1, 0, -1, 0};
        static constexpr int dy[4] = {0, 1, 0, -1};
        for (auto& animal : wildlife_) {
            bool reserved = false;
            for (const auto& worker : workers_) {
                if (worker.role == WorkerRole::Hunter && worker.wildlife_target_id == animal.id) { reserved = true; break; }
            }
            if (reserved) continue;
            const int first = static_cast<int>((ticks_ / 8U + static_cast<std::uint64_t>(animal.id * 3)) % 4U);
            for (int attempt = 0; attempt < 4; ++attempt) {
                const int dir = (first + attempt) % 4;
                const int nx = animal.x + dx[dir], ny = animal.y + dy[dir];
                if (!in_bounds(nx, ny)) continue;
                const Tile& t = tile(nx, ny);
                if (t.terrain == Terrain::Water || t.terrain == Terrain::Reeds) continue;
                if (t.structure != Structure::Empty) continue;
                animal.x = nx;
                animal.y = ny;
                break;
            }
        }
    }

    if ((ticks_ % 96U) == 0U && wildlife_.size() < 20U) {
        for (int attempt = 0; attempt < kWidth * kHeight; ++attempt) {
            const int x = 3 + static_cast<int>((ticks_ + static_cast<std::uint64_t>(next_wildlife_id_ * 11 + attempt * 7)) % (kWidth - 6));
            const int y = 3 + static_cast<int>(((ticks_ / 3U) + static_cast<std::uint64_t>(next_wildlife_id_ * 5 + attempt * 13)) % (kHeight - 6));
            const Tile& t = tile(x, y);
            if (t.structure != Structure::Empty) continue;
            if (t.terrain != Terrain::Desert && t.terrain != Terrain::Floodplain) continue;
            WildlifeAgent animal;
            animal.id = next_wildlife_id_++;
            animal.x = x;
            animal.y = y;
            animal.kind = WildlifeKind::Gazelle;
            wildlife_.push_back(animal);
            break;
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
    if (s == Structure::House) {
        t.residence_anchor_x = static_cast<std::int16_t>(x);
        t.residence_anchor_y = static_cast<std::int16_t>(y);
        t.residence_tiles = 1;
    }
    treasury_ -= c;
    return true;
}

bool World::canonical_house(int x, int y, int& ax, int& ay) const {
    if (!in_bounds(x, y) || tile(x, y).structure != Structure::House) return false;
    const Tile& member = tile(x, y);
    ax = member.residence_anchor_x >= 0 ? member.residence_anchor_x : x;
    ay = member.residence_anchor_y >= 0 ? member.residence_anchor_y : y;
    if (!in_bounds(ax, ay) || tile(ax, ay).structure != Structure::House) {
        ax = x;
        ay = y;
    }
    return true;
}

int World::residence_anchor_x(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) ? ax : -1;
}

int World::residence_anchor_y(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) ? ay : -1;
}

bool World::is_residence_anchor(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) && ax == x && ay == y;
}

int World::residence_tiles(int x, int y) const {
    int ax = -1, ay = -1;
    if (!canonical_house(x, y, ax, ay)) return 0;
    return std::max(1, static_cast<int>(tile(ax, ay).residence_tiles));
}

int World::residence_population(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) ? tile(ax, ay).population : 0;
}

int World::residence_employed(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) ? tile(ax, ay).employed : 0;
}

int World::residence_food(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) ? tile(ax, ay).food_stock : 0;
}

int World::residence_pottery(int x, int y) const {
    int ax = -1, ay = -1;
    return canonical_house(x, y, ax, ay) ? tile(ax, ay).pottery_stock : 0;
}

const char* World::residence_name(int x, int y) const {
    int ax = -1, ay = -1;
    if (!canonical_house(x, y, ax, ay)) return "NOT A RESIDENCE";
    if (tile(ax, ay).residence_tiles >= 4) {
        if (tile(ax, ay).housing_level >= 3) return "COURTYARD COMPOUND";
        return "MERGED RESIDENCE";
    }
    return housing_name(tile(ax, ay).housing_level);
}

int World::base_house_capacity(std::uint8_t level) const {
    switch (level) {
        case 0: return 8;
        case 1: return 12;
        case 2: return 16;
        default: return 20;
    }
}

int World::residence_extent(int ax, int ay) const {
    if (!in_bounds(ax, ay) || tile(ax, ay).structure != Structure::House) return 0;
    return tile(ax, ay).residence_tiles >= 4 ? 2 : 1;
}

bool World::can_merge_residence(int ax, int ay) const {
    if (!in_bounds(ax, ay) || !in_bounds(ax + 1, ay + 1)) return false;
    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            const int x = ax + dx, y = ay + dy;
            const Tile& h = tile(x, y);
            if (h.structure != Structure::House || h.housing_level < 2) return false;
            int hx = -1, hy = -1;
            if (!canonical_house(x, y, hx, hy) || hx != x || hy != y || h.residence_tiles != 1) return false;
        }
    }
    return true;
}

void World::sync_residence_members(int ax, int ay) {
    if (!in_bounds(ax, ay) || tile(ax, ay).structure != Structure::House) return;
    Tile& anchor = tile(ax, ay);
    const int extent = residence_extent(ax, ay);
    for (int dy = 0; dy < extent; ++dy) {
        for (int dx = 0; dx < extent; ++dx) {
            Tile& member = tile(ax + dx, ay + dy);
            member.residence_anchor_x = static_cast<std::int16_t>(ax);
            member.residence_anchor_y = static_cast<std::int16_t>(ay);
            member.residence_tiles = anchor.residence_tiles;
            member.housing_level = anchor.housing_level;
            if (dx != 0 || dy != 0) {
                member.population = 0;
                member.employed = 0;
                member.food_stock = 0;
                member.pottery_stock = 0;
                member.housing_service_ticks = 0;
                member.housing_goods_ticks = 0;
            }
        }
    }
}

void World::merge_residence(int ax, int ay) {
    if (!can_merge_residence(ax, ay)) return;

    int population = 0, employed = 0, food = 0, pottery = 0;
    std::uint8_t level = std::numeric_limits<std::uint8_t>::max();
    std::uint8_t service = std::numeric_limits<std::uint8_t>::max();
    std::uint8_t goods = std::numeric_limits<std::uint8_t>::max();

    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            const Tile& h = tile(ax + dx, ay + dy);
            population += h.population;
            employed += h.employed;
            food += h.food_stock;
            pottery += h.pottery_stock;
            level = std::min(level, h.housing_level);
            service = std::min(service, h.housing_service_ticks);
            goods = std::min(goods, h.housing_goods_ticks);
        }
    }

    Tile& anchor = tile(ax, ay);
    anchor.residence_anchor_x = static_cast<std::int16_t>(ax);
    anchor.residence_anchor_y = static_cast<std::int16_t>(ay);
    anchor.residence_tiles = 4;
    anchor.housing_level = level;
    anchor.housing_service_ticks = service;
    anchor.housing_goods_ticks = goods;
    anchor.population = static_cast<std::uint8_t>(std::min(population, 255));
    anchor.employed = static_cast<std::uint8_t>(std::min({employed, population, 255}));
    anchor.food_stock = static_cast<std::uint16_t>(std::min(food, 48));
    anchor.pottery_stock = static_cast<std::uint16_t>(std::min(pottery, 24));

    sync_residence_members(ax, ay);

    for (auto& worker : workers_) {
        if (worker.home_x >= ax && worker.home_x <= ax + 1 &&
            worker.home_y >= ay && worker.home_y <= ay + 1) {
            worker.home_x = ax;
            worker.home_y = ay;
        }
    }
    for (auto& immigrant : immigrants_) {
        if (immigrant.target_x >= ax && immigrant.target_x <= ax + 1 &&
            immigrant.target_y >= ay && immigrant.target_y <= ay + 1) {
            immigrant.target_x = ax;
            immigrant.target_y = ay;
        }
    }
    for (auto& goods_agent : goods_agents_) {
        if (goods_agent.target_x >= ax && goods_agent.target_x <= ax + 1 &&
            goods_agent.target_y >= ay && goods_agent.target_y <= ay + 1 &&
            tile(goods_agent.target_x, goods_agent.target_y).structure == Structure::House) {
            goods_agent.target_x = ax;
            goods_agent.target_y = ay;
        }
    }
}

void World::update_residence_merges() {
    for (int y = 0; y < kHeight - 1; ++y) {
        for (int x = 0; x < kWidth - 1; ++x) {
            if (can_merge_residence(x, y)) merge_residence(x, y);
        }
    }
}

bool World::is_workplace(Structure s) const {
    return s == Structure::Farm || s == Structure::ClayPit || s == Structure::Potter ||
           s == Structure::Granary || s == Structure::Market || s == Structure::HuntingLodge;
}

int World::desired_workers_for(Structure s) const {
    if (s == Structure::HuntingLodge) return 2;
    return is_workplace(s) ? 1 : 0;
}

WorkerRole World::role_for(Structure s) const {
    switch (s) {
        case Structure::Farm: return WorkerRole::Farmer;
        case Structure::ClayPit: return WorkerRole::ClayWorker;
        case Structure::Potter: return WorkerRole::Potter;
        case Structure::Granary: return WorkerRole::GranaryWorker;
        case Structure::Market: return WorkerRole::MarketWorker;
        case Structure::HuntingLodge: return WorkerRole::Hunter;
        default: return WorkerRole::Hunter;
    }
}

int World::assigned_workers_for_job(int job_x, int job_y) const {
    int count = 0;
    for (const auto& w : workers_) if (w.job_x == job_x && w.job_y == job_y) ++count;
    return count;
}

int World::active_workers_for_job(int job_x, int job_y) const {
    int count = 0;
    for (const auto& w : workers_) {
        if (w.job_x != job_x || w.job_y != job_y) continue;
        if (w.state == WorkerState::WorkingAtJob || w.state == WorkerState::HunterOutbound ||
            w.state == WorkerState::Hunting || w.state == WorkerState::HunterReturning) ++count;
    }
    return count;
}

int World::workers_assigned(int x, int y) const { return in_bounds(x, y) ? assigned_workers_for_job(x, y) : 0; }
int World::workers_active(int x, int y) const { return in_bounds(x, y) ? active_workers_for_job(x, y) : 0; }
int World::worker_capacity(int x, int y) const { return in_bounds(x, y) ? desired_workers_for(tile(x, y).structure) : 0; }

void World::release_worker(const WorkerAgent& worker) {
    int hx = worker.home_x, hy = worker.home_y;
    int ax = -1, ay = -1;
    if (canonical_house(hx, hy, ax, ay)) {
        Tile& home = tile(ax, ay);
        if (home.employed > 0) --home.employed;
    }
}

bool World::bulldoze(int x, int y) {
    if (!in_bounds(x, y)) return false;
    if (tile(x, y).structure == Structure::Empty) return false;

    int ax = x, ay = y;
    int extent = 1;
    const bool house = canonical_house(x, y, ax, ay);
    if (house) extent = residence_extent(ax, ay);

    auto belongs_to_target = [&](int tx, int ty) {
        if (!house) return tx == x && ty == y;
        return tx >= ax && tx < ax + extent && ty >= ay && ty < ay + extent;
    };

    for (std::size_t i = 0; i < workers_.size();) {
        const WorkerAgent& w = workers_[i];
        if (belongs_to_target(w.home_x, w.home_y) || (w.job_x == x && w.job_y == y)) {
            release_worker(w);
            workers_.erase(workers_.begin() + static_cast<std::ptrdiff_t>(i));
        } else ++i;
    }
    for (std::size_t i = 0; i < goods_agents_.size();) {
        const GoodsAgent& g = goods_agents_[i];
        if ((g.source_x == x && g.source_y == y) || belongs_to_target(g.target_x, g.target_y)) {
            goods_agents_.erase(goods_agents_.begin() + static_cast<std::ptrdiff_t>(i));
        } else ++i;
    }
    for (std::size_t i = 0; i < immigrants_.size();) {
        const ImmigrantAgent& a = immigrants_[i];
        if (belongs_to_target(a.target_x, a.target_y)) immigrants_.erase(immigrants_.begin() + static_cast<std::ptrdiff_t>(i));
        else ++i;
    }

    if (house) {
        for (int dy = 0; dy < extent; ++dy) {
            for (int dx = 0; dx < extent; ++dx) {
                Tile& t = tile(ax + dx, ay + dy);
                const Terrain terrain = t.terrain;
                t = {};
                t.terrain = terrain;
            }
        }
    } else {
        Tile& t = tile(x, y);
        const Terrain terrain = t.terrain;
        t = {};
        t.terrain = terrain;
    }
    return true;
}

std::vector<std::size_t> World::adjacent_roads(int x, int y) const {
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};
    std::vector<std::size_t> roads;

    int ax = x, ay = y;
    int extent = 1;
    if (canonical_house(x, y, ax, ay)) extent = residence_extent(ax, ay);

    for (int oy = 0; oy < extent; ++oy) {
        for (int ox = 0; ox < extent; ++ox) {
            const int hx = ax + ox, hy = ay + oy;
            for (int i = 0; i < 4; ++i) {
                const int nx = hx + dx[i], ny = hy + dy[i];
                if (!in_bounds(nx, ny) || tile(nx, ny).structure != Structure::Road) continue;
                const auto ni = index(nx, ny);
                if (std::find(roads.begin(), roads.end(), ni) == roads.end()) roads.push_back(ni);
            }
        }
    }
    return roads;
}

bool World::has_road_access(int x, int y) const { return !adjacent_roads(x, y).empty(); }

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
    int ax = x, ay = y;
    int extent = 1;
    if (canonical_house(x, y, ax, ay)) extent = residence_extent(ax, ay);
    if (!in_bounds(ax, ay)) return false;

    for (int hy = ay; hy < ay + extent; ++hy) {
        for (int hx = ax; hx < ax + extent; ++hx) {
            for (int wy = std::max(0, hy - 4); wy <= std::min(kHeight - 1, hy + 4); ++wy) {
                for (int wx = std::max(0, hx - 4); wx <= std::min(kWidth - 1, hx + 4); ++wx) {
                    if (std::abs(wx - hx) + std::abs(wy - hy) > 4) continue;
                    if (tile(wx, wy).structure == Structure::Well) return true;
                }
            }
        }
    }
    return false;
}

int World::house_capacity(int x, int y) const {
    int ax = -1, ay = -1;
    if (!canonical_house(x, y, ax, ay)) return 0;
    const Tile& anchor = tile(ax, ay);
    const int count = std::max(1, static_cast<int>(anchor.residence_tiles));
    return base_house_capacity(anchor.housing_level) * count + (count > 1 ? 8 : 0);
}

int World::house_desirability(int x, int y) const {
    int ax = -1, ay = -1;
    if (!canonical_house(x, y, ax, ay)) return 0;
    const int extent = residence_extent(ax, ay);
    int total = 0;
    int members = 0;

    for (int oy = 0; oy < extent; ++oy) {
        for (int ox = 0; ox < extent; ++ox) {
            const int hx = ax + ox, hy = ay + oy;
            int score = 0;
            for (int yy = std::max(0, hy - 4); yy <= std::min(kHeight - 1, hy + 4); ++yy) {
                for (int xx = std::max(0, hx - 4); xx <= std::min(kWidth - 1, hx + 4); ++xx) {
                    const int distance = std::abs(xx - hx) + std::abs(yy - hy);
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
            total += std::clamp(score, -20, 20);
            ++members;
        }
    }
    return members > 0 ? std::clamp(total / members, -20, 20) : 0;
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
    int ax = -1, ay = -1;
    if (!canonical_house(x, y, ax, ay)) return "NOT A HOUSE";
    const Tile& h = tile(ax, ay);
    if (!has_road_access(ax, ay)) return "NEEDS ROAD ACCESS";
    if (h.population == 0) return "WAITING FOR SETTLERS";
    if (h.food_stock == 0 && pending_goods_for(ax, ay, Resource::Food) == 0) return "NEEDS A RELIABLE FOOD SUPPLY";
    if (!has_well_service(ax, ay)) return "NEEDS WATER FROM A NEARBY WELL";
    if (h.housing_level == 0 && h.housing_service_ticks < 4) return "FOOD AND WATER ARE STABILISING";
    if (h.housing_level == 1 && h.housing_service_ticks < 12) return "SUSTAIN FOOD AND WATER TO EVOLVE";
    if (h.housing_level == 2 && h.pottery_stock == 0) return "NEEDS POTTERY FROM A MARKET";
    if (h.housing_level == 2 && h.housing_goods_ticks < 6) return "POTTERY SUPPLY IS STABILISING";
    if (h.housing_level >= 3 && house_desirability(ax, ay) < 3) return "NEEDS A MORE DESIRABLE NEIGHBOURHOOD";
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
        if (t.structure == Structure::Farm && active_workers_for_job(x, y) > 0) {
            t.food_stock = static_cast<std::uint16_t>(std::min<int>(32, t.food_stock + 2));
        }
    }
}

void World::produce_clay() {
    if ((ticks_ % 4U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        Tile& t = tile(x, y);
        if (t.structure == Structure::ClayPit && active_workers_for_job(x, y) > 0) {
            t.clay_stock = static_cast<std::uint16_t>(std::min<int>(32, t.clay_stock + 2));
        }
    }
}

void World::produce_pottery() {
    if ((ticks_ % 4U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        Tile& t = tile(x, y);
        if (t.structure != Structure::Potter || active_workers_for_job(x, y) == 0) continue;
        if (t.clay_stock >= 2 && t.pottery_stock <= 22) {
            t.clay_stock = static_cast<std::uint16_t>(t.clay_stock - 2);
            t.pottery_stock = static_cast<std::uint16_t>(t.pottery_stock + 2);
        }
    }
}

int World::pending_goods_for(int target_x, int target_y, Resource resource) const {
    int query_x = target_x, query_y = target_y;
    (void)canonical_house(target_x, target_y, query_x, query_y);

    int total = 0;
    for (const auto& g : goods_agents_) {
        int gx = g.target_x, gy = g.target_y;
        (void)canonical_house(g.target_x, g.target_y, gx, gy);
        if (gx == query_x && gy == query_y && g.resource == resource) total += g.amount;
    }
    return total;
}

bool World::spawn_goods_agent(int source_x, int source_y, int target_x, int target_y, Resource resource, int amount) {
    if (amount <= 0 || goods_agents_.size() >= 128U) return false;

    int tx = target_x, ty = target_y;
    (void)canonical_house(target_x, target_y, tx, ty);
    auto path = road_path_between(source_x, source_y, tx, ty);
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
    g.target_x = tx; g.target_y = ty;
    g.resource = resource; g.amount = static_cast<std::uint8_t>(amount);
    g.road_path = std::move(path);
    const auto start = g.road_path.front();
    g.x = static_cast<int>(start % kWidth); g.y = static_cast<int>(start / kWidth);
    record_road_traffic(g.x, g.y);
    goods_agents_.push_back(std::move(g));
    return true;
}

void World::move_food_to_granaries() {
    for (int gy = 0; gy < kHeight; ++gy) for (int gx = 0; gx < kWidth; ++gx) {
        Tile& granary = tile(gx, gy);
        if (granary.structure != Structure::Granary || active_workers_for_job(gx, gy) == 0) continue;
        const int incoming = pending_goods_for(gx, gy, Resource::Food);
        const int space = 96 - static_cast<int>(granary.food_stock) - incoming;
        if (space <= 0) continue;
        bool dispatched = false;
        for (int sy = 0; sy < kHeight && !dispatched; ++sy) for (int sx = 0; sx < kWidth && !dispatched; ++sx) {
            Tile& source = tile(sx, sy);
            if ((source.structure != Structure::Farm && source.structure != Structure::HuntingLodge) || source.food_stock == 0) continue;
            dispatched = spawn_goods_agent(sx, sy, gx, gy, Resource::Food, std::min<int>({6, source.food_stock, space}));
        }
    }
}

void World::move_food_to_markets() {
    for (int my = 0; my < kHeight; ++my) for (int mx = 0; mx < kWidth; ++mx) {
        Tile& market = tile(mx, my);
        if (market.structure != Structure::Market || active_workers_for_job(mx, my) == 0) continue;
        const int incoming = pending_goods_for(mx, my, Resource::Food);
        const int space = 32 - static_cast<int>(market.food_stock) - incoming;
        if (space <= 0) continue;
        bool dispatched = false;
        for (int gy = 0; gy < kHeight && !dispatched; ++gy) for (int gx = 0; gx < kWidth && !dispatched; ++gx) {
            Tile& granary = tile(gx, gy);
            if (granary.structure != Structure::Granary || granary.food_stock == 0 || active_workers_for_job(gx, gy) == 0) continue;
            dispatched = spawn_goods_agent(gx, gy, mx, my, Resource::Food, std::min<int>({4, granary.food_stock, space}));
        }
    }
}

void World::feed_houses() {
    for (int hy = 0; hy < kHeight; ++hy) for (int hx = 0; hx < kWidth; ++hx) {
        if (tile(hx, hy).structure != Structure::House || !is_residence_anchor(hx, hy)) continue;
        Tile& house = tile(hx, hy);
        const bool road = has_road_access(hx, hy);
        const int incoming = pending_goods_for(hx, hy, Resource::Food);
        const int food_capacity = 12 * residence_tiles(hx, hy);
        const int space = food_capacity - static_cast<int>(house.food_stock) - incoming;
        if (road && space > 0) {
            bool dispatched = false;
            for (int my = 0; my < kHeight && !dispatched; ++my) for (int mx = 0; mx < kWidth && !dispatched; ++mx) {
                Tile& market = tile(mx, my);
                if (market.structure != Structure::Market || market.food_stock == 0 || active_workers_for_job(mx, my) == 0) continue;
                if (!within_delivery_range(hx, hy, mx, my)) continue;
                dispatched = spawn_goods_agent(mx, my, hx, hy, Resource::Food,
                    std::min<int>({2, market.food_stock, space}));
            }
        }

        if ((ticks_ % 4U) == 0U && house.population > 0 && house.food_stock > 0) {
            const int demand = std::max(1, (static_cast<int>(house.population) + 7) / 8);
            house.food_stock = static_cast<std::uint16_t>(house.food_stock - std::min<int>(house.food_stock, demand));
        }
        if (!road) {
            if ((ticks_ % 12U) == 0U && house.population > house.employed) --house.population;
            continue;
        }
        if (house.food_stock == 0 && pending_goods_for(hx, hy, Resource::Food) == 0 &&
            (ticks_ % 24U) == 0U && house.population > house.employed) {
            --house.population;
        }
    }
}

void World::consume_household_goods() {
    if ((ticks_ % 24U) != 0U) return;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        if (tile(x, y).structure != Structure::House || !is_residence_anchor(x, y)) continue;
        Tile& h = tile(x, y);
        if (h.population > 0 && h.pottery_stock > 0) --h.pottery_stock;
    }
}

void World::update_housing() {
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        if (tile(x, y).structure != Structure::House || !is_residence_anchor(x, y)) continue;
        Tile& house = tile(x, y);
        const bool food_service = house.food_stock > 0 || pending_goods_for(x, y, Resource::Food) > 0;
        const bool supported = house.population > 0 && has_road_access(x, y) && food_service && has_well_service(x, y);
        if (supported) {
            if (house.housing_service_ticks < 24) ++house.housing_service_ticks;
            if (house.housing_level == 0 && house.housing_service_ticks >= 4) house.housing_level = 1;
            if (house.housing_level == 1 && house.housing_service_ticks >= 12) house.housing_level = 2;
            if (house.housing_level >= 2 && house.pottery_stock > 0) {
                if (house.housing_goods_ticks < 12) ++house.housing_goods_ticks;
                if (house.housing_level == 2 && house.housing_goods_ticks >= 6) house.housing_level = 3;
            } else if (house.housing_goods_ticks > 0) --house.housing_goods_ticks;
        } else {
            if ((ticks_ % 4U) == 0U && house.housing_service_ticks > 0) --house.housing_service_ticks;
            if ((ticks_ % 4U) == 0U && house.housing_goods_ticks > 0) --house.housing_goods_ticks;
            if ((ticks_ % 8U) == 0U && house.housing_service_ticks == 0 && house.housing_level > 0) --house.housing_level;
        }
        if (house.housing_level >= 3 && house.pottery_stock == 0 && house.housing_goods_ticks == 0 &&
            (ticks_ % 16U) == 0U) {
            house.housing_level = 2;
        }
        const int capacity = house_capacity(x, y);
        if (house.population > capacity && house.population > house.employed) --house.population;
        sync_residence_members(x, y);
    }
}

std::vector<std::size_t> World::immigration_path_to(int house_x, int house_y) const {
    int hx = house_x, hy = house_y;
    (void)canonical_house(house_x, house_y, hx, hy);
    const auto goals = adjacent_roads(hx, hy);
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
    int qx = house_x, qy = house_y;
    (void)canonical_house(house_x, house_y, qx, qy);
    int pending = 0;
    for (const auto& a : immigrants_) {
        int ax = a.target_x, ay = a.target_y;
        (void)canonical_house(a.target_x, a.target_y, ax, ay);
        if (ax == qx && ay == qy) pending += a.group_size;
    }
    return pending;
}

void World::create_immigration() {
    if ((ticks_ % 2U) != 0U || immigrants_.size() >= 32U) return;
    int groups = 0;
    for (int hy = 0; hy < kHeight && groups < 4; ++hy) for (int hx = 0; hx < kWidth && groups < 4; ++hx) {
        const Tile& house = tile(hx, hy);
        if (house.structure != Structure::House || !is_residence_anchor(hx, hy) || !has_road_access(hx, hy)) continue;
        const bool supplied = house.food_stock > 0 || pending_goods_for(hx, hy, Resource::Food) > 0;
        const int pioneer_limit = std::min(house_capacity(hx, hy), 4 * residence_tiles(hx, hy));
        const int attraction_capacity = supplied ? house_capacity(hx, hy) : pioneer_limit;
        const int vacancies = attraction_capacity - static_cast<int>(house.population) - pending_immigrants_for(hx, hy);
        if (vacancies <= 0) continue;
        auto path = immigration_path_to(hx, hy);
        if (path.empty()) continue;
        const int group = std::min(3, vacancies);
        const auto start = path.front();
        ImmigrantAgent a;
        a.x = static_cast<int>(start % kWidth); a.y = static_cast<int>(start / kWidth);
        a.target_x = hx; a.target_y = hy; a.group_size = static_cast<std::uint8_t>(group);
        a.road_path = std::move(path);
        record_road_traffic(a.x, a.y, group);
        immigrants_.push_back(std::move(a));
        ++groups;
    }
}

void World::move_immigrants() {
    for (std::size_t i = 0; i < immigrants_.size();) {
        ImmigrantAgent& a = immigrants_[i];
        bool remove = false;
        int tx = a.target_x, ty = a.target_y;
        if (!canonical_house(a.target_x, a.target_y, tx, ty) || !has_road_access(tx, ty)) remove = true;
        else {
            a.target_x = tx;
            a.target_y = ty;
        }

        if (!remove && a.path_position + 1 < a.road_path.size()) {
            const auto next = a.road_path[++a.path_position];
            const int nx = static_cast<int>(next % kWidth), ny = static_cast<int>(next / kWidth);
            if (tile(nx, ny).structure != Structure::Road) remove = true;
            else { a.x = nx; a.y = ny; record_road_traffic(nx, ny, a.group_size); }
        } else if (!remove) {
            Tile& h = tile(tx, ty);
            const int space = house_capacity(tx, ty) - static_cast<int>(h.population);
            if (space > 0) h.population = static_cast<std::uint8_t>(h.population + std::min<int>(space, a.group_size));
            remove = true;
        }

        if (remove) immigrants_.erase(immigrants_.begin() + static_cast<std::ptrdiff_t>(i)); else ++i;
    }
}

int World::find_wildlife_target(int job_x, int job_y) const {
    int best_id = -1;
    std::size_t best_path = std::numeric_limits<std::size_t>::max();
    for (const auto& animal : wildlife_) {
        bool reserved = false;
        for (const auto& worker : workers_) {
            if (worker.role == WorkerRole::Hunter && worker.wildlife_target_id == animal.id) { reserved = true; break; }
        }
        if (reserved) continue;
        if (std::abs(animal.x - job_x) + std::abs(animal.y - job_y) > 20) continue;
        const auto path = hunting_path_from(job_x, job_y, animal.id);
        if (!path.empty() && path.size() < best_path) {
            best_path = path.size();
            best_id = animal.id;
        }
    }
    return best_id;
}

std::vector<std::size_t> World::hunting_path_from(int job_x, int job_y, int wildlife_id) const {
    const WildlifeAgent* target = wildlife_by_id(wildlife_id);
    if (!target || !in_bounds(job_x, job_y)) return {};
    const std::size_t start = index(job_x, job_y);
    const std::size_t goal = index(target->x, target->y);
    if (start == goal) return {};

    std::vector<std::uint8_t> seen(tiles_.size(), 0);
    std::vector<int> previous(tiles_.size(), -1);
    std::queue<std::size_t> q;
    q.push(start);
    seen[start] = 1;
    static constexpr int dx[4] = {1, -1, 0, 0};
    static constexpr int dy[4] = {0, 0, 1, -1};

    while (!q.empty() && !seen[goal]) {
        const auto current = q.front(); q.pop();
        const int x = static_cast<int>(current % kWidth), y = static_cast<int>(current / kWidth);
        for (int i = 0; i < 4; ++i) {
            const int nx = x + dx[i], ny = y + dy[i];
            if (!in_bounds(nx, ny)) continue;
            const auto ni = index(nx, ny);
            if (seen[ni]) continue;
            const Tile& t = tile(nx, ny);
            if (t.terrain == Terrain::Water) continue;
            if (ni != goal && t.structure != Structure::Empty && t.structure != Structure::Road) continue;
            seen[ni] = 1;
            previous[ni] = static_cast<int>(current);
            q.push(ni);
        }
    }
    if (!seen[goal]) return {};

    std::vector<std::size_t> reverse;
    for (std::size_t cur = goal; cur != start;) {
        reverse.push_back(cur);
        const int p = previous[cur];
        if (p < 0) return {};
        cur = static_cast<std::size_t>(p);
    }
    std::reverse(reverse.begin(), reverse.end());
    return reverse;
}

void World::recruit_workers() {
    if ((ticks_ % 4U) != 0U) return;
    for (int jy = 0; jy < kHeight; ++jy) for (int jx = 0; jx < kWidth; ++jx) {
        const Structure job_structure = tile(jx, jy).structure;
        if (!is_workplace(job_structure) || !has_road_access(jx, jy)) continue;
        if (assigned_workers_for_job(jx, jy) >= desired_workers_for(job_structure)) continue;

        bool hired = false;
        for (int hy = 0; hy < kHeight && !hired; ++hy) for (int hx = 0; hx < kWidth && !hired; ++hx) {
            Tile& home = tile(hx, hy);
            if (home.structure != Structure::House || !is_residence_anchor(hx, hy) ||
                home.population <= home.employed || !has_road_access(hx, hy)) continue;
            auto path = road_path_between(hx, hy, jx, jy);
            if (path.empty()) continue;

            WorkerAgent w;
            w.home_x = hx; w.home_y = hy; w.job_x = jx; w.job_y = jy;
            w.role = role_for(job_structure);
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

void World::start_commute_home(WorkerAgent& w) {
    w.path = road_path_between(w.home_x, w.home_y, w.job_x, w.job_y);
    if (w.path.empty()) return;
    std::reverse(w.path.begin(), w.path.end());
    w.path_position = 0;
    w.state = WorkerState::CommutingHome;
}

void World::move_workers() {
    for (std::size_t i = 0; i < workers_.size();) {
        WorkerAgent& w = workers_[i];
        bool remove = false;
        int home_x = w.home_x, home_y = w.home_y;
        if (!canonical_house(w.home_x, w.home_y, home_x, home_y) ||
            !in_bounds(w.job_x, w.job_y) || !is_workplace(tile(w.job_x, w.job_y).structure) ||
            role_for(tile(w.job_x, w.job_y).structure) != w.role) {
            remove = true;
        } else {
            w.home_x = home_x;
            w.home_y = home_y;
        }

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
                        if (w.role == WorkerRole::Hunter) {
                            w.wildlife_target_id = find_wildlife_target(w.job_x, w.job_y);
                            w.path = hunting_path_from(w.job_x, w.job_y, w.wildlife_target_id);
                            w.path_position = 0;
                            if (w.wildlife_target_id < 0 || w.path.empty()) {
                                w.wildlife_target_id = -1;
                                w.work_ticks = 4;
                                w.state = WorkerState::WorkingAtJob;
                            } else w.state = WorkerState::HunterOutbound;
                        } else {
                            w.work_ticks = 12;
                            w.state = WorkerState::WorkingAtJob;
                        }
                    }
                    break;

                case WorkerState::WorkingAtJob:
                    if (w.work_ticks > 0) --w.work_ticks;
                    if (w.work_ticks == 0) {
                        start_commute_home(w);
                        if (w.path.empty()) remove = true;
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
                    if (w.work_ticks == 0) {
                        w.payload_food = harvest_wildlife(w.wildlife_target_id) ? 5 : 0;
                        w.wildlife_target_id = -1;
                        w.state = WorkerState::HunterReturning;
                    }
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
                        start_commute_home(w);
                        if (w.path.empty()) remove = true;
                    }
                    break;

                case WorkerState::CommutingHome:
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

void World::queue_goods_deliveries() {
    if (goods_agents_.size() >= 128U) return;

    for (int py = 0; py < kHeight; ++py) for (int px = 0; px < kWidth; ++px) {
        Tile& potter = tile(px, py);
        if (potter.structure != Structure::Potter || active_workers_for_job(px, py) == 0) continue;
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
        if (market.structure != Structure::Market || active_workers_for_job(mx, my) == 0) continue;
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
        if (house.structure != Structure::House || !is_residence_anchor(hx, hy) || !has_road_access(hx, hy)) continue;
        const int incoming = pending_goods_for(hx, hy, Resource::Pottery);
        const int target = 4 * residence_tiles(hx, hy);
        if (static_cast<int>(house.pottery_stock) + incoming >= target) continue;
        bool dispatched = false;
        for (int my = 0; my < kHeight && !dispatched; ++my) for (int mx = 0; mx < kWidth && !dispatched; ++mx) {
            Tile& market = tile(mx, my);
            if (market.structure != Structure::Market || market.pottery_stock == 0 || active_workers_for_job(mx, my) == 0) continue;
            if (!within_delivery_range(hx, hy, mx, my)) continue;
            dispatched = spawn_goods_agent(mx, my, hx, hy, Resource::Pottery,
                std::min<int>(2, target - static_cast<int>(house.pottery_stock) - incoming));
        }
    }
}

void World::move_goods() {
    for (std::size_t i = 0; i < goods_agents_.size();) {
        GoodsAgent& g = goods_agents_[i];
        bool remove = false;

        int tx = g.target_x, ty = g.target_y;
        if (in_bounds(tx, ty) && tile(tx, ty).structure == Structure::House) {
            if (canonical_house(tx, ty, tx, ty)) {
                g.target_x = tx;
                g.target_y = ty;
            }
        }

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
                const int space = 12 * residence_tiles(g.target_x, g.target_y) - target.food_stock;
                target.food_stock = static_cast<std::uint16_t>(target.food_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Clay && target.structure == Structure::Potter) {
                const int space = 24 - target.clay_stock;
                target.clay_stock = static_cast<std::uint16_t>(target.clay_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Pottery && target.structure == Structure::Market) {
                const int space = 24 - target.pottery_stock;
                target.pottery_stock = static_cast<std::uint16_t>(target.pottery_stock + std::min<int>(space, g.amount));
            } else if (g.resource == Resource::Pottery && target.structure == Structure::House) {
                const int space = 6 * residence_tiles(g.target_x, g.target_y) - target.pottery_stock;
                target.pottery_stock = static_cast<std::uint16_t>(target.pottery_stock + std::min<int>(space, g.amount));
            }
            remove = true;
        }
        if (remove) goods_agents_.erase(goods_agents_.begin() + static_cast<std::ptrdiff_t>(i)); else ++i;
    }
}

void World::tick() {
    ++ticks_;
    move_wildlife();
    move_workers();
    move_goods();
    create_immigration();
    move_immigrants();
    recruit_workers();
    produce_food();
    produce_clay();
    produce_pottery();
    move_food_to_granaries();
    move_food_to_markets();
    feed_houses();
    consume_household_goods();
    update_housing();
    update_residence_merges();
    queue_goods_deliveries();
}

int World::population() const {
    int p = 0;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        if (tile(x, y).structure == Structure::House && !is_residence_anchor(x, y)) continue;
        p += tile(x, y).population;
    }
    return p;
}

int World::employed_population() const {
    int p = 0;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        if (tile(x, y).structure == Structure::House && !is_residence_anchor(x, y)) continue;
        p += tile(x, y).employed;
    }
    return p;
}

int World::total_food() const {
    int v = 0;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        if (tile(x, y).structure == Structure::House && !is_residence_anchor(x, y)) continue;
        v += tile(x, y).food_stock;
    }
    for (const auto& g : goods_agents_) if (g.resource == Resource::Food) v += g.amount;
    return v;
}

int World::total_clay() const {
    int v = 0;
    for (const auto& t : tiles_) v += t.clay_stock;
    for (const auto& g : goods_agents_) if (g.resource == Resource::Clay) v += g.amount;
    return v;
}

int World::total_pottery() const {
    int v = 0;
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        if (tile(x, y).structure == Structure::House && !is_residence_anchor(x, y)) continue;
        v += tile(x, y).pottery_stock;
    }
    for (const auto& g : goods_agents_) if (g.resource == Resource::Pottery) v += g.amount;
    return v;
}

int World::immigrants_in_transit() const {
    int n = 0;
    for (const auto& a : immigrants_) n += a.group_size;
    return n;
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
        case Structure::Well: return "WELL";
        case Structure::HuntingLodge: return "HUNTING LODGE";
    }
    return "?";
}

const char* World::worker_state_name(WorkerState s) {
    switch (s) {
        case WorkerState::CommutingToJob: return "TO JOB";
        case WorkerState::WorkingAtJob: return "WORKING";
        case WorkerState::HunterOutbound: return "HUNTER OUT";
        case WorkerState::Hunting: return "HUNTING";
        case WorkerState::HunterReturning: return "HUNTER RETURN";
        case WorkerState::CommutingHome: return "TO HOME";
        case WorkerState::RestingAtHome: return "HOME";
    }
    return "?";
}

const char* World::worker_role_name(WorkerRole r) {
    switch (r) {
        case WorkerRole::Hunter: return "HUNTER";
        case WorkerRole::Farmer: return "FARMER";
        case WorkerRole::ClayWorker: return "CLAY WORKER";
        case WorkerRole::Potter: return "POTTER";
        case WorkerRole::GranaryWorker: return "GRANARY WORKER";
        case WorkerRole::MarketWorker: return "MARKET WORKER";
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
