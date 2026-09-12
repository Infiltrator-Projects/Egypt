#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <infiltratr/timing.h>

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
    int x, y, w, h;
    [[nodiscard]] bool contains(int px, int py) const {
        return px >= x && py >= y && px < x + w && py < y + h;
    }
};

enum class Screen { Menu, Game, Settings };
enum class Tool {
    Inspect,
    Road,
    House,
    Farm,
    Granary,
    Market,
    Well,
    HuntingLodge,
    ClayPit,
    Potter,
    Bulldoze
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

    void fill_rect(Rect r, Color c) {
        for (int y = std::max(0, r.y); y < std::min(h_, r.y + r.h); ++y) {
            for (int x = std::max(0, r.x); x < std::min(w_, r.x + r.w); ++x) pixel(x, y, c);
        }
    }

    void blend_rect(Rect r, Color c, std::uint8_t alpha) {
        const unsigned inv = 255U - alpha;
        for (int y = std::max(0, r.y); y < std::min(h_, r.y + r.h); ++y) {
            for (int x = std::max(0, r.x); x < std::min(w_, r.x + r.w); ++x) {
                Color& d = pixels_[static_cast<std::size_t>(y * w_ + x)];
                d.r = static_cast<std::uint8_t>((d.r * inv + c.r * alpha) / 255U);
                d.g = static_cast<std::uint8_t>((d.g * inv + c.g * alpha) / 255U);
                d.b = static_cast<std::uint8_t>((d.b * inv + c.b * alpha) / 255U);
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
        const int minx = std::max(0, std::min({x0, x1, x2}));
        const int maxx = std::min(w_ - 1, std::max({x0, x1, x2}));
        const int miny = std::max(0, std::min({y0, y1, y2}));
        const int maxy = std::min(h_ - 1, std::max({y0, y1, y2}));
        auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
            return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
        };
        for (int y = miny; y <= maxy; ++y) {
            for (int x = minx; x <= maxx; ++x) {
                const int a = edge(x0, y0, x1, y1, x, y);
                const int b = edge(x1, y1, x2, y2, x, y);
                const int d = edge(x2, y2, x0, y0, x, y);
                if ((a >= 0 && b >= 0 && d >= 0) || (a <= 0 && b <= 0 && d <= 0)) pixel(x, y, c);
            }
        }
    }

    void quad(IsoPoint a, IsoPoint b, IsoPoint c, IsoPoint d, Color color) {
        triangle(a.x, a.y, b.x, b.y, c.x, c.y, color);
        triangle(a.x, a.y, c.x, c.y, d.x, d.y, color);
    }

    void diamond(IsoPoint p, int tw, int th, Color fill, Color edge) {
        const IsoPoint top{p.x, p.y - th / 2};
        const IsoPoint right{p.x + tw / 2, p.y};
        const IsoPoint bottom{p.x, p.y + th / 2};
        const IsoPoint left{p.x - tw / 2, p.y};
        triangle(top.x, top.y, right.x, right.y, bottom.x, bottom.y, fill);
        triangle(top.x, top.y, bottom.x, bottom.y, left.x, left.y, fill);
        diamond_outline(p, tw, th, edge);
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
    {':',{0,12,12,0,12,12,0}},{'/',{1,2,2,4,8,8,16}}
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

class Game {
public:
    explicit Game(Framebuffer& fb)
        : fb_(fb), menu_(common::load_e16("build/assets/menu.e16")), view_w_(fb.width()), view_h_(fb.height()) {
        if (!menu_.valid()) std::cerr << "Egypt: menu artwork failed to load\n";
    }

    [[nodiscard]] bool running() const { return running_; }
    [[nodiscard]] bool dirty() const { return dirty_; }
    void rendered() { dirty_ = false; }

    void resize() {
        const int dx = fb_.width() - view_w_;
        const int dy = fb_.height() - view_h_;
        cam_.origin_x += dx / 2;
        cam_.origin_y += dy / 3;
        view_w_ = fb_.width();
        view_h_ = fb_.height();
        dirty_ = true;
        update_hover();
    }

    void tick() {
        if (screen_ == Screen::Game) {
            world_.tick();
            dirty_ = true;
        }
    }

    void frame(double dt) {
        if (screen_ != Screen::Game || dragging_ || !edge_scroll_ || over_game_ui(mx_, my_)) return;
        const int margin = 18;
        double dx = 0.0, dy = 0.0;
        if (mx_ <= margin) dx += scroll_speed_px_ * dt;
        else if (mx_ >= fb_.width() - 1 - margin) dx -= scroll_speed_px_ * dt;
        if (my_ <= 78 + margin) dy += scroll_speed_px_ * dt;
        else if (my_ >= fb_.height() - 1 - margin) dy -= scroll_speed_px_ * dt;
        if (dx != 0.0 || dy != 0.0) {
            cam_.pan_x += static_cast<int>(std::lround(dx));
            cam_.pan_y += static_cast<int>(std::lround(dy));
            update_hover();
            dirty_ = true;
        }
    }

    bool take_resize_request(int& w, int& h) {
        if (!resize_pending_) return false;
        w = requested_w_;
        h = requested_h_;
        resize_pending_ = false;
        return true;
    }

    void on_key(KeySym key) {
        if (key == XK_Escape) {
            if (screen_ == Screen::Game || screen_ == Screen::Settings) screen_ = Screen::Menu;
            else running_ = false;
            dragging_ = false;
            dirty_ = true;
            return;
        }
        if (screen_ == Screen::Settings || screen_ != Screen::Game) return;
        const int step = 48;
        if (key == XK_Left || key == XK_a || key == XK_A) cam_.pan_x += step;
        else if (key == XK_Right || key == XK_d || key == XK_D) cam_.pan_x -= step;
        else if (key == XK_Up || key == XK_w || key == XK_W) cam_.pan_y += step;
        else if (key == XK_Down || key == XK_s || key == XK_S) cam_.pan_y -= step;
        else if (key == XK_plus || key == XK_equal) zoom_by(10);
        else if (key == XK_minus) zoom_by(-10);
        else if (key == XK_Home) reset_camera();
        else if (key >= XK_1 && key <= XK_9) tool_ = static_cast<Tool>(key - XK_1);
        update_hover();
        dirty_ = true;
    }

    void on_motion(int x, int y) {
        const int oldx = hover_x_, oldy = hover_y_;
        if (dragging_) {
            cam_.pan_x += x - drag_last_x_;
            cam_.pan_y += y - drag_last_y_;
            drag_last_x_ = x;
            drag_last_y_ = y;
        }
        mx_ = x;
        my_ = y;
        update_hover();
        if (dragging_ || oldx != hover_x_ || oldy != hover_y_ || screen_ == Screen::Menu || screen_ == Screen::Settings) dirty_ = true;
    }

    void on_button_press(unsigned button, int x, int y) {
        mx_ = x;
        my_ = y;
        if (screen_ == Screen::Game && (button == Button4 || button == Button5)) {
            zoom_by(button == Button4 ? 10 : -10);
            update_hover();
            dirty_ = true;
            return;
        }
        if (screen_ == Screen::Game && (button == Button2 || button == Button3) && drag_pan_) {
            dragging_ = true;
            drag_button_ = button;
            drag_last_x_ = x;
            drag_last_y_ = y;
            dirty_ = true;
            return;
        }
        if (button == Button1) on_click(x, y);
    }

    void on_button_release(unsigned button, int x, int y) {
        mx_ = x;
        my_ = y;
        if (dragging_ && button == drag_button_) {
            dragging_ = false;
            update_hover();
            dirty_ = true;
        }
    }

    void on_click(int x, int y) {
        mx_ = x;
        my_ = y;
        if (screen_ == Screen::Menu) {
            const int choice = menu_button_at(x, y);
            if (choice == 0) { screen_ = Screen::Game; status_.clear(); reset_camera(); }
            else if (choice == 2) screen_ = Screen::Settings;
            else if (choice == 3) running_ = false;
            dirty_ = true;
            return;
        }
        if (screen_ == Screen::Settings) { handle_settings_click(x, y); return; }
        if (main_menu_rect().contains(x, y)) { screen_ = Screen::Menu; dirty_ = true; return; }
        const auto tools = tool_buttons();
        for (int i = 0; i < static_cast<int>(tools.size()); ++i) {
            if (tools[i].contains(x, y)) {
                tool_ = static_cast<Tool>(i);
                dirty_ = true;
                return;
            }
        }
        update_hover();
        if (!world_.in_bounds(hover_x_, hover_y_)) return;
        selected_x_ = hover_x_;
        selected_y_ = hover_y_;
        if (tool_ == Tool::Bulldoze) world_.bulldoze(selected_x_, selected_y_);
        else if (tool_ != Tool::Inspect) world_.place(tool_structure(), selected_x_, selected_y_);
        dirty_ = true;
    }

    void draw() {
        if (screen_ == Screen::Menu) draw_menu();
        else if (screen_ == Screen::Settings) draw_settings();
        else draw_game();
    }

private:
    Framebuffer& fb_;
    ImageAsset menu_;
    World world_;
    IsoCamera cam_;
    Screen screen_ = Screen::Menu;
    Tool tool_ = Tool::Inspect;
    bool running_ = true, dirty_ = true;
    int mx_ = 0, my_ = 0, hover_x_ = -1, hover_y_ = -1, selected_x_ = -1, selected_y_ = -1;
    std::string status_ = "PRE-ALPHA - NATIVE ENGINE";
    int view_w_ = 0, view_h_ = 0;
    bool edge_scroll_ = true, drag_pan_ = true, dragging_ = false;
    unsigned drag_button_ = 0;
    int drag_last_x_ = 0, drag_last_y_ = 0;
    double scroll_speed_px_ = 620.0;
    bool resize_pending_ = false;
    int requested_w_ = 1280, requested_h_ = 720, resolution_index_ = 0;

    const Color gold{217,177,95}, pale{242,215,154}, panel{19,14,12}, hi{55,40,27};
    static constexpr std::array<std::array<int,2>,3> kResolutions{{{{1280,720}},{{1600,900}},{{1920,1080}}}};

    void zoom_by(int delta) { cam_.zoom_percent = std::clamp(cam_.zoom_percent + delta, 50, 180); }
    void reset_camera() { cam_.origin_x = fb_.width()/2 + 80; cam_.origin_y = 150; cam_.pan_x = -260; cam_.pan_y = -30; cam_.zoom_percent = 90; update_hover(); }

    bool over_game_ui(int x, int y) const {
        if (y < 78 || y >= fb_.height() - 104) return true;
        if (main_menu_rect().contains(x, y)) return true;
        return Rect{fb_.width() - 360, 82, 350, 154}.contains(x, y);
    }

    std::array<Rect,4> menu_buttons() const {
        const int x = 72, y = 300, w = 340, h = 50, gap = 12;
        return {Rect{x,y,w,h}, Rect{x,y+h+gap,w,h}, Rect{x,y+2*(h+gap),w,h}, Rect{x,y+3*(h+gap),w,h}};
    }

    int menu_button_at(int x, int y) const {
        const auto buttons = menu_buttons();
        for (int i=0;i<4;++i) if (buttons[i].contains(x,y)) return i;
        return -1;
    }

    Rect main_menu_rect() const { return {fb_.width() - 210, 18, 190, 42}; }

    std::array<Rect,11> tool_buttons() const {
        std::array<Rect,11> result{};
        const int w = 112, h = 36, gap = 6;
        const int first_y = fb_.height() - 92;
        for (int i=0;i<6;++i) result[i] = {14 + i * (w + gap), first_y, w, h};
        for (int i=6;i<11;++i) result[i] = {14 + (i-6) * (w + gap), first_y + 42, w, h};
        return result;
    }

    Rect settings_back() const { return {fb_.width()/2 - 100, fb_.height() - 92, 200, 44}; }
    std::array<Rect,3> resolution_buttons() const { const int cx=fb_.width()/2; return {Rect{cx-330,240,200,44},Rect{cx-100,240,200,44},Rect{cx+130,240,200,44}}; }
    Rect edge_scroll_rect() const { return {fb_.width()/2 - 165, 338, 330, 44}; }
    std::array<Rect,3> speed_buttons() const { const int cx=fb_.width()/2; return {Rect{cx-260,440,160,44},Rect{cx-80,440,160,44},Rect{cx+100,440,160,44}}; }

    void button(Rect r, const std::string& label, bool enabled, bool active = false) {
        fb_.blend_rect(r, active ? hi : panel, active ? 235 : 205);
        fb_.rect(r, enabled ? gold : Color{90,78,65}, 2);
        text(fb_, r.x + 7, r.y + 11, label, enabled ? pale : Color{115,105,92}, 2);
    }

    Structure tool_structure() const {
        switch (tool_) {
            case Tool::Road: return Structure::Road;
            case Tool::House: return Structure::House;
            case Tool::Farm: return Structure::Farm;
            case Tool::Granary: return Structure::Granary;
            case Tool::Market: return Structure::Market;
            case Tool::Well: return Structure::Well;
            case Tool::HuntingLodge: return Structure::HuntingLodge;
            case Tool::ClayPit: return Structure::ClayPit;
            case Tool::Potter: return Structure::Potter;
            default: return Structure::Empty;
        }
    }

    const char* tool_name() const {
        switch (tool_) {
            case Tool::Inspect: return "INSPECT";
            case Tool::Road: return "ROAD";
            case Tool::House: return "HOUSE";
            case Tool::Farm: return "FARM";
            case Tool::Granary: return "GRANARY";
            case Tool::Market: return "MARKET";
            case Tool::Well: return "WELL";
            case Tool::HuntingLodge: return "HUNT LODGE";
            case Tool::ClayPit: return "CLAY PIT";
            case Tool::Potter: return "POTTER";
            case Tool::Bulldoze: return "BULLDOZE";
        }
        return "?";
    }

    void update_hover() {
        if (screen_ != Screen::Game) { hover_x_=hover_y_=-1; return; }
        int x=0,y=0;
        if (!cam_.pick(mx_,my_,x,y) || !world_.in_bounds(x,y)) { hover_x_=hover_y_=-1; return; }
        hover_x_=x;hover_y_=y;
    }

    Color terrain_color(Terrain terrain, int x, int y) const {
        Color c{};
        switch (terrain) {
            case Terrain::Desert: c={202,161,92}; break;
            case Terrain::Floodplain: c={124,145,78}; break;
            case Terrain::Water: c={38,104,129}; break;
            case Terrain::Clay: c={150,84,55}; break;
            case Terrain::Reeds: c={72,127,72}; break;
        }
        const int jitter=((x*17+y*31)%9)-4;
        auto add=[&](std::uint8_t v){return static_cast<std::uint8_t>(std::clamp(int(v)+jitter,0,255));};
        return {add(c.r),add(c.g),add(c.b)};
    }

    void draw_block(IsoPoint p, int tw, int th, int height, Color top, Color left, Color right) {
        const int hw=tw/2, hh=th/2;
        const IsoPoint bt{p.x,p.y-hh},br{p.x+hw,p.y},bb{p.x,p.y+hh},bl{p.x-hw,p.y};
        const IsoPoint tt{bt.x,bt.y-height},tr{br.x,br.y-height},tb{bb.x,bb.y-height},tl{bl.x,bl.y-height};
        fb_.quad(bl,bb,tb,tl,left);fb_.quad(bb,br,tr,tb,right);fb_.quad(tt,tr,tb,tl,top);
    }

    bool high_house(int x,int y) const {
        return world_.in_bounds(x,y) && world_.tile(x,y).structure==Structure::House && world_.tile(x,y).housing_level>=2;
    }

    bool merged_anchor(int x,int y) const {
        if (!high_house(x,y)||!high_house(x+1,y)||!high_house(x,y+1)||!high_house(x+1,y+1)) return false;
        const bool blocked_left = high_house(x-1,y) && high_house(x-1,y+1);
        const bool blocked_up = high_house(x,y-1) && high_house(x+1,y-1);
        return !blocked_left && !blocked_up;
    }

    bool merged_part(int x,int y) const {
        for(int ay=y-1;ay<=y;++ay) for(int ax=x-1;ax<=x;++ax) {
            if(!world_.in_bounds(ax,ay)) continue;
            if(merged_anchor(ax,ay) && !(ax==x&&ay==y) && x>=ax&&x<=ax+1&&y>=ay&&y<=ay+1) return true;
        }
        return false;
    }

    void draw_house(const Tile& tile, IsoPoint p, int tw, int th) {
        if(tile.housing_level==0) {
            const int h=tile.population>=5?19:15;
            draw_block(p,tw*2/3,th*2/3,h,{202,161,95},{129,78,46},{151,91,49});
            fb_.fill_rect({p.x-3,p.y-h+5,6,10},{72,48,34});
        } else if(tile.housing_level==1) {
            draw_block(p,tw*4/5,th*4/5,25,{220,188,122},{146,91,55},{173,108,58});
            fb_.fill_rect({p.x-4,p.y-19,8,13},{67,47,35});
            fb_.fill_rect({p.x-16,p.y-24,5,6},{52,91,108});
            fb_.fill_rect({p.x+11,p.y-24,5,6},{52,91,108});
            fb_.fill_rect({p.x-12,p.y-31,24,4},{224,190,123});
        } else {
            draw_block(p,tw*9/10,th*9/10,31,{229,201,144},{157,101,65},{185,119,67});
            fb_.fill_rect({p.x-5,p.y-23,10,16},{63,44,32});
            fb_.fill_rect({p.x-19,p.y-29,6,7},{52,94,113});
            fb_.fill_rect({p.x+13,p.y-29,6,7},{52,94,113});
            fb_.fill_rect({p.x-16,p.y-38,32,5},{237,211,157});
            fb_.fill_rect({p.x-2,p.y-45,4,8},{47,104,62});
        }
    }

    void draw_merged_residence(int x,int y,int tw,int th) {
        const IsoPoint a=cam_.project(x,y);
        const IsoPoint b=cam_.project(x+1,y+1);
        IsoPoint center{(a.x+b.x)/2,(a.y+b.y)/2+th/2};
        draw_block(center,tw*17/10,th*17/10,42,{235,209,156},{160,100,62},{192,122,68});
        fb_.fill_rect({center.x-8,center.y-31,16,22},{63,44,32});
        fb_.fill_rect({center.x-30,center.y-38,8,8},{54,101,119});
        fb_.fill_rect({center.x+22,center.y-38,8,8},{54,101,119});
        fb_.fill_rect({center.x-29,center.y-52,58,6},{239,216,167});
        fb_.fill_rect({center.x-3,center.y-61,6,9},{48,108,65});
    }

    void draw_farm(IsoPoint p,int tw,int th) {
        fb_.diamond(p,tw*9/10,th*9/10,{106,128,61},{106,128,61});
        for(int i=-3;i<=3;++i){const int ox=i*std::max(2,tw/12);fb_.line(p.x+ox-tw/5,p.y+th/6,p.x+ox+tw/5,p.y-th/6,{67,90,42});}
        fb_.fill_rect({p.x-3,p.y-14,6,13},{148,111,62});
    }

    void draw_granary(const Tile& tile,IsoPoint p,int tw,int th) {
        draw_block(p,tw*4/5,th*4/5,31,{216,184,117},{128,82,53},{157,99,58});
        fb_.fill_rect({p.x-15,p.y-37,30,5},{240,208,143});
        fb_.fill_rect({p.x-5,p.y-23,10,15},{75,53,37});
        const int sacks=std::min<int>(5,(tile.food_stock+19)/20);
        for(int i=0;i<sacks;++i)fb_.fill_rect({p.x-18+i*8,p.y-45,6,5},{222,177,72});
    }

    void draw_market(const Tile& tile,IsoPoint p,int tw,int th) {
        fb_.diamond(p,tw*4/5,th*4/5,{156,112,68},{156,112,68});
        const int y=p.y-25;fb_.line(p.x-16,p.y-3,p.x-16,y,{79,55,41});fb_.line(p.x+16,p.y-3,p.x+16,y,{79,55,41});
        fb_.fill_rect({p.x-21,y-5,42,7},{157,52,42});fb_.fill_rect({p.x-21,y+2,42,4},{221,185,102});
        for(int i=0;i<std::min<int>(5,tile.food_stock/4+1);++i)fb_.fill_rect({p.x-14+i*7,p.y-8,4,4},{199,158,55});
    }

    void draw_well(IsoPoint p,int tw,int th) {
        fb_.diamond(p,tw*2/3,th*2/3,{170,145,91},{128,100,63});
        fb_.diamond({p.x,p.y-2},tw/2,th/2,{55,119,145},{231,205,145});
        fb_.fill_rect({p.x-15,p.y-23,4,22},{104,70,44});
        fb_.fill_rect({p.x+11,p.y-23,4,22},{104,70,44});
        fb_.fill_rect({p.x-15,p.y-25,30,4},{182,132,69});
    }

    void draw_hunting_lodge(const Tile& tile,IsoPoint p,int tw,int th) {
        draw_block(p,tw*9/10,th*9/10,28,{171,119,67},{101,66,43},{128,79,45});
        fb_.triangle(p.x-22,p.y-29,p.x,p.y-45,p.x+22,p.y-29,{101,57,35});
        fb_.fill_rect({p.x-4,p.y-21,8,14},{67,44,31});
        if(tile.food_stock>0) fb_.fill_rect({p.x+14,p.y-13,8,6},{163,49,38});
    }

    void draw_structure_at(int x,int y,const Tile& tile,IsoPoint p,int tw,int th) {
        switch(tile.structure) {
            case Structure::Empty: break;
            case Structure::Road: fb_.diamond(p,tw*3/4,th*3/4,{136,109,74},{136,109,74}); break;
            case Structure::House:
                if(merged_part(x,y)) break;
                if(merged_anchor(x,y)) draw_merged_residence(x,y,tw,th); else draw_house(tile,p,tw,th);
                break;
            case Structure::Farm: draw_farm(p,tw,th); break;
            case Structure::Granary: draw_granary(tile,p,tw,th); break;
            case Structure::Market: draw_market(tile,p,tw,th); break;
            case Structure::Well: draw_well(p,tw,th); break;
            case Structure::HuntingLodge: draw_hunting_lodge(tile,p,tw,th); break;
            case Structure::ClayPit:
                fb_.diamond({p.x,p.y+2},tw*2/3,th/2,{91,54,43},{66,42,33});fb_.fill_rect({p.x-7,p.y-5,14,3},{183,113,72});break;
            case Structure::Potter:
                draw_block(p,tw*3/4,th*3/4,24,{165,122,79},{111,71,48},{133,82,51});fb_.fill_rect({p.x+8,p.y-39,5,18},{72,54,45});break;
        }
    }

    void draw_person(IsoPoint p,Color clothes,Color skin,int offset=0,bool carrying=false) {
        const int x=p.x+offset,y=p.y-8;
        fb_.fill_rect({x-2,y-7,5,5},skin);
        fb_.fill_rect({x-2,y-2,5,8},clothes);
        fb_.pixel(x-3,y+6,{52,39,31});fb_.pixel(x+3,y+6,{52,39,31});
        if(carrying) fb_.fill_rect({x+4,y-1,4,4},{180,55,42});
    }

    void draw_agents() {
        for(const auto& a:world_.immigrants()) {
            const IsoPoint p=cam_.project(a.x,a.y);
            for(int i=0;i<a.group_size;++i) draw_person(p,{185,122,76},{166,111,77},(i-1)*5,false);
        }
        for(const auto& w:world_.workers()) {
            const IsoPoint p=cam_.project(w.x,w.y);
            const bool hunter=w.state==WorkerState::HunterOutbound||w.state==WorkerState::Hunting||w.state==WorkerState::HunterReturning;
            const Color clothes=hunter?Color{79,101,49}:Color{186,145,91};
            draw_person(p,clothes,{171,112,76},0,w.payload_food>0);
        }
    }

    void draw_menu() {
        if(menu_.valid())fb_.blit_cover(menu_);else fb_.clear({8,18,35});
        fb_.blend_rect({38,30,520,610},{5,4,4},112);
        text(fb_,72,72,"EGYPT",gold,8);text(fb_,74,151,"A LIVING CITY ON THE NILE",pale,3);fb_.fill_rect({72,205,420,2},gold);
        const auto b=menu_buttons();button(b[0],"NEW GAME",true,b[0].contains(mx_,my_));button(b[1],"CONTINUE",false);button(b[2],"SETTINGS",true,b[2].contains(mx_,my_));button(b[3],"QUIT",true,b[3].contains(mx_,my_));
        if(!status_.empty())text(fb_,72,570,status_,{210,192,160},2);
    }

    void draw_settings() {
        if(menu_.valid())fb_.blit_cover(menu_);else fb_.clear({8,18,35});
        fb_.blend_rect({fb_.width()/2-390,74,780,fb_.height()-122},{6,5,4},220);fb_.rect({fb_.width()/2-390,74,780,fb_.height()-122},gold,2);
        text(fb_,fb_.width()/2-180,104,"DISPLAY OPTIONS",gold,4);text(fb_,fb_.width()/2-120,192,"WINDOW SIZE",pale,2);
        const auto rb=resolution_buttons();for(int i=0;i<3;++i)button(rb[i],std::to_string(kResolutions[i][0])+"X"+std::to_string(kResolutions[i][1]),true,i==resolution_index_);
        text(fb_,fb_.width()/2-120,306,"MAP SCROLLING",pale,2);button(edge_scroll_rect(),std::string("EDGE SCROLL  ")+(edge_scroll_?"ON":"OFF"),true,edge_scroll_);
        text(fb_,fb_.width()/2-114,408,"SCROLL SPEED",pale,2);const auto sb=speed_buttons();button(sb[0],"SLOW",true,scroll_speed_px_<500);button(sb[1],"NORMAL",true,scroll_speed_px_>=500&&scroll_speed_px_<850);button(sb[2],"FAST",true,scroll_speed_px_>=850);
        text(fb_,fb_.width()/2-300,520,"RIGHT OR MIDDLE DRAG PANS THE MAP",pale,2);text(fb_,fb_.width()/2-300,546,"MOUSE WHEEL ZOOMS   ARROWS OR WASD PAN",pale,2);button(settings_back(),"BACK",true,settings_back().contains(mx_,my_));
    }

    void handle_settings_click(int x,int y) {
        if(settings_back().contains(x,y)){screen_=Screen::Menu;dirty_=true;return;}
        const auto rb=resolution_buttons();for(int i=0;i<3;++i)if(rb[i].contains(x,y)){resolution_index_=i;requested_w_=kResolutions[i][0];requested_h_=kResolutions[i][1];resize_pending_=true;dirty_=true;return;}
        if(edge_scroll_rect().contains(x,y)){edge_scroll_=!edge_scroll_;dirty_=true;return;}
        const auto sb=speed_buttons();if(sb[0].contains(x,y))scroll_speed_px_=360.0;else if(sb[1].contains(x,y))scroll_speed_px_=620.0;else if(sb[2].contains(x,y))scroll_speed_px_=980.0;else return;dirty_=true;
    }

    void draw_game() {
        fb_.clear({47,34,28});
        fb_.fill_rect({0,0,fb_.width(),78},panel);
        text(fb_,20,11,"SETTLEMENT ON THE NILE",gold,3);
        const std::string stats="POP "+std::to_string(world_.population())+
            "  IMM "+std::to_string(world_.immigrants_in_transit())+
            "  EMP "+std::to_string(world_.employed_population())+
            "  FOOD "+std::to_string(world_.total_food())+
            "  TREASURY "+std::to_string(world_.treasury());
        text(fb_,20,47,stats,pale,2);button(main_menu_rect(),"MAIN MENU",true,main_menu_rect().contains(mx_,my_));

        const int tw=cam_.tile_w(),th=cam_.tile_h();
        for(int sum=0;sum<World::kWidth+World::kHeight-1;++sum){
            for(int y=0;y<World::kHeight;++y){
                const int x=sum-y;if(!world_.in_bounds(x,y))continue;
                const IsoPoint p=cam_.project(x,y);if(p.x<-tw||p.x>fb_.width()+tw||p.y<70-th||p.y>fb_.height()+th)continue;
                const Tile& tile=world_.tile(x,y);const Color ground=terrain_color(tile.terrain,x,y);fb_.diamond(p,tw,th,ground,ground);
                if(tile.terrain==Terrain::Water&&((x+y)&3)==0)fb_.line(p.x-tw/5,p.y-1,p.x+tw/5,p.y-1,{73,143,163});
                if(tile.terrain==Terrain::Reeds)for(int k=-2;k<=2;++k)fb_.line(p.x+k*3,p.y,p.x+k*3+1,p.y-10,{38,83,43});
                draw_structure_at(x,y,tile,p,tw,th);
            }
        }
        draw_agents();

        if(world_.in_bounds(hover_x_,hover_y_)){const IsoPoint p=cam_.project(hover_x_,hover_y_);fb_.diamond_outline(p,tw,th,{255,230,130});}

        const auto tools=tool_buttons();
        const char* labels[11]={"INSPECT","ROAD","HOUSE","FARM","GRANARY","MARKET","WELL","HUNT LODGE","CLAY PIT","POTTER","BULLDOZE"};
        for(int i=0;i<11;++i)button(tools[i],labels[i],true,static_cast<int>(tool_)==i);

        const Rect info{fb_.width()-360,88,342,142};fb_.blend_rect(info,{8,7,6},205);fb_.rect(info,gold,1);
        if(world_.in_bounds(selected_x_,selected_y_)){
            const Tile& tile=world_.tile(selected_x_,selected_y_);
            text(fb_,info.x+14,info.y+12,"TILE "+std::to_string(selected_x_)+","+std::to_string(selected_y_),pale,2);
            text(fb_,info.x+14,info.y+34,World::terrain_name(tile.terrain),gold,2);text(fb_,info.x+14,info.y+56,World::structure_name(tile.structure),gold,2);
            if(tile.structure==Structure::House){
                text(fb_,info.x+14,info.y+78,"POP "+std::to_string(tile.population)+"/"+std::to_string(world_.house_capacity(selected_x_,selected_y_))+"  JOB "+std::to_string(tile.employed),pale,2);
                text(fb_,info.x+14,info.y+100,"LEVEL "+std::to_string(tile.housing_level)+"  WATER "+(world_.has_well_service(selected_x_,selected_y_)?"YES":"NO"),pale,2);
                text(fb_,info.x+14,info.y+122,"FOOD "+std::to_string(tile.food_stock),pale,2);
            }else if(tile.structure==Structure::Farm||tile.structure==Structure::Granary||tile.structure==Structure::Market||tile.structure==Structure::HuntingLodge){
                text(fb_,info.x+14,info.y+82,"FOOD STOCK "+std::to_string(tile.food_stock),pale,2);
            }
        }else text(fb_,info.x+14,info.y+48,"CLICK A TILE TO INSPECT",pale,2);

        text(fb_,18,fb_.height()-116,"VISIBLE CITY: IMMIGRANTS > HOMES > JOBS > HUNTERS > GRANARY > MARKET",pale,2);
    }
};

class X11App {
public:
    X11App(int w,int h):fb_(w,h),game_(fb_){
        display_=XOpenDisplay(nullptr);if(!display_)throw std::runtime_error("Unable to open X11 display");
        screen_=DefaultScreen(display_);window_=XCreateSimpleWindow(display_,RootWindow(display_,screen_),100,100,w,h,0,BlackPixel(display_,screen_),BlackPixel(display_,screen_));
        XStoreName(display_,window_,"Egypt");XSelectInput(display_,window_,ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|StructureNotifyMask);
        delete_atom_=XInternAtom(display_,"WM_DELETE_WINDOW",False);XSetWMProtocols(display_,window_,&delete_atom_,1);gc_=XCreateGC(display_,window_,0,nullptr);XMapWindow(display_,window_);recreate(w,h);
        if(!infiltratr_fixed_step_configure(&scheduler_,1000000000ULL,4ULL,500000000ULL,8ULL))throw std::runtime_error("Common fixed-step scheduler configuration failed");
        infiltratr_fixed_step_reset(&scheduler_,now_ns());last_frame_ns_=now_ns();
    }

    ~X11App(){if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);}if(gc_)XFreeGC(display_,gc_);if(window_)XDestroyWindow(display_,window_);if(display_)XCloseDisplay(display_);}

    int run(){
        while(game_.running()){
            while(XPending(display_)>0){XEvent event;XNextEvent(display_,&event);switch(event.type){
                case Expose:game_.resize();break;
                case ConfigureNotify:if(event.xconfigure.width!=fb_.width()||event.xconfigure.height!=fb_.height()){fb_.resize(event.xconfigure.width,event.xconfigure.height);recreate(event.xconfigure.width,event.xconfigure.height);game_.resize();}break;
                case MotionNotify:game_.on_motion(event.xmotion.x,event.xmotion.y);break;
                case ButtonPress:game_.on_button_press(event.xbutton.button,event.xbutton.x,event.xbutton.y);break;
                case ButtonRelease:game_.on_button_release(event.xbutton.button,event.xbutton.x,event.xbutton.y);break;
                case KeyPress:game_.on_key(XLookupKeysym(&event.xkey,0));break;
                case ClientMessage:if(static_cast<Atom>(event.xclient.data.l[0])==delete_atom_)return 0;break;
                default:break;}}
            const std::uint64_t now=now_ns();const double dt=static_cast<double>(now-last_frame_ns_)/1000000000.0;last_frame_ns_=now;game_.frame(std::clamp(dt,0.0,0.05));
            int rw=0,rh=0;if(game_.take_resize_request(rw,rh))XResizeWindow(display_,window_,static_cast<unsigned>(rw),static_cast<unsigned>(rh));
            InfiltratrFixedStepResult result{};if(infiltratr_fixed_step_advance(&scheduler_,now,&result))for(std::uint64_t i=0;i<result.steps_to_run;++i)game_.tick();
            if(game_.dirty()){game_.draw();present();game_.rendered();}std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
        return 0;
    }

private:
    Display* display_=nullptr;int screen_=0;Window window_=0;GC gc_=0;Atom delete_atom_=0;XImage* image_=nullptr;Framebuffer fb_;Game game_;InfiltratrFixedStepScheduler scheduler_{};std::uint64_t last_frame_ns_=0;
    static std::uint64_t now_ns(){return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
    static unsigned long pack(std::uint8_t value,unsigned long mask){if(!mask)return 0;unsigned shift=0;while(((mask>>shift)&1UL)==0UL)++shift;const unsigned long max=mask>>shift;return((static_cast<unsigned long>(value)*max+127UL)/255UL<<shift)&mask;}
    void recreate(int w,int h){if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);image_=nullptr;}image_=XCreateImage(display_,DefaultVisual(display_,screen_),DefaultDepth(display_,screen_),ZPixmap,0,nullptr,w,h,32,0);if(!image_)throw std::runtime_error("Unable to create XImage");image_->data=static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line)*h,1));if(!image_->data)throw std::bad_alloc();}
    void present(){const auto& pixels=fb_.pixels();const int w=fb_.width(),h=fb_.height();for(int y=0;y<h;++y)for(int x=0;x<w;++x){const Color c=pixels[static_cast<std::size_t>(y*w+x)];XPutPixel(image_,x,y,pack(c.r,image_->red_mask)|pack(c.g,image_->green_mask)|pack(c.b,image_->blue_mask));}XPutImage(display_,window_,gc_,image_,0,0,0,0,w,h);XFlush(display_);}
};

} // namespace egypt

int main(){try{egypt::X11App app(1280,720);return app.run();}catch(const std::exception& error){std::cerr<<"Egypt failed to start: "<<error.what()<<'\n';return 1;}}
