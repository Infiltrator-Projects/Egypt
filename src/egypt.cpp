#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <array>
#include <cmath>
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
    bool contains(int px, int py) const { return px >= x && py >= y && px < x + w && py < y + h; }
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

    Color get(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return {0, 0, 0};
        return pixels_[static_cast<std::size_t>(y * width_ + x)];
    }

    void fill_rect(Rect r, Color c) {
        const int x0 = std::max(0, r.x), y0 = std::max(0, r.y);
        const int x1 = std::min(width_, r.x + r.w), y1 = std::min(height_, r.y + r.h);
        for (int y = y0; y < y1; ++y)
            for (int x = x0; x < x1; ++x)
                pixel(x, y, c);
    }

    void blend_rect(Rect r, Color c, std::uint8_t alpha) {
        const int x0 = std::max(0, r.x), y0 = std::max(0, r.y);
        const int x1 = std::min(width_, r.x + r.w), y1 = std::min(height_, r.y + r.h);
        const unsigned a = alpha, ia = 255U - a;
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                Color& d = pixels_[static_cast<std::size_t>(y * width_ + x)];
                d.r = static_cast<std::uint8_t>((d.r * ia + c.r * a) / 255U);
                d.g = static_cast<std::uint8_t>((d.g * ia + c.g * a) / 255U);
                d.b = static_cast<std::uint8_t>((d.b * ia + c.b * a) / 255U);
            }
        }
    }

    void rect(Rect r, Color c, int t = 1) {
        fill_rect({r.x, r.y, r.w, t}, c);
        fill_rect({r.x, r.y + r.h - t, r.w, t}, c);
        fill_rect({r.x, r.y, t, r.h}, c);
        fill_rect({r.x + r.w - t, r.y, t, r.h}, c);
    }

    void line(int x0, int y0, int x1, int y1, Color c) {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            pixel(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            const int e2 = err * 2;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void triangle(int x0, int y0, int x1, int y1, int x2, int y2, Color c) {
        const int minx = std::max(0, std::min({x0, x1, x2}));
        const int maxx = std::min(width_ - 1, std::max({x0, x1, x2}));
        const int miny = std::max(0, std::min({y0, y1, y2}));
        const int maxy = std::min(height_ - 1, std::max({y0, y1, y2}));
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

    void circle(int cx, int cy, int r, Color c) {
        const int rr = r * r;
        for (int y = -r; y <= r; ++y)
            for (int x = -r; x <= r; ++x)
                if (x * x + y * y <= rr) pixel(cx + x, cy + y, c);
    }

    void glow(int cx, int cy, int r, Color c, std::uint8_t alpha) {
        const float rf = static_cast<float>(r);
        for (int y = -r; y <= r; ++y) {
            for (int x = -r; x <= r; ++x) {
                const float d = std::sqrt(static_cast<float>(x * x + y * y));
                if (d > rf) continue;
                const float f = 1.0f - d / rf;
                const unsigned a = static_cast<unsigned>(alpha * f * f), ia = 255U - a;
                Color old = get(cx + x, cy + y);
                pixel(cx + x, cy + y, {
                    static_cast<std::uint8_t>((old.r * ia + c.r * a) / 255U),
                    static_cast<std::uint8_t>((old.g * ia + c.g * a) / 255U),
                    static_cast<std::uint8_t>((old.b * ia + c.b * a) / 255U)});
            }
        }
    }

private:
    int width_ = 1, height_ = 1;
    std::vector<Color> pixels_;
};

static Color mix(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return {
        static_cast<std::uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<std::uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<std::uint8_t>(a.b + (b.b - a.b) * t)};
}

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

static void text(Framebuffer& fb, int x, int y, const std::string& s, Color c, int scale = 2) {
    int cursor = x;
    for (char raw : s) {
        const char ch = raw >= 'a' && raw <= 'z' ? static_cast<char>(raw - 'a' + 'A') : raw;
        auto it = FONT.find(ch);
        const Glyph& g = it == FONT.end() ? FONT.at(' ') : it->second;
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col)
                if ((g[row] >> (4 - col)) & 1U)
                    fb.fill_rect({cursor + col * scale, y + row * scale, scale, scale}, c);
        cursor += 6 * scale;
    }
}

static void palm(Framebuffer& fb, int x, int y, int h, float sway = 0.0f) {
    const Color trunk{79,48,28}, frond{24,58,31};
    int topx = x;
    for (int i = 0; i < h; ++i) {
        const int xx = x + static_cast<int>(std::sin(i * 0.035f + sway) * i * 0.05f);
        fb.fill_rect({xx, y - i, std::max(2, h / 35), 1}, trunk);
        topx = xx;
    }
    const int topy = y - h;
    for (int a = -6; a <= 6; ++a) {
        fb.line(topx, topy, topx + a * h / 12, topy - h / 8 - std::abs(a) * h / 60, frond);
        fb.line(topx, topy + 1, topx + a * h / 11, topy + std::abs(a) * h / 90, frond);
    }
}

static void pyramid(Framebuffer& fb, int cx, int base, int half, int height) {
    fb.triangle(cx - half, base, cx, base - height, cx, base, {92,61,39});
    fb.triangle(cx, base - height, cx + half, base, cx, base, {186,125,65});
    for (int y = base - height + 12; y < base; y += 13) {
        float t = static_cast<float>(y - (base - height)) / height;
        int hw = static_cast<int>(half * t);
        fb.line(cx - hw, y, cx + hw, y, {122,82,49});
    }
}

static void menu_scene(Framebuffer& fb) {
    const int w = fb.width(), h = fb.height();
    const int horizon = h * 58 / 100;

    for (int y = 0; y < horizon; ++y) {
        float t = static_cast<float>(y) / std::max(1, horizon - 1);
        Color c = t < 0.72f ? mix({4,15,39},{15,42,82},t / 0.72f)
                            : mix({15,42,82},{118,69,54},(t - 0.72f) / 0.28f);
        fb.fill_rect({0,y,w,1},c);
    }
    for (int y = horizon; y < h; ++y) {
        float t = static_cast<float>(y - horizon) / std::max(1, h - horizon - 1);
        fb.fill_rect({0,y,w,1},mix({137,91,52},{64,48,39},t));
    }

    for (int i = 0; i < 300; ++i) {
        int x = (i * 811 + 29) % w;
        int y = (i * 353 + 7) % std::max(1,horizon * 72 / 100);
        if ((i % 7) == 0) fb.fill_rect({x,y,2,2},{210,220,224});
        else fb.pixel(x,y,{168,183,203});
    }

    const int mx = w * 84 / 100, my = h * 17 / 100, mr = std::max(17,h / 36);
    fb.glow(mx,my,mr*3,{255,226,177},115);
    fb.circle(mx,my,mr,{242,219,172});
    fb.circle(mx-mr/4,my-mr/4,std::max(2,mr/8),{213,191,151});
    fb.circle(mx+mr/3,my+mr/5,std::max(2,mr/10),{220,198,157});

    for (int x = 0; x < w; ++x) {
        float xf = static_cast<float>(x) / w;
        int top = horizon - 15 + static_cast<int>(16*std::sin(xf*13.0f)+7*std::sin(xf*31.0f));
        fb.fill_rect({x,top,1,horizon-top+8},{67,59,66});
    }

    const int base = h * 69 / 100;
    pyramid(fb,w*13/100,base,w*5/100,h*14/100);
    pyramid(fb,w*25/100,base,w*8/100,h*24/100);
    pyramid(fb,w*41/100,base,w*5/100,h*15/100);

    for (int y = horizon; y < h; ++y) {
        float t = static_cast<float>(y - horizon) / std::max(1,h-horizon);
        const int centre = static_cast<int>(w*(0.70f - 0.05f*t + 0.025f*std::sin(t*5.4f)));
        const int half = static_cast<int>(w*(0.045f + 0.105f*t));
        fb.fill_rect({centre-half-8,y,half*2+16,1},{34,67,39});
        fb.fill_rect({centre-half,y,half*2,1},mix({25,74,102},{28,88,113},t));
        if ((y%9)==0) fb.line(centre-half/2,y,centre+half/2,y,{104,153,162});
    }

    for (int i=0;i<80;++i) {
        int x=w*53/100+(i*47)%(w*43/100);
        int y=h*65/100+(i*19)%(h*7/100);
        fb.glow(x,y,3,{255,166,73},210);
    }

    fb.fill_rect({w*91/100,h*61/100,w*4/100,h*13/100},{76,51,34});
    fb.fill_rect({w*90/100,h*58/100,w/90,h*16/100},{91,61,39});
    fb.fill_rect({w*95/100,h*56/100,w/110,h*18/100},{97,67,42});

    palm(fb,w*8/100,h*84/100,h/9,0.3f);
    palm(fb,w*47/100,h*84/100,h/11,0.8f);
    palm(fb,w*57/100,h*85/100,h/10,1.2f);
    palm(fb,w*79/100,h*84/100,h/11,0.5f);
    palm(fb,w*88/100,h*85/100,h/9,1.4f);

    auto boat=[&](int x,int y,int s){
        fb.line(x-s,y,x+s,y,{64,44,28});
        fb.line(x,y,x,y-s*3,{63,55,45});
        fb.triangle(x,y-s*3,x,y-1,x+s*2,y-s,{217,210,181});
    };
    boat(w*72/100,h*83/100,std::max(7,w/125));
    boat(w*75/100,h*69/100,std::max(4,w/210));

    fb.blend_rect({0,0,w*45/100,h},{5,7,12},80);
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
        if (screen_ == Screen::Game) screen_ = Screen::Menu; else running_ = false;
        dirty_ = true;
    }

    void on_motion(int x,int y) {
        mx_=x; my_=y;
        int old=hover_;
        hover_=screen_==Screen::Menu?button_at(x,y):-1;
        if(old!=hover_) dirty_=true;
    }

    void on_click(int x,int y) {
        if(screen_==Screen::Game) {
            if(back_rect().contains(x,y)) { screen_=Screen::Menu; dirty_=true; }
            return;
        }
        const int b=button_at(x,y);
        if(b==0){screen_=Screen::Game;status_.clear();}
        else if(b==2)status_="SETTINGS WILL COME AFTER FIRST CITY INTERACTION";
        else if(b==3)running_=false;
        dirty_=true;
    }

    void draw(){ if(screen_==Screen::Menu)draw_menu();else draw_game(); }

private:
    Framebuffer& fb_;
    Screen screen_=Screen::Menu;
    bool running_=true,dirty_=true;
    int mx_=0,my_=0,hover_=-1;
    std::string status_="PRE-ALPHA - FIRST NATIVE SHELL";
    const Color gold{224,184,95}, pale{247,224,174}, panel{22,18,18}, panel_hi{58,42,31};

    std::array<Rect,4> menu_buttons() const {
        const int x=72,y=300,w=340,h=50,g=12;
        return {Rect{x,y,w,h},Rect{x,y+h+g,w,h},Rect{x,y+2*(h+g),w,h},Rect{x,y+3*(h+g),w,h}};
    }
    Rect back_rect() const { return {fb_.width()-250,24,220,42}; }
    int button_at(int x,int y) const {
        auto b=menu_buttons(); for(int i=0;i<4;++i)if(b[i].contains(x,y))return i; return -1;
    }
    void button(Rect r,const std::string& label,bool enabled,bool hovered=false) {
        fb_.blend_rect(r,hovered&&enabled?panel_hi:panel,hovered&&enabled?225:195);
        fb_.rect(r,enabled?gold:Color{95,87,79},2);
        text(fb_,r.x+18,r.y+16,label,enabled?pale:Color{120,113,104},2);
    }

    void draw_menu() {
        menu_scene(fb_);
        fb_.blend_rect({46,48,490,545},{8,8,10},60);
        text(fb_,72,78,"EGYPT",gold,8);
        text(fb_,74,155,"A LIVING CITY ON THE NILE",pale,3);
        fb_.fill_rect({72,206,420,2},gold);
        auto b=menu_buttons();
        button(b[0],"NEW GAME",true,hover_==0);
        button(b[1],"CONTINUE",false);
        button(b[2],"SETTINGS",true,hover_==2);
        button(b[3],"QUIT",true,hover_==3);
        if(!status_.empty())text(fb_,72,570,status_,{215,200,174},2);
    }

    void draw_game() {
        const int w=fb_.width(),h=fb_.height(),mapy=84;
        fb_.clear({190,139,79});
        fb_.fill_rect({0,0,w,mapy},panel);
        text(fb_,24,20,"SETTLEMENT ON THE NILE",gold,3);
        text(fb_,24,58,"POPULATION 0   TREASURY 5000   YEAR 1   FLOOD FORECAST -",pale,2);
        Rect back=back_rect(); button(back,"MAIN MENU",true,back.contains(mx_,my_));
        const int rx=w*58/100,rh=std::max(70,w*8/100);
        fb_.fill_rect({rx-rh-28,mapy,rh*2+56,h-mapy},{102,119,78});
        fb_.fill_rect({rx-rh,mapy,rh*2,h-mapy},{45,113,135});
        for(int x=0;x<w;x+=32)fb_.line(x,mapy,x,h-1,{137,104,70});
        for(int y=mapy;y<h;y+=32)fb_.line(0,y,w-1,y,{137,104,70});
        text(fb_,22,h-34,"FIRST MILESTONE: NATIVE ENGINE SHELL ACTIVE",{70,52,38},2);
    }
};

