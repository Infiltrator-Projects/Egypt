#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "common/image.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace egypt {

using Color = common::Color;
using ImageAsset = common::Image;

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

    Color sample(const ImageAsset& img, float x, float y) const {
        x = std::clamp(x, 0.0f, static_cast<float>(img.width - 1));
        y = std::clamp(y, 0.0f, static_cast<float>(img.height - 1));
        const int x0 = static_cast<int>(x);
        const int y0 = static_cast<int>(y);
        const int x1 = std::min(x0 + 1, img.width - 1);
        const int y1 = std::min(y0 + 1, img.height - 1);
        const float fx = x - static_cast<float>(x0);
        const float fy = y - static_cast<float>(y0);
        auto at = [&](int sx, int sy) -> Color {
            return img.pixels[static_cast<std::size_t>(sy * img.width + sx)];
        };
        const Color a = at(x0, y0), b = at(x1, y0), c = at(x0, y1), d = at(x1, y1);
        auto mix = [](std::uint8_t p, std::uint8_t q, float t) {
            return static_cast<float>(p) + (static_cast<float>(q) - static_cast<float>(p)) * t;
        };
        return {
            static_cast<std::uint8_t>(std::clamp(mix(static_cast<std::uint8_t>(mix(a.r,b.r,fx)), static_cast<std::uint8_t>(mix(c.r,d.r,fx)), fy), 0.0f, 255.0f)),
            static_cast<std::uint8_t>(std::clamp(mix(static_cast<std::uint8_t>(mix(a.g,b.g,fx)), static_cast<std::uint8_t>(mix(c.g,d.g,fx)), fy), 0.0f, 255.0f)),
            static_cast<std::uint8_t>(std::clamp(mix(static_cast<std::uint8_t>(mix(a.b,b.b,fx)), static_cast<std::uint8_t>(mix(c.b,d.b,fx)), fy), 0.0f, 255.0f))
        };
    }

    void blit_cover(const ImageAsset& img) {
        if (!img.valid()) return;
        const float sx = static_cast<float>(width_) / static_cast<float>(img.width);
        const float sy = static_cast<float>(height_) / static_cast<float>(img.height);
        const float scale = std::max(sx, sy);
        const float draw_w = img.width * scale;
        const float draw_h = img.height * scale;
        const float ox = (width_ - draw_w) * 0.5f;
        const float oy = (height_ - draw_h) * 0.5f;
        for (int y = 0; y < height_; ++y) {
            const float src_y = (static_cast<float>(y) - oy) / scale;
            for (int x = 0; x < width_; ++x) {
                const float src_x = (static_cast<float>(x) - ox) / scale;
                pixels_[static_cast<std::size_t>(y * width_ + x)] = sample(img, src_x, src_y);
            }
        }
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
            const int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void triangle(int x0,int y0,int x1,int y1,int x2,int y2,Color c) {
        const int minx=std::max(0,std::min({x0,x1,x2})), maxx=std::min(width_-1,std::max({x0,x1,x2}));
        const int miny=std::max(0,std::min({y0,y1,y2})), maxy=std::min(height_-1,std::max({y0,y1,y2}));
        auto edge=[](int ax,int ay,int bx,int by,int px,int py){return(px-ax)*(by-ay)-(py-ay)*(bx-ax);};
        for(int y=miny;y<=maxy;++y) for(int x=minx;x<=maxx;++x){
            const int a=edge(x0,y0,x1,y1,x,y), b=edge(x1,y1,x2,y2,x,y), d=edge(x2,y2,x0,y0,x,y);
            if((a>=0&&b>=0&&d>=0)||(a<=0&&b<=0&&d<=0)) pixel(x,y,c);
        }
    }

private:
    int width_ = 1, height_ = 1;
    std::vector<Color> pixels_;
};

