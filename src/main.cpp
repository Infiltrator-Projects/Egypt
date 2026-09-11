#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace egypt {

struct Color { std::uint8_t r, g, b; };
struct Rect {
    int x, y, w, h;
    bool contains(int px, int py) const {
        return px >= x && py >= y && px < x + w && py < y + h;
    }
};

enum class Screen { Menu, Game };

class Framebuffer {
public:
    Framebuffer(int w, int h) { resize(w, h); }

    void resize(int w, int h) {
        width_ = std::max(1, w);
        height_ = std::max(1, h);
        pixels_.assign(static_cast<std::size_t>(width_ * height_), Color{0, 0, 0});
    }

    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<Color>& pixels() const { return pixels_; }

    void clear(Color c) { std::fill(pixels_.begin(), pixels_.end(), c); }

    void pixel(int x, int y, Color c) {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        pixels_[static_cast<std::size_t>(y * width_ + x)] = c;
    }

    void fill_rect(Rect r, Color c) {
        const int x0 = std::max(0, r.x);
        const int y0 = std::max(0, r.y);
        const int x1 = std::min(width_, r.x + r.w);
        const int y1 = std::min(height_, r.y + r.h);
        for (int y = y0; y < y1; ++y)
            for (int x = x0; x < x1; ++x)
                pixel(x, y, c);
    }

    void rect(Rect r, Color c, int thickness = 1) {
        fill_rect({r.x, r.y, r.w, thickness}, c);
        fill_rect({r.x, r.y + r.h - thickness, r.w, thickness}, c);
        fill_rect({r.x, r.y, thickness, r.h}, c);
        fill_rect({r.x + r.w - thickness, r.y, thickness, r.h}, c);
    }

    void line(int x0, int y0, int x1, int y1, Color c) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
            pixel(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            const int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void triangle(int x0, int y0, int x1, int y1, int x2, int y2, Color c) {
        const int min_y = std::max(0, std::min({y0, y1, y2}));
        const int max_y = std::min(height_ - 1, std::max({y0, y1, y2}));
        const int min_x = std::max(0, std::min({x0, x1, x2}));
        const int max_x = std::min(width_ - 1, std::max({x0, x1, x2}));
        auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
            return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
        };
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                const int e0 = edge(x0, y0, x1, y1, x, y);
                const int e1 = edge(x1, y1, x2, y2, x, y);
                const int e2 = edge(x2, y2, x0, y0, x, y);
                if ((e0 >= 0 && e1 >= 0 && e2 >= 0) || (e0 <= 0 && e1 <= 0 && e2 <= 0))
                    pixel(x, y, c);
            }
        }
    }

private:
    int width_ = 1;
    int height_ = 1;
    std::vector<Color> pixels_;
};

using Glyph = std::array<std::uint8_t, 7>;

const std::unordered_map<char, Glyph> FONT = {
    {' ',{0,0,0,0,0,0,0}}, {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}},
    {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}}, {'E',{31,16,16,30,16,16,31}},
    {'F',{31,16,16,30,16,16,16}}, {'G',{14,17,16,23,17,17,14}}, {'H',{17,17,17,31,17,17,17}},
    {'I',{14,4,4,4,4,4,14}}, {'J',{1,1,1,1,17,17,14}}, {'K',{17,18,20,24,20,18,17}},
    {'L',{16,16,16,16,16,16,31}}, {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}},
    {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}}, {'Q',{14,17,17,17,21,18,13}},
    {'R',{30,17,17,30,20,18,17}}, {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}},
    {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}}, {'W',{17,17,17,21,21,21,10}},
    {'X',{17,17,10,4,10,17,17}}, {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}},
    {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}}, {'2',{14,17,1,2,4,8,31}},
    {'3',{30,1,1,14,1,1,30}}, {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}},
    {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}}, {'8',{14,17,17,14,17,17,14}},
    {'9',{14,17,17,15,1,1,14}}, {'-',{0,0,0,31,0,0,0}}, {'.',{0,0,0,0,0,12,12}},
    {':',{0,12,12,0,12,12,0}}, {'/',{1,2,2,4,8,8,16}}
};

void draw_text(Framebuffer& fb, int x, int y, const std::string& text, Color c, int scale = 2) {
    int cursor = x;
    for (char raw : text) {
        const char ch = raw >= 'a' && raw <= 'z' ? static_cast<char>(raw - 'a' + 'A') : raw;
        const auto it = FONT.find(ch);
        const Glyph& glyph = it != FONT.end() ? it->second : FONT.at(' ');
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col)
                if ((glyph[row] >> (4 - col)) & 1U)
                    fb.fill_rect({cursor + col * scale, y + row * scale, scale, scale}, c);
        cursor += 6 * scale;
    }
}

class Game {
public:
    explicit Game(Framebuffer& fb) : fb_(fb) {}

    bool running() const { return running_; }
    bool dirty() const { return dirty_; }
    void rendered() { dirty_ = false; }
    void resize() { dirty_ = true; }

