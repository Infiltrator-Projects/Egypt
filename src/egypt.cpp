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
using Color=common::Color; using ImageAsset=common::Image;
struct Rect{int x,y,w,h; bool contains(int px,int py)const{return px>=x&&py>=y&&px<x+w&&py<y+h;}};
enum class Screen{Menu,Game};
enum class Tool{Inspect,Road,House,ClayPit,Potter,Bulldoze};

class Framebuffer{
public:
    Framebuffer(int w,int h){resize(w,h);} void resize(int w,int h){w_=std::max(1,w);h_=std::max(1,h);p_.assign(static_cast<std::size_t>(w_*h_),{0,0,0});}
    int width()const{return w_;} int height()const{return h_;} const std::vector<Color>& pixels()const{return p_;}
    void clear(Color c){std::fill(p_.begin(),p_.end(),c);} void pixel(int x,int y,Color c){if(x>=0&&y>=0&&x<w_&&y<h_)p_[static_cast<std::size_t>(y*w_+x)]=c;}
    void fill_rect(Rect r,Color c){for(int y=std::max(0,r.y);y<std::min(h_,r.y+r.h);++y)for(int x=std::max(0,r.x);x<std::min(w_,r.x+r.w);++x)pixel(x,y,c);}
    void blend_rect(Rect r,Color c,std::uint8_t a){const unsigned ia=255U-a;for(int y=std::max(0,r.y);y<std::min(h_,r.y+r.h);++y)for(int x=std::max(0,r.x);x<std::min(w_,r.x+r.w);++x){Color&d=p_[static_cast<std::size_t>(y*w_+x)];d.r=static_cast<std::uint8_t>((d.r*ia+c.r*a)/255U);d.g=static_cast<std::uint8_t>((d.g*ia+c.g*a)/255U);d.b=static_cast<std::uint8_t>((d.b*ia+c.b*a)/255U);}}
    void line(int x0,int y0,int x1,int y1,Color c){int dx=std::abs(x1-x0),sx=x0<x1?1:-1,dy=-std::abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;for(;;){pixel(x0,y0,c);if(x0==x1&&y0==y1)break;int e2=2*err;if(e2>=dy){err+=dy;x0+=sx;}if(e2<=dx){err+=dx;y0+=sy;}}}
    void rect(Rect r,Color c,int t=1){fill_rect({r.x,r.y,r.w,t},c);fill_rect({r.x,r.y+r.h-t,r.w,t},c);fill_rect({r.x,r.y,t,r.h},c);fill_rect({r.x+r.w-t,r.y,t,r.h},c);}
    void triangle(int x0,int y0,int x1,int y1,int x2,int y2,Color c){const int minx=std::max(0,std::min({x0,x1,x2})),maxx=std::min(w_-1,std::max({x0,x1,x2})),miny=std::max(0,std::min({y0,y1,y2})),maxy=std::min(h_-1,std::max({y0,y1,y2}));auto edge=[](int ax,int ay,int bx,int by,int px,int py){return(px-ax)*(by-ay)-(py-ay)*(bx-ax);};for(int y=miny;y<=maxy;++y)for(int x=minx;x<=maxx;++x){int a=edge(x0,y0,x1,y1,x,y),b=edge(x1,y1,x2,y2,x,y),d=edge(x2,y2,x0,y0,x,y);if((a>=0&&b>=0&&d>=0)||(a<=0&&b<=0&&d<=0))pixel(x,y,c);}}
    void quad(IsoPoint a,IsoPoint b,IsoPoint c,IsoPoint d,Color col){triangle(a.x,a.y,b.x,b.y,c.x,c.y,col);triangle(a.x,a.y,c.x,c.y,d.x,d.y,col);}
    void diamond(IsoPoint p,int tw,int th,Color fill,Color edge){IsoPoint t{p.x,p.y-th/2},r{p.x+tw/2,p.y},b{p.x,p.y+th/2},l{p.x-tw/2,p.y};triangle(t.x,t.y,r.x,r.y,b.x,b.y,fill);triangle(t.x,t.y,b.x,b.y,l.x,l.y,fill);diamond_outline(p,tw,th,edge);}
    void diamond_outline(IsoPoint p,int tw,int th,Color edge){IsoPoint t{p.x,p.y-th/2},r{p.x+tw/2,p.y},b{p.x,p.y+th/2},l{p.x-tw/2,p.y};line(t.x,t.y,r.x,r.y,edge);line(r.x,r.y,b.x,b.y,edge);line(b.x,b.y,l.x,l.y,edge);line(l.x,l.y,t.x,t.y,edge);}
    Color sample(const ImageAsset&i,float x,float y)const{x=std::clamp(x,0.f,float(i.width-1));y=std::clamp(y,0.f,float(i.height-1));int x0=int(x),y0=int(y),x1=std::min(x0+1,i.width-1),y1=std::min(y0+1,i.height-1);float fx=x-x0,fy=y-y0;auto at=[&](int sx,int sy){return i.pixels[static_cast<std::size_t>(sy*i.width+sx)];};auto mix=[](std::uint8_t a,std::uint8_t b,float t){return float(a)+(float(b)-float(a))*t;};Color a=at(x0,y0),b=at(x1,y0),c=at(x0,y1),d=at(x1,y1);return{std::uint8_t(std::clamp(mix(std::uint8_t(mix(a.r,b.r,fx)),std::uint8_t(mix(c.r,d.r,fx)),fy),0.f,255.f)),std::uint8_t(std::clamp(mix(std::uint8_t(mix(a.g,b.g,fx)),std::uint8_t(mix(c.g,d.g,fx)),fy),0.f,255.f)),std::uint8_t(std::clamp(mix(std::uint8_t(mix(a.b,b.b,fx)),std::uint8_t(mix(c.b,d.b,fx)),fy),0.f,255.f))};}
    void blit_cover(const ImageAsset&i){if(!i.valid())return;float sx=float(w_)/i.width,sy=float(h_)/i.height,s=std::max(sx,sy),dw=i.width*s,dh=i.height*s,ox=(w_-dw)*.5f,oy=(h_-dh)*.5f;for(int y=0;y<h_;++y)for(int x=0;x<w_;++x)p_[static_cast<std::size_t>(y*w_+x)]=sample(i,(x-ox)/s,(y-oy)/s);}
private:int w_=1,h_=1;std::vector<Color> p_;
};