using Glyph = std::array<std::uint8_t, 7>;
const std::unordered_map<char,Glyph> FONT = {
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

static void text(Framebuffer& fb, int x, int y, const std::string& s, Color c, int scale=2) {
    int cursor=x;
    for(char raw:s){
        const char ch=raw>='a'&&raw<='z'?static_cast<char>(raw-'a'+'A'):raw;
        auto it=FONT.find(ch); const Glyph& g=it==FONT.end()?FONT.at(' '):it->second;
        for(int r=0;r<7;++r) for(int col=0;col<5;++col)
            if((g[r]>>(4-col))&1U) fb.fill_rect({cursor+col*scale,y+r*scale,scale,scale},c);
        cursor += 6*scale;
    }
}

class Game {
public:
    explicit Game(Framebuffer& fb) : fb_(fb), menu_(common::load_e16("build/assets/menu.e16")) {
        if (!menu_.valid()) std::cerr << "Egypt: could not load build/assets/menu.e16\n";
    }

    bool running() const { return running_; }
    bool dirty() const { return dirty_; }
    void rendered() { dirty_=false; }
    void resize() { dirty_=true; }

    void on_key(KeySym key) {
        if(key!=XK_Escape) return;
        if(screen_==Screen::Game) screen_=Screen::Menu; else running_=false;
        dirty_=true;
    }

    void on_motion(int x,int y) {
        mx_=x; my_=y;
        const int old=hover_;
        hover_=screen_==Screen::Menu?button_at(x,y):-1;
        if(old!=hover_) dirty_=true;
    }

    void on_click(int x,int y) {
        if(screen_==Screen::Game){
            if(back().contains(x,y)){screen_=Screen::Menu;dirty_=true;}
            return;
        }
        const int b=button_at(x,y);
        if(b==0){screen_=Screen::Game;status_.clear();}
        else if(b==2) status_="SETTINGS WILL COME AFTER FIRST CITY INTERACTION";
        else if(b==3) running_=false;
        dirty_=true;
    }

    void draw(){ screen_==Screen::Menu ? draw_menu() : draw_game(); }

private:
    Framebuffer& fb_;
    ImageAsset menu_;
    Screen screen_=Screen::Menu;
    bool running_=true, dirty_=true;
    int mx_=0,my_=0,hover_=-1;
    std::string status_="PRE-ALPHA - FIRST NATIVE SHELL";
    const Color gold{217,177,95}, pale{242,215,154}, panel{19,14,12}, hi{55,40,27};
    const Color sand{185,131,71}, fertile{102,119,78}, nile{45,113,135};

    std::array<Rect,4> buttons() const {
        const int x=72,y=300,w=340,h=50,g=12;
        return {Rect{x,y,w,h},Rect{x,y+h+g,w,h},Rect{x,y+2*(h+g),w,h},Rect{x,y+3*(h+g),w,h}};
    }
    Rect back() const { return {fb_.width()-250,24,220,42}; }
    int button_at(int x,int y) const {
        auto b=buttons(); for(int i=0;i<4;++i) if(b[i].contains(x,y)) return i; return -1;
    }
    void button(Rect r,const std::string& label,bool enabled,bool hovered=false){
        fb_.blend_rect(r,hovered&&enabled?hi:panel,hovered&&enabled?225:185);
        fb_.rect(r,enabled?gold:Color{90,78,65},2);
        text(fb_,r.x+18,r.y+16,label,enabled?pale:Color{115,105,92},2);
    }

    void draw_menu(){
        if(menu_.valid()) fb_.blit_cover(menu_); else fb_.clear({8,18,35});
        fb_.blend_rect({38,30,520,610},{5,4,4},112);
        text(fb_,72,72,"EGYPT",gold,8);
        text(fb_,74,151,"A LIVING CITY ON THE NILE",pale,3);
        fb_.fill_rect({72,205,420,2},gold);
        auto b=buttons();
        button(b[0],"NEW GAME",true,hover_==0);
        button(b[1],"CONTINUE",false);
        button(b[2],"SETTINGS",true,hover_==2);
        button(b[3],"QUIT",true,hover_==3);
        if(!status_.empty()) text(fb_,72,570,status_,{210,192,160},2);
    }

    void draw_game(){
        const int w=fb_.width(),h=fb_.height();
        fb_.clear(sand);
        fb_.fill_rect({0,0,w,84},panel);
        text(fb_,24,20,"SETTLEMENT ON THE NILE",gold,3);
        text(fb_,24,58,"POPULATION 0   TREASURY 5000   YEAR 1   FLOOD FORECAST -",pale,2);
        Rect r=back(); button(r,"MAIN MENU",true,r.contains(mx_,my_));
        const int mapy=84,rx=w*58/100,rh=std::max(70,w*8/100);
        fb_.fill_rect({rx-rh-28,mapy,rh*2+56,h-mapy},fertile);
        fb_.fill_rect({rx-rh,mapy,rh*2,h-mapy},nile);
        for(int x=0;x<w;x+=32) fb_.line(x,mapy,x,h-1,{137,104,70});
        for(int y=mapy;y<h;y+=32) fb_.line(0,y,w-1,y,{137,104,70});
        const int hx=w*36/100,hy=mapy+(h-mapy)*52/100;
        fb_.fill_rect({hx-18,hy-14,36,28},{109,67,41});
        fb_.triangle(hx-22,hy-14,hx,hy-34,hx+22,hy-14,{140,90,50});
        text(fb_,22,h-34,"FIRST MILESTONE: NATIVE ENGINE SHELL ACTIVE",{70,52,38},2);
    }
};

class X11App {
public:
    X11App(int w,int h):fb_(w,h),game_(fb_){
        d_=XOpenDisplay(nullptr); if(!d_) throw std::runtime_error("Unable to open X11 display");
        s_=DefaultScreen(d_);
        win_=XCreateSimpleWindow(d_,RootWindow(d_,s_),100,100,w,h,0,BlackPixel(d_,s_),BlackPixel(d_,s_));
        XStoreName(d_,win_,"Egypt");
        XSelectInput(d_,win_,ExposureMask|KeyPressMask|ButtonPressMask|PointerMotionMask|StructureNotifyMask);
        del_=XInternAtom(d_,"WM_DELETE_WINDOW",False); XSetWMProtocols(d_,win_,&del_,1);
        gc_=XCreateGC(d_,win_,0,nullptr); XMapWindow(d_,win_); recreate(w,h);
    }

    ~X11App(){
        if(image_){image_->data=nullptr;XDestroyImage(image_);} if(gc_)XFreeGC(d_,gc_); if(win_)XDestroyWindow(d_,win_); if(d_)XCloseDisplay(d_);
    }

    int run(){
        while(game_.running()){
            XEvent e; XNextEvent(d_,&e);
            switch(e.type){
                case Expose: game_.resize(); break;
                case ConfigureNotify:
                    if(e.xconfigure.width!=fb_.width()||e.xconfigure.height!=fb_.height()){
                        fb_.resize(e.xconfigure.width,e.xconfigure.height); recreate(e.xconfigure.width,e.xconfigure.height); game_.resize();
                    } break;
                case MotionNotify: game_.on_motion(e.xmotion.x,e.xmotion.y); break;
                case ButtonPress: if(e.xbutton.button==Button1) game_.on_click(e.xbutton.x,e.xbutton.y); break;
                case KeyPress: game_.on_key(XLookupKeysym(&e.xkey,0)); break;
                case ClientMessage: if(static_cast<Atom>(e.xclient.data.l[0])==del_) return 0; break;
                default: break;
            }
            if(game_.dirty()){game_.draw();present();game_.rendered();}
        }
        return 0;
    }

private:
    Display* d_=nullptr; int s_=0; Window win_=0; GC gc_=0; Atom del_=0; XImage* image_=nullptr;
    Framebuffer fb_; Game game_;

    static unsigned long pack(std::uint8_t v,unsigned long mask){
        if (!mask) return 0;
        unsigned shift = 0;
        while (((mask >> shift) & 1UL) == 0UL) ++shift;
        const unsigned long max = mask >> shift;
        return ((static_cast<unsigned long>(v) * max + 127UL) / 255UL << shift) & mask;
    }
    void recreate(int w,int h){
        if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);image_=nullptr;}
        image_=XCreateImage(d_,DefaultVisual(d_,s_),DefaultDepth(d_,s_),ZPixmap,0,nullptr,w,h,32,0);
        if(!image_)throw std::runtime_error("Unable to create XImage");
        image_->data=static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line)*h,1));
        if(!image_->data)throw std::bad_alloc();
    }
    void present(){
        const auto& p=fb_.pixels(); const int w=fb_.width(),h=fb_.height();
        for(int y=0;y<h;++y)for(int x=0;x<w;++x){
            const Color c=p[static_cast<std::size_t>(y*w+x)];
            XPutPixel(image_,x,y,pack(c.r,image_->red_mask)|pack(c.g,image_->green_mask)|pack(c.b,image_->blue_mask));
        }
        XPutImage(d_,win_,gc_,image_,0,0,0,0,w,h); XFlush(d_);
    }
};

} // namespace egypt

int main(){
    try { egypt::X11App app(1280,720); return app.run(); }
    catch(const std::exception& e){ std::cerr<<"Egypt failed to start: "<<e.what()<<'\n'; return 1; }
}