class X11App {
public:
    X11App(int w,int h):fb_(w,h),game_(fb_) {
        display_=XOpenDisplay(nullptr); if(!display_)throw std::runtime_error("Unable to open X11 display");
        screen_=DefaultScreen(display_);
        window_=XCreateSimpleWindow(display_,RootWindow(display_,screen_),100,100,w,h,0,BlackPixel(display_,screen_),BlackPixel(display_,screen_));
        XStoreName(display_,window_,"Egypt");
        XSelectInput(display_,window_,ExposureMask|KeyPressMask|ButtonPressMask|PointerMotionMask|StructureNotifyMask);
        wm_delete_=XInternAtom(display_,"WM_DELETE_WINDOW",False); XSetWMProtocols(display_,window_,&wm_delete_,1);
        gc_=XCreateGC(display_,window_,0,nullptr); XMapWindow(display_,window_); recreate_image(w,h);
    }
    ~X11App(){
        if(image_){image_->data=nullptr;XDestroyImage(image_);} if(gc_)XFreeGC(display_,gc_);
        if(window_)XDestroyWindow(display_,window_); if(display_)XCloseDisplay(display_);
    }
    int run(){
        while(game_.running()){
            XEvent e; XNextEvent(display_,&e);
            switch(e.type){
                case Expose: game_.resize(); break;
                case ConfigureNotify:
                    if(e.xconfigure.width!=fb_.width()||e.xconfigure.height!=fb_.height()){
                        fb_.resize(e.xconfigure.width,e.xconfigure.height);recreate_image(e.xconfigure.width,e.xconfigure.height);game_.resize();
                    } break;
                case MotionNotify: game_.on_motion(e.xmotion.x,e.xmotion.y); break;
                case ButtonPress: if(e.xbutton.button==Button1)game_.on_click(e.xbutton.x,e.xbutton.y); break;
                case KeyPress: game_.on_key(XLookupKeysym(&e.xkey,0)); break;
                case ClientMessage: if(static_cast<Atom>(e.xclient.data.l[0])==wm_delete_)return 0; break;
                default: break;
            }
            if(game_.dirty()){game_.draw();present();game_.rendered();}
        }
        return 0;
    }
private:
    Display* display_=nullptr; int screen_=0; Window window_=0; GC gc_=0; Atom wm_delete_=0; XImage* image_=nullptr;
    Framebuffer fb_; Game game_;
    static unsigned long pack(std::uint8_t v,unsigned long mask){
        if(!mask)return 0; unsigned shift=0;while(((mask>>shift)&1UL)==0UL)++shift;unsigned long max=mask>>shift;
        return ((static_cast<unsigned long>(v)*max+127UL)/255UL<<shift)&mask;
    }
    void recreate_image(int w,int h){
        if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);image_=nullptr;}
        image_=XCreateImage(display_,DefaultVisual(display_,screen_),DefaultDepth(display_,screen_),ZPixmap,0,nullptr,w,h,32,0);
        if(!image_)throw std::runtime_error("Unable to create XImage");
        image_->data=static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line)*h,1));
        if(!image_->data)throw std::bad_alloc();
    }
    void present(){
        const auto& p=fb_.pixels(); const int w=fb_.width(),h=fb_.height();
        for(int y=0;y<h;++y)for(int x=0;x<w;++x){
            Color c=p[static_cast<std::size_t>(y*w+x)];
            XPutPixel(image_,x,y,pack(c.r,image_->red_mask)|pack(c.g,image_->green_mask)|pack(c.b,image_->blue_mask));
        }
        XPutImage(display_,window_,gc_,image_,0,0,0,0,w,h);XFlush(display_);
    }
};

} // namespace egypt

int main(){
    try{egypt::X11App app(1280,720);return app.run();}
    catch(const std::exception& e){std::cerr<<"Egypt failed to start: "<<e.what()<<'\n';return 1;}
}