using Glyph=std::array<std::uint8_t,7>;
const std::unordered_map<char,Glyph> FONT={
{' ',{0,0,0,0,0,0,0}},{'A',{14,17,17,31,17,17,17}},{'B',{30,17,17,30,17,17,30}},{'C',{14,17,16,16,16,17,14}},{'D',{30,17,17,17,17,17,30}},{'E',{31,16,16,30,16,16,31}},{'F',{31,16,16,30,16,16,16}},{'G',{14,17,16,23,17,17,14}},{'H',{17,17,17,31,17,17,17}},{'I',{14,4,4,4,4,4,14}},{'J',{1,1,1,1,17,17,14}},{'K',{17,18,20,24,20,18,17}},{'L',{16,16,16,16,16,16,31}},{'M',{17,27,21,21,17,17,17}},{'N',{17,25,21,19,17,17,17}},{'O',{14,17,17,17,17,17,14}},{'P',{30,17,17,30,16,16,16}},{'Q',{14,17,17,17,21,18,13}},{'R',{30,17,17,30,20,18,17}},{'S',{15,16,16,14,1,1,30}},{'T',{31,4,4,4,4,4,4}},{'U',{17,17,17,17,17,17,14}},{'V',{17,17,17,17,17,10,4}},{'W',{17,17,17,21,21,21,10}},{'X',{17,17,10,4,10,17,17}},{'Y',{17,17,10,4,4,4,4}},{'Z',{31,1,2,4,8,16,31}},{'0',{14,17,19,21,25,17,14}},{'1',{4,12,4,4,4,4,14}},{'2',{14,17,1,2,4,8,31}},{'3',{30,1,1,14,1,1,30}},{'4',{2,6,10,18,31,2,2}},{'5',{31,16,16,30,1,1,30}},{'6',{14,16,16,30,17,17,14}},{'7',{31,1,2,4,8,8,8}},{'8',{14,17,17,14,17,17,14}},{'9',{14,17,17,15,1,1,14}},{'-',{0,0,0,31,0,0,0}},{'.',{0,0,0,0,0,12,12}},{':',{0,12,12,0,12,12,0}},{'/',{1,2,2,4,8,8,16}}
};
static void text(Framebuffer&fb,int x,int y,const std::string&s,Color c,int scale=2){int cur=x;for(char raw:s){char ch=raw>='a'&&raw<='z'?char(raw-'a'+'A'):raw;auto it=FONT.find(ch);const Glyph&g=it==FONT.end()?FONT.at(' '):it->second;for(int r=0;r<7;++r)for(int col=0;col<5;++col)if((g[r]>>(4-col))&1U)fb.fill_rect({cur+col*scale,y+r*scale,scale,scale},c);cur+=6*scale;}}