    void on_key(KeySym key) {
        if (key != XK_Escape) return;
        if (screen_ == Screen::Game) screen_ = Screen::Menu;
        else running_ = false;
        dirty_ = true;
    }

    void on_motion(int x, int y) {
        mouse_x_ = x;
        mouse_y_ = y;
        const int old = hover_;
        hover_ = screen_ == Screen::Menu ? button_at(x, y) : -1;
        if (old != hover_) dirty_ = true;
    }

    void on_click(int x, int y) {
        if (screen_ == Screen::Game) {
            if (back_rect().contains(x, y)) {
                screen_ = Screen::Menu;
                dirty_ = true;
            }
            return;
        }
        const int button = button_at(x, y);
        if (button == 0) {
            screen_ = Screen::Game;
            status_.clear();
        } else if (button == 2) {
            status_ = "SETTINGS WILL COME AFTER FIRST CITY INTERACTION";
        } else if (button == 3) {
            running_ = false;
        }
        dirty_ = true;
    }

    void draw() {
        if (screen_ == Screen::Menu) draw_menu();
        else draw_game();
    }

private:
    Framebuffer& fb_;
    Screen screen_ = Screen::Menu;
    bool running_ = true;
    bool dirty_ = true;
    int mouse_x_ = 0;
    int mouse_y_ = 0;
    int hover_ = -1;
    std::string status_ = "PRE-ALPHA - FIRST NATIVE SHELL";

    const Color night{23,18,15}, sand{185,131,71}, sand_dark{112,71,44};
    const Color gold{217,177,95}, pale{242,215,154}, nile{45,113,135};
    const Color fertile{102,119,78}, panel{42,31,24}, panel_hi{70,50,35};

    std::array<Rect, 4> menu_buttons() const {
        const int x = 72, y = 300, w = 340, h = 50, gap = 12;
        return {Rect{x,y,w,h}, Rect{x,y+h+gap,w,h}, Rect{x,y+2*(h+gap),w,h}, Rect{x,y+3*(h+gap),w,h}};
    }

    Rect back_rect() const { return {fb_.width() - 250, 24, 220, 42}; }

    int button_at(int x, int y) const {
        const auto buttons = menu_buttons();
        for (int i = 0; i < 4; ++i)
            if (buttons[i].contains(x, y)) return i;
        return -1;
    }

    void button(Rect r, const std::string& label, bool enabled, bool hovered = false) {
        fb_.fill_rect(r, hovered && enabled ? panel_hi : panel);
        fb_.rect(r, enabled ? gold : Color{90,78,65}, 2);
        draw_text(fb_, r.x + 18, r.y + 16, label, enabled ? pale : Color{115,105,92}, 2);
    }

    void draw_menu() {
        const int w = fb_.width(), h = fb_.height();
        fb_.clear(night);
        fb_.fill_rect({0, h * 54 / 100, w, h * 46 / 100}, sand_dark);
        fb_.fill_rect({0, h * 68 / 100, w, h * 32 / 100}, sand);

        const int rx = w * 72 / 100;
        const int rw = std::max(90, w * 10 / 100);
        fb_.triangle(rx - rw / 4, h * 44 / 100, rx + rw / 2, h * 44 / 100, rx - rw / 2, h, nile);
        fb_.triangle(w * 5 / 100, h * 68 / 100, w * 12 / 100, h * 43 / 100, w * 20 / 100, h * 68 / 100, {59,43,34});
        fb_.triangle(w * 19 / 100, h * 68 / 100, w * 27 / 100, h * 50 / 100, w * 34 / 100, h * 68 / 100, {62,45,35});
        fb_.triangle(w * 31 / 100, h * 68 / 100, w * 38 / 100, h * 55 / 100, w * 45 / 100, h * 68 / 100, {66,47,36});

        draw_text(fb_, 72, 78, "EGYPT", gold, 8);
        draw_text(fb_, 74, 155, "A LIVING CITY ON THE NILE", pale, 3);
        fb_.fill_rect({72, 206, 420, 2}, gold);

        const auto buttons = menu_buttons();
        button(buttons[0], "NEW GAME", true, hover_ == 0);
        button(buttons[1], "CONTINUE", false);
        button(buttons[2], "SETTINGS", true, hover_ == 2);
        button(buttons[3], "QUIT", true, hover_ == 3);

        if (!status_.empty()) draw_text(fb_, 72, 570, status_, {210,192,160}, 2);
    }

