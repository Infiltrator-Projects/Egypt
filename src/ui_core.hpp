#include "input.hpp"

#include "common/image.hpp"
#include "isometric.hpp"
#include "world.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace egypt {

using Color = common::Color;
using ImageAsset = common::Image;

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    [[nodiscard]] bool contains(int px, int py) const {
        return px >= x && py >= y && px < x + w && py < y + h;
    }
};

class Framebuffer {
public:
    Framebuffer(int w, int h) { resize(w, h); }

    void resize(int w, int h) {
        w_ = std::max(1, w);
        h_ = std::max(1, h);
        pixels_.assign(static_cast<std::size_t>(w_ * h_), {0, 0, 0});
    }

    [[nodiscard]] int width() const { return w_; }
    [[nodiscard]] int height() const { return h_; }
    [[nodiscard]] const std::vector<Color>& pixels() const { return pixels_; }

    void clear(Color c) { std::fill(pixels_.begin(), pixels_.end(), c); }

    void pixel(int x, int y, Color c) {
        if (x >= 0 && y >= 0 && x < w_ && y < h_) {
            pixels_[static_cast<std::size_t>(y * w_ + x)] = c;
        }
    }

    void fill_span(int y, int x0, int x1, Color c) {
        if (y < 0 || y >= h_) return;
        if (x0 > x1) std::swap(x0, x1);
        x0 = std::max(0, x0);
        x1 = std::min(w_ - 1, x1);
        if (x0 > x1) return;
        auto first = pixels_.begin() + static_cast<std::ptrdiff_t>(y * w_ + x0);
        std::fill(first, first + (x1 - x0 + 1), c);
    }

    void fill_rect(Rect r, Color c) {
        const int x0 = std::max(0, r.x);
        const int x1 = std::min(w_, r.x + r.w);
        const int y0 = std::max(0, r.y);
        const int y1 = std::min(h_, r.y + r.h);
        if (x0 >= x1 || y0 >= y1) return;
        for (int y = y0; y < y1; ++y) {
            auto first = pixels_.begin() + static_cast<std::ptrdiff_t>(y * w_ + x0);
            std::fill(first, first + (x1 - x0), c);
        }
    }

    void blend_rect(Rect r, Color c, std::uint8_t alpha) {
        const int x0 = std::max(0, r.x);
        const int x1 = std::min(w_, r.x + r.w);
        const int y0 = std::max(0, r.y);
        const int y1 = std::min(h_, r.y + r.h);
        if (x0 >= x1 || y0 >= y1) return;
        const unsigned inv = 255U - alpha;
        for (int y = y0; y < y1; ++y) {
            Color* row = pixels_.data() + static_cast<std::size_t>(y * w_ + x0);
            for (int x = x0; x < x1; ++x, ++row) {
                row->r = static_cast<std::uint8_t>((row->r * inv + c.r * alpha) / 255U);
                row->g = static_cast<std::uint8_t>((row->g * inv + c.g * alpha) / 255U);
                row->b = static_cast<std::uint8_t>((row->b * inv + c.b * alpha) / 255U);
            }
        }
    }