class Game{
public:
    explicit Game(Framebuffer&fb):fb_(fb),menu_(common::load_e16("build/assets/menu.e16")){if(!menu_.valid())std::cerr<<"Egypt: menu artwork failed to load\n";}
    bool running()const{return running_;} bool dirty()const{return dirty_;} void rendered(){dirty_=false;} void resize(){dirty_=true;} void tick(){if(screen_==Screen::Game){world_.tick();dirty_=true;}}
    void on_key(KeySym k){
        if(k==XK_Escape){if(screen_==Screen::Game)screen_=Screen::Menu;else running_=false;dirty_=true;return;}
        if(screen_!=Screen::Game)return;
        if(k==XK_Left)cam_.pan_x+=36; else if(k==XK_Right)cam_.pan_x-=36; else if(k==XK_Up)cam_.pan_y+=28; else if(k==XK_Down)cam_.pan_y-=28;
        else if(k==XK_plus||k==XK_equal)cam_.zoom_percent=std::min(160,cam_.zoom_percent+10); else if(k==XK_minus)cam_.zoom_percent=std::max(50,cam_.zoom_percent-10);
        else if(k>=XK_1&&k<=XK_6)tool_=static_cast<Tool>(k-XK_1);
        update_hover();dirty_=true;
    }
    void on_motion(int x,int y){mx_=x;my_=y;int oldx=hover_x_,oldy=hover_y_;update_hover();if(oldx!=hover_x_||oldy!=hover_y_)dirty_=true;}
    void on_click(int x,int y){mx_=x;my_=y;if(screen_==Screen::Menu){int b=menu_button_at(x,y);if(b==0){screen_=Screen::Game;status_.clear();reset_camera();}else if(b==2)status_="SETTINGS WILL FOLLOW THE PLAYABLE CITY SLICE";else if(b==3)running_=false;dirty_=true;return;}
        if(main_menu_rect().contains(x,y)){screen_=Screen::Menu;dirty_=true;return;}
        auto tb=tool_buttons();for(int i=0;i<6;++i)if(tb[i].contains(x,y)){tool_=static_cast<Tool>(i);dirty_=true;return;}
        update_hover();if(!world_.in_bounds(hover_x_,hover_y_))return;selected_x_=hover_x_;selected_y_=hover_y_;
        if(tool_==Tool::Bulldoze)world_.bulldoze(selected_x_,selected_y_);else if(tool_!=Tool::Inspect)world_.place(tool_structure(),selected_x_,selected_y_);dirty_=true;
    }
    void draw(){screen_==Screen::Menu?draw_menu():draw_game();}
private:
    Framebuffer&fb_;ImageAsset menu_;World world_;IsoCamera cam_;Screen screen_=Screen::Menu;Tool tool_=Tool::Inspect;bool running_=true,dirty_=true;int mx_=0,my_=0,hover_x_=-1,hover_y_=-1,selected_x_=-1,selected_y_=-1;std::string status_="PRE-ALPHA - NATIVE ENGINE";
    const Color gold{217,177,95},pale{242,215,154},panel{19,14,12},hi{55,40,27};
    void reset_camera(){cam_.origin_x=fb_.width()/2+80;cam_.origin_y=150;cam_.pan_x=-260;cam_.pan_y=-30;cam_.zoom_percent=90;update_hover();}
    std::array<Rect,4> menu_buttons()const{int x=72,y=300,w=340,h=50,g=12;return{Rect{x,y,w,h},Rect{x,y+h+g,w,h},Rect{x,y+2*(h+g),w,h},Rect{x,y+3*(h+g),w,h}};}
    int menu_button_at(int x,int y)const{auto b=menu_buttons();for(int i=0;i<4;++i)if(b[i].contains(x,y))return i;return-1;}
    Rect main_menu_rect()const{return{fb_.width()-210,18,190,42};}
    std::array<Rect,6> tool_buttons()const{std::array<Rect,6> r{};int x=18,y=fb_.height()-54,w=118,h=38,g=8;for(int i=0;i<6;++i)r[i]={x+i*(w+g),y,w,h};return r;}
    void button(Rect r,const std::string&label,bool enabled,bool active=false){fb_.blend_rect(r,active?hi:panel,active?235:205);fb_.rect(r,enabled?gold:Color{90,78,65},2);text(fb_,r.x+10,r.y+12,label,enabled?pale:Color{115,105,92},2);}
    Structure tool_structure()const{switch(tool_){case Tool::Road:return Structure::Road;case Tool::House:return Structure::House;case Tool::ClayPit:return Structure::ClayPit;case Tool::Potter:return Structure::Potter;default:return Structure::Empty;}}
    const char* tool_name()const{switch(tool_){case Tool::Inspect:return"INSPECT";case Tool::Road:return"ROAD";case Tool::House:return"HOUSE";case Tool::ClayPit:return"CLAY PIT";case Tool::Potter:return"POTTER";case Tool::Bulldoze:return"BULLDOZE";}return"?";}
    void update_hover(){if(screen_!=Screen::Game){hover_x_=hover_y_=-1;return;}int x=0,y=0;if(!cam_.pick(mx_,my_,x,y)){hover_x_=hover_y_=-1;return;}if(world_.in_bounds(x,y)){hover_x_=x;hover_y_=y;}else hover_x_=hover_y_=-1;}
    Color terrain_color(Terrain t)const{switch(t){case Terrain::Desert:return{191,139,73};case Terrain::Floodplain:return{113,132,77};case Terrain::Water:return{43,110,139};case Terrain::Clay:return{157,87,54};case Terrain::Reeds:return{73,122,72};}return{255,0,255};}
    void draw_block(IsoPoint p,int tw,int th,int height,Color top,Color left,Color right){int hw=tw/2,hh=th/2;IsoPoint bt{p.x,p.y-hh},br{p.x+hw,p.y},bb{p.x,p.y+hh},bl{p.x-hw,p.y};IsoPoint tt{bt.x,bt.y-height},tr{br.x,br.y-height},tb{bb.x,bb.y-height},tl{bl.x,bl.y-height};fb_.quad(bl,bb,tb,tl,left);fb_.quad(bb,br,tr,tb,right);fb_.quad(tt,tr,tb,tl,top);fb_.line(tt.x,tt.y,tr.x,tr.y,gold);fb_.line(tr.x,tr.y,tb.x,tb.y,gold);fb_.line(tb.x,tb.y,tl.x,tl.y,gold);fb_.line(tl.x,tl.y,tt.x,tt.y,gold);}
    void draw_structure(const Tile&t,IsoPoint p,int tw,int th){switch(t.structure){case Structure::Empty:break;case Structure::Road:fb_.diamond(p,tw*3/4,th*3/4,{122,91,57},{103,72,43});break;case Structure::House:draw_block(p,tw*2/3,th*2/3,18,{190,151,91},{122,78,48},{145,92,51});break;case Structure::ClayPit:fb_.diamond({p.x,p.y+2},tw*2/3,th/2,{91,54,43},{66,42,33});break;case Structure::Potter:draw_block(p,tw*3/4,th*3/4,24,{165,122,79},{111,71,48},{133,82,51});fb_.fill_rect({p.x+8,p.y-39,5,18},{72,54,45});break;}}
    void draw_menu(){if(menu_.valid())fb_.blit_cover(menu_);else fb_.clear({8,18,35});fb_.blend_rect({38,30,520,610},{5,4,4},112);text(fb_,72,72,"EGYPT",gold,8);text(fb_,74,151,"A LIVING CITY ON THE NILE",pale,3);fb_.fill_rect({72,205,420,2},gold);auto b=menu_buttons();button(b[0],"NEW GAME",true,b[0].contains(mx_,my_));button(b[1],"CONTINUE",false);button(b[2],"SETTINGS",true,b[2].contains(mx_,my_));button(b[3],"QUIT",true,b[3].contains(mx_,my_));if(!status_.empty())text(fb_,72,570,status_,{210,192,160},2);}
    void draw_game(){
        fb_.clear({47,34,28});fb_.fill_rect({0,0,fb_.width(),78},panel);text(fb_,20,14,"SETTLEMENT ON THE NILE",gold,3);std::string stats="POP "+std::to_string(world_.population())+"   TREASURY "+std::to_string(world_.treasury())+"   TICK "+std::to_string(world_.simulation_ticks())+"   TOOL "+tool_name();text(fb_,20,49,stats,pale,2);button(main_menu_rect(),"MAIN MENU",true,main_menu_rect().contains(mx_,my_));
        const int tw=cam_.tile_w(),th=cam_.tile_h();
        for(int sum=0;sum<World::kWidth+World::kHeight-1;++sum){for(int y=0;y<World::kHeight;++y){int x=sum-y;if(!world_.in_bounds(x,y))continue;IsoPoint p=cam_.project(x,y);if(p.x<-tw||p.x>fb_.width()+tw||p.y<70-th||p.y>fb_.height()+th)continue;const Tile&t=world_.tile(x,y);Color c=terrain_color(t.terrain);fb_.diamond(p,tw,th,c,{87,68,48});if(t.terrain==Terrain::Reeds){for(int k=-2;k<=2;++k)fb_.line(p.x+k*3,p.y,p.x+k*3+1,p.y-10,{40,82,43});}draw_structure(t,p,tw,th);}}
        if(world_.in_bounds(hover_x_,hover_y_)){IsoPoint p=cam_.project(hover_x_,hover_y_);fb_.diamond_outline(p,tw,th,{255,230,130});}
        auto tb=tool_buttons();const char* labels[6]={"INSPECT","ROAD","HOUSE","CLAY PIT","POTTER","BULLDOZE"};for(int i=0;i<6;++i)button(tb[i],labels[i],true,static_cast<int>(tool_)==i);
        fb_.blend_rect({fb_.width()-330,88,312,94},{8,7,6},205);fb_.rect({fb_.width()-330,88,312,94},gold,1);if(world_.in_bounds(selected_x_,selected_y_)){const Tile&t=world_.tile(selected_x_,selected_y_);text(fb_,fb_.width()-314,101,"TILE "+std::to_string(selected_x_)+","+std::to_string(selected_y_),pale,2);text(fb_,fb_.width()-314,123,World::terrain_name(t.terrain),gold,2);text(fb_,fb_.width()-314,145,World::structure_name(t.structure),gold,2);if(t.structure==Structure::House)text(fb_,fb_.width()-160,145,"POP "+std::to_string(t.population),pale,2);}else text(fb_,fb_.width()-314,119,"CLICK A TILE TO INSPECT",pale,2);
    }
};