    void draw_game() {
        const int w = fb_.width(), h = fb_.height();
        fb_.clear(sand);
        fb_.fill_rect({0, 0, w, 84}, panel);
        draw_text(fb_, 24, 20, "SETTLEMENT ON THE NILE", gold, 3);
        draw_text(fb_, 24, 58, "POPULATION 0   TREASURY 5000   YEAR 1   FLOOD FORECAST -", pale, 2);

        const Rect back = back_rect();
        button(back, "MAIN MENU", true, back.contains(mouse_x_, mouse_y_));

        const int map_y = 84;
        const int river_x = w * 58 / 100;
        const int river_half = std::max(70, w * 8 / 100);
        fb_.fill_rect({river_x - river_half - 28, map_y, river_half * 2 + 56, h - map_y}, fertile);
        fb_.fill_rect({river_x - river_half, map_y, river_half * 2, h - map_y}, nile);

        for (int x = 0; x < w; x += 32) fb_.line(x, map_y, x, h - 1, {137,104,70});
        for (int y = map_y; y < h; y += 32) fb_.line(0, y, w - 1, y, {137,104,70});

        const int hx = w * 36 / 100;
        const int hy = map_y + (h - map_y) * 52 / 100;
        fb_.fill_rect({hx - 18, hy - 14, 36, 28}, {109,67,41});
        fb_.triangle(hx - 22, hy - 14, hx, hy - 34, hx + 22, hy - 14, {140,90,50});

        draw_text(fb_, 22, h - 34, "FIRST MILESTONE: NATIVE ENGINE SHELL ACTIVE", {70,52,38}, 2);
    }
};

class X11App {
public:
    X11App(int w, int h) : fb_(w, h), game_(fb_) {
        display_ = XOpenDisplay(nullptr);
        if (!display_) throw std::runtime_error("Unable to open X11 display");

        screen_ = DefaultScreen(display_);
        window_ = XCreateSimpleWindow(display_, RootWindow(display_, screen_), 100, 100, w, h, 0,
                                      BlackPixel(display_, screen_), BlackPixel(display_, screen_));
        XStoreName(display_, window_, "Egypt");
        XSelectInput(display_, window_, ExposureMask | KeyPressMask | ButtonPressMask |
                                       PointerMotionMask | StructureNotifyMask);
        wm_delete_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display_, window_, &wm_delete_, 1);
        gc_ = XCreateGC(display_, window_, 0, nullptr);
        XMapWindow(display_, window_);
        recreate_image(w, h);
    }

    ~X11App() {
        if (image_) {
            image_->data = nullptr;
            XDestroyImage(image_);
        }
        if (gc_) XFreeGC(display_, gc_);
        if (window_) XDestroyWindow(display_, window_);
        if (display_) XCloseDisplay(display_);
    }

    int run() {
        while (game_.running()) {
            XEvent event;
            XNextEvent(display_, &event);
            switch (event.type) {
                case Expose:
                    game_.resize();
                    break;
                case ConfigureNotify:
                    if (event.xconfigure.width != fb_.width() || event.xconfigure.height != fb_.height()) {
                        fb_.resize(event.xconfigure.width, event.xconfigure.height);
                        recreate_image(event.xconfigure.width, event.xconfigure.height);
                        game_.resize();
                    }
                    break;
                case MotionNotify:
                    game_.on_motion(event.xmotion.x, event.xmotion.y);
                    break;
                case ButtonPress:
                    if (event.xbutton.button == Button1)
                        game_.on_click(event.xbutton.x, event.xbutton.y);
                    break;
                case KeyPress:
                    game_.on_key(XLookupKeysym(&event.xkey, 0));
                    break;
                case ClientMessage:
                    if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_) return 0;
                    break;
                default:
                    break;
            }

            if (game_.dirty()) {
                game_.draw();
                present();
                game_.rendered();
            }
        }
        return 0;
    }

private:
    Display* display_ = nullptr;
    int screen_ = 0;
    Window window_ = 0;
    GC gc_ = 0;
    Atom wm_delete_ = 0;
    XImage* image_ = nullptr;
    Framebuffer fb_;
    Game game_;

    static unsigned long pack_component(std::uint8_t value, unsigned long mask) {
        if (!mask) return 0;
        unsigned shift = 0;
        while (((mask >> shift) & 1UL) == 0UL) ++shift;
        const unsigned long max = mask >> shift;
        return ((static_cast<unsigned long>(value) * max + 127UL) / 255UL << shift) & mask;
    }

    void recreate_image(int w, int h) {
        if (image_) {
            std::free(image_->data);
            image_->data = nullptr;
            XDestroyImage(image_);
            image_ = nullptr;
        }

        const int depth = DefaultDepth(display_, screen_);
        image_ = XCreateImage(display_, DefaultVisual(display_, screen_), depth, ZPixmap,
                              0, nullptr, w, h, 32, 0);
        if (!image_) throw std::runtime_error("Unable to create XImage");

        image_->data = static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line) * h, 1));
        if (!image_->data) throw std::bad_alloc();
    }

    void present() {
        const auto& pixels = fb_.pixels();
        const int w = fb_.width(), h = fb_.height();
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const Color c = pixels[static_cast<std::size_t>(y * w + x)];
                const unsigned long packed = pack_component(c.r, image_->red_mask) |
                                             pack_component(c.g, image_->green_mask) |
                                             pack_component(c.b, image_->blue_mask);
                XPutPixel(image_, x, y, packed);
            }
        }
        XPutImage(display_, window_, gc_, image_, 0, 0, 0, 0, w, h);
        XFlush(display_);
    }
};

} // namespace egypt

int main() {
    try {
        egypt::X11App app(1280, 720);
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Egypt failed to start: " << e.what() << '\n';
        return 1;
    }
}