    void line(int x0, int y0, int x1, int y1, Color c) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            pixel(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            const int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void rect(Rect r, Color c, int thickness = 1) {
        fill_rect({r.x, r.y, r.w, thickness}, c);
        fill_rect({r.x, r.y + r.h - thickness, r.w, thickness}, c);
        fill_rect({r.x, r.y, thickness, r.h}, c);
        fill_rect({r.x + r.w - thickness, r.y, thickness, r.h}, c);
    }

    void triangle(int x0, int y0, int x1, int y1, int x2, int y2, Color c) {
        struct V { int x; int y; };
        std::array<V,3> v{{{x0,y0},{x1,y1},{x2,y2}}};
        std::sort(v.begin(), v.end(), [](const V& a, const V& b) { return a.y < b.y; });
        if (v[0].y == v[2].y) {
            fill_span(v[0].y, std::min({v[0].x,v[1].x,v[2].x}), std::max({v[0].x,v[1].x,v[2].x}), c);
            return;
        }
        auto edge_x = [](const V& a, const V& b, int y) {
            if (a.y == b.y) return a.x;
            const long long num = static_cast<long long>(b.x - a.x) * (y - a.y);
            return a.x + static_cast<int>(num / (b.y - a.y));
        };
        const int from = std::max(0, v[0].y);
        const int to = std::min(h_ - 1, v[2].y);
        for (int y = from; y <= to; ++y) {
            const int xa = edge_x(v[0], v[2], y);
            const int xb = (y <= v[1].y) ? edge_x(v[0], v[1], y) : edge_x(v[1], v[2], y);
            fill_span(y, xa, xb, c);
        }
    }

    void quad(IsoPoint a, IsoPoint b, IsoPoint c, IsoPoint d, Color color) {
        triangle(a.x, a.y, b.x, b.y, c.x, c.y, color);
        triangle(a.x, a.y, c.x, c.y, d.x, d.y, color);
    }

    void diamond(IsoPoint p, int tw, int th, Color fill, Color edge) {
        const int hw = std::max(1, tw / 2);
        const int hh = std::max(1, th / 2);
        const int y0 = std::max(0, p.y - hh);
        const int y1 = std::min(h_ - 1, p.y + hh);
        for (int y = y0; y <= y1; ++y) {
            const int dy = std::abs(y - p.y);
            const int half = hw * (hh - std::min(hh, dy)) / hh;
            fill_span(y, p.x - half, p.x + half, fill);
        }
        if (edge.r != fill.r || edge.g != fill.g || edge.b != fill.b) diamond_outline(p, tw, th, edge);
    }

    void diamond_gradient(IsoPoint p, int tw, int th, Color top, Color right, Color bottom, Color left) {
        const int hw = std::max(1, tw / 2);
        const int hh = std::max(1, th / 2);
        const int y0 = std::max(0, p.y - hh);
        const int y1 = std::min(h_ - 1, p.y + hh);
        for (int y = y0; y <= y1; ++y) {
            const bool upper = y <= p.y;
            const int local = upper ? (y - (p.y - hh)) : (y - p.y);
            const int denom = hh;
            const int half = upper ? (hw * local / denom) : (hw * (hh - local) / denom);
            const Color lc = upper ? lerp(top, left, local, denom) : lerp(left, bottom, local, denom);
            const Color rc = upper ? lerp(top, right, local, denom) : lerp(right, bottom, local, denom);
            int xa = p.x - half;
            int xb = p.x + half;
            if (xa > xb) std::swap(xa, xb);
            const int clip_a = std::max(0, xa);
            const int clip_b = std::min(w_ - 1, xb);
            const int width = std::max(1, xb - xa);
            if (y < 0 || y >= h_ || clip_a > clip_b) continue;
            Color* dst = pixels_.data() + static_cast<std::size_t>(y * w_ + clip_a);
            for (int x = clip_a; x <= clip_b; ++x, ++dst) {
                const int t = x - xa;
                *dst = lerp(lc, rc, t, width);
            }
        }
    }

    void diamond_outline(IsoPoint p, int tw, int th, Color edge) {
        const IsoPoint top{p.x, p.y - th / 2};
        const IsoPoint right{p.x + tw / 2, p.y};
        const IsoPoint bottom{p.x, p.y + th / 2};
        const IsoPoint left{p.x - tw / 2, p.y};
        line(top.x, top.y, right.x, right.y, edge);
        line(right.x, right.y, bottom.x, bottom.y, edge);
        line(bottom.x, bottom.y, left.x, left.y, edge);
        line(left.x, left.y, top.x, top.y, edge);
    }

    Color sample(const ImageAsset& image, float x, float y) const {
        x = std::clamp(x, 0.0f, static_cast<float>(image.width - 1));
        y = std::clamp(y, 0.0f, static_cast<float>(image.height - 1));
        const int x0 = static_cast<int>(x), y0 = static_cast<int>(y);
        const int x1 = std::min(x0 + 1, image.width - 1), y1 = std::min(y0 + 1, image.height - 1);
        const float fx = x - x0, fy = y - y0;
        auto at = [&](int sx, int sy) { return image.pixels[static_cast<std::size_t>(sy * image.width + sx)]; };
        auto mix = [](std::uint8_t a, std::uint8_t b, float t) { return float(a) + (float(b) - float(a)) * t; };
        const Color a = at(x0, y0), b = at(x1, y0), c = at(x0, y1), d = at(x1, y1);
        return {
            static_cast<std::uint8_t>(std::clamp(mix(static_cast<std::uint8_t>(mix(a.r,b.r,fx)), static_cast<std::uint8_t>(mix(c.r,d.r,fx)), fy), 0.0f, 255.0f)),
            static_cast<std::uint8_t>(std::clamp(mix(static_cast<std::uint8_t>(mix(a.g,b.g,fx)), static_cast<std::uint8_t>(mix(c.g,d.g,fx)), fy), 0.0f, 255.0f)),
            static_cast<std::uint8_t>(std::clamp(mix(static_cast<std::uint8_t>(mix(a.b,b.b,fx)), static_cast<std::uint8_t>(mix(c.b,d.b,fx)), fy), 0.0f, 255.0f))
        };
    }

    void blit_cover(const ImageAsset& image) {
        if (!image.valid()) return;
        const float sx = float(w_) / image.width;
        const float sy = float(h_) / image.height;
        const float scale = std::max(sx, sy);
        const float dw = image.width * scale, dh = image.height * scale;
        const float ox = (w_ - dw) * 0.5f, oy = (h_ - dh) * 0.5f;
        for (int y = 0; y < h_; ++y) {
            for (int x = 0; x < w_; ++x) {
                pixels_[static_cast<std::size_t>(y * w_ + x)] = sample(image, (x - ox) / scale, (y - oy) / scale);
            }
        }
    }

private:
    static Color lerp(Color a, Color b, int t, int denom) {
        if (denom <= 0) return b;
        t = std::clamp(t, 0, denom);
        const int inv = denom - t;
        return {
            static_cast<std::uint8_t>((a.r * inv + b.r * t) / denom),
            static_cast<std::uint8_t>((a.g * inv + b.g * t) / denom),
            static_cast<std::uint8_t>((a.b * inv + b.b * t) / denom)
        };
    }

    int w_ = 1;
    int h_ = 1;
    std::vector<Color> pixels_;
};

using Glyph = std::array<std::uint8_t, 7>;
const std::unordered_map<char, Glyph> FONT = {
    {' ',{0,0,0,0,0,0,0}},{'A',{14,17,17,31,17,17,17}},{'B',{30,17,17,30,17,17,30}},
    {'C',{14,17,16,16,16,17,14}},{'D',{30,17,17,17,17,17,30}},{'E',{31,16,16,30,16,16,31}},
    {'F',{31,16,16,30,16,16,16}},{'G',{14,17,16,23,17,17,14}},{'H',{17,17,17,31,17,17,17}},
    {'I',{14,4,4,4,4,4,14}},{'J',{1,1,1,1,17,17,14}},{'K',{17,18,20,24,20,18,17}},
    {'L',{16,16,16,16,16,16,31}},{'M',{17,27,21,21,17,17,17}},{'N',{17,25,21,19,17,17,17}},
    {'O',{14,17,17,17,17,17,14}},{'P',{30,17,17,30,16,16,16}},{'Q',{14,17,17,17,21,18,13}},
    {'R',{30,17,17,30,20,18,17}},{'S',{15,16,16,14,1,1,30}},{'T',{31,4,4,4,4,4,4}},
    {'U',{17,17,17,17,17,17,14}},{'V',{17,17,17,17,17,10,4}},{'W',{17,17,17,21,21,21,10}},
    {'X',{17,17,10,4,10,17,17}},{'Y',{17,17,10,4,4,4,4}},{'Z',{31,1,2,4,8,16,31}},
    {'0',{14,17,19,21,25,17,14}},{'1',{4,12,4,4,4,4,14}},{'2',{14,17,1,2,4,8,31}},
    {'3',{30,1,1,14,1,1,30}},{'4',{2,6,10,18,31,2,2}},{'5',{31,16,16,30,1,1,30}},
    {'6',{14,16,16,30,17,17,14}},{'7',{31,1,2,4,8,8,8}},{'8',{14,17,17,14,17,17,14}},
    {'9',{14,17,17,15,1,1,14}},{'-',{0,0,0,31,0,0,0}},{'.',{0,0,0,0,0,12,12}},
    {':',{0,12,12,0,12,12,0}},{'/',{1,2,2,4,8,8,16}},{'?',{14,17,1,2,4,0,4}}
};

static void text(Framebuffer& fb, int x, int y, const std::string& value, Color color, int scale = 2) {
    int cursor = x;
    for (char raw : value) {
        const char ch = raw >= 'a' && raw <= 'z' ? static_cast<char>(raw - 'a' + 'A') : raw;
        const auto it = FONT.find(ch);
        const Glyph& glyph = it == FONT.end() ? FONT.at(' ') : it->second;
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if ((glyph[row] >> (4 - col)) & 1U) fb.fill_rect({cursor + col * scale, y + row * scale, scale, scale}, color);
            }
        }
        cursor += 6 * scale;
    }
}

enum class Screen { Menu, Game, Settings };
enum class Tool { Inspect, Road, House, Farm, Granary, Market, Well, HuntingLodge, ClayPit, Potter, Bulldoze };


} // namespace egypt