class X11App{
public:
    X11App(int w,int h):fb_(w,h),game_(fb_){d_=XOpenDisplay(nullptr);if(!d_)throw std::runtime_error("Unable to open X11 display");s_=DefaultScreen(d_);win_=XCreateSimpleWindow(d_,RootWindow(d_,s_),100,100,w,h,0,BlackPixel(d_,s_),BlackPixel(d_,s_));XStoreName(d_,win_,"Egypt");XSelectInput(d_,win_,ExposureMask|KeyPressMask|ButtonPressMask|PointerMotionMask|StructureNotifyMask);del_=XInternAtom(d_,"WM_DELETE_WINDOW",False);XSetWMProtocols(d_,win_,&del_,1);gc_=XCreateGC(d_,win_,0,nullptr);XMapWindow(d_,win_);recreate(w,h);if(!infiltratr_fixed_step_configure(&scheduler_,1000000000ULL,4ULL,500000000ULL,8ULL))throw std::runtime_error("Common fixed-step scheduler configuration failed");infiltratr_fixed_step_reset(&scheduler_,now_ns());}
    ~X11App(){if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);}if(gc_)XFreeGC(d_,gc_);if(win_)XDestroyWindow(d_,win_);if(d_)XCloseDisplay(d_);}
    int run(){while(game_.running()){while(XPending(d_)>0){XEvent e;XNextEvent(d_,&e);switch(e.type){case Expose:game_.resize();break;case ConfigureNotify:if(e.xconfigure.width!=fb_.width()||e.xconfigure.height!=fb_.height()){fb_.resize(e.xconfigure.width,e.xconfigure.height);recreate(e.xconfigure.width,e.xconfigure.height);game_.resize();}break;case MotionNotify:game_.on_motion(e.xmotion.x,e.xmotion.y);break;case ButtonPress:if(e.xbutton.button==Button1)game_.on_click(e.xbutton.x,e.xbutton.y);break;case KeyPress:game_.on_key(XLookupKeysym(&e.xkey,0));break;case ClientMessage:if(static_cast<Atom>(e.xclient.data.l[0])==del_)return 0;break;default:break;}}
        InfiltratrFixedStepResult r{};if(infiltratr_fixed_step_advance(&scheduler_,now_ns(),&r))for(std::uint64_t i=0;i<r.steps_to_run;++i)game_.tick();if(game_.dirty()){game_.draw();present();game_.rendered();}std::this_thread::sleep_for(std::chrono::milliseconds(8));}return 0;}
private:
    Display*d_=nullptr;int s_=0;Window win_=0;GC gc_=0;Atom del_=0;XImage*image_=nullptr;Framebuffer fb_;Game game_;InfiltratrFixedStepScheduler scheduler_{};
    static std::uint64_t now_ns(){return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
    static unsigned long pack(std::uint8_t v,unsigned long mask){if(!mask)return 0;unsigned sh=0;while(((mask>>sh)&1UL)==0UL)++sh;unsigned long max=mask>>sh;return((static_cast<unsigned long>(v)*max+127UL)/255UL<<sh)&mask;}
    void recreate(int w,int h){if(image_){std::free(image_->data);image_->data=nullptr;XDestroyImage(image_);image_=nullptr;}image_=XCreateImage(d_,DefaultVisual(d_,s_),DefaultDepth(d_,s_),ZPixmap,0,nullptr,w,h,32,0);if(!image_)throw std::runtime_error("Unable to create XImage");image_->data=static_cast<char*>(std::calloc(static_cast<std::size_t>(image_->bytes_per_line)*h,1));if(!image_->data)throw std::bad_alloc();}
    void present(){const auto&p=fb_.pixels();int w=fb_.width(),h=fb_.height();for(int y=0;y<h;++y)for(int x=0;x<w;++x){Color c=p[static_cast<std::size_t>(y*w+x)];XPutPixel(image_,x,y,pack(c.r,image_->red_mask)|pack(c.g,image_->green_mask)|pack(c.b,image_->blue_mask));}XPutImage(d_,win_,gc_,image_,0,0,0,0,w,h);XFlush(d_);}
};
}
int main(){try{egypt::X11App app(1280,720);return app.run();}catch(const std::exception&e){std::cerr<<"Egypt failed to start: "<<e.what()<<'\n';return 1;}}
