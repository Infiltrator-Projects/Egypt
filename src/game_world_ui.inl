    void draw_cloud_shadows() {
        const int span=fb_.width()+900;
        const int x=static_cast<int>(std::fmod(atmosphere_time_*34.0,static_cast<double>(span)))-520;
        const int y=fb_.height()/3;
        const Color shadow{43,47,40};
        auto soft_blob=[&](int cx,int cy,int w,int h,std::uint8_t alpha){
            const int bands=8;
            for(int i=0;i<bands;++i){
                const double t=(double(i)+0.5)/bands;
                const double yy=(t*2.0-1.0);
                const double profile=std::sqrt(std::max(0.0,1.0-yy*yy));
                const int bw=std::max(4,static_cast<int>(w*profile));
                const int bh=std::max(2,h/bands+2);
                const int by=cy-h/2+i*h/bands;
                const std::uint8_t a=static_cast<std::uint8_t>(alpha*(0.60+0.40*profile));
                fb_.blend_rect({cx-bw/2,by,bw,bh},shadow,a);
            }
        };
        soft_blob(x+210,y,430,105,24);
        soft_blob(x+340,y-42,310,78,17);
        soft_blob(x+390,y+58,340,82,15);
        const int x2=(x+fb_.width()/2+560)%span-320;
        soft_blob(x2,y+170,360,88,15);
        soft_blob(x2+95,y+133,250,64,11);
    }

    void draw_minimap() {
        const Rect r=minimap_rect();
        fb_.blend_rect(r,{37,28,20},220);fb_.rect(r,gold,2);
        const int inner_x=r.x+2,inner_y=r.y+2,inner_w=r.w-4,inner_h=r.h-4;

        auto mix=[](Color a,Color b,double t){
            return Color{
                static_cast<std::uint8_t>(std::lround(a.r+(b.r-a.r)*t)),
                static_cast<std::uint8_t>(std::lround(a.g+(b.g-a.g)*t)),
                static_cast<std::uint8_t>(std::lround(a.b+(b.b-a.b)*t))
            };
        };
        for(int py=0;py<inner_h;++py){
            const double fy=double(py)*(World::kHeight-1)/std::max(1,inner_h-1);
            const int y0=std::clamp(static_cast<int>(std::floor(fy)),0,World::kHeight-1);
            const int y1=std::min(World::kHeight-1,y0+1);
            const double ty=fy-y0;
            for(int px=0;px<inner_w;++px){
                const double fx=double(px)*(World::kWidth-1)/std::max(1,inner_w-1);
                const int x0=std::clamp(static_cast<int>(std::floor(fx)),0,World::kWidth-1);
                const int x1=std::min(World::kWidth-1,x0+1);
                const double tx=fx-x0;
                const Color c00=terrain_color(world_.tile(x0,y0).terrain,x0,y0);
                const Color c10=terrain_color(world_.tile(x1,y0).terrain,x1,y0);
                const Color c01=terrain_color(world_.tile(x0,y1).terrain,x0,y1);
                const Color c11=terrain_color(world_.tile(x1,y1).terrain,x1,y1);
                const Color top=mix(c00,c10,tx);
                const Color bottom=mix(c01,c11,tx);
                fb_.pixel(inner_x+px,inner_y+py,mix(top,bottom,ty));
            }
        }

        for(int y=0;y<World::kHeight;++y){
            for(int x=0;x<World::kWidth;++x){
                const Tile& t=world_.tile(x,y);
                if(t.structure==Structure::Empty)continue;
                const int px=inner_x+x*(inner_w-1)/std::max(1,World::kWidth-1);
                const int py=inner_y+y*(inner_h-1)/std::max(1,World::kHeight-1);
                const int size=t.structure==Structure::Road?2:3;
                fb_.fill_rect({px-size/2,py-size/2,size,size},flat_structure_color(t.structure));
            }
        }

        int cx=World::kWidth/2,cy=World::kHeight/2;
        if(!cam_.pick(fb_.width()/2,fb_.height()/2,cx,cy)){cx=World::kWidth/2;cy=World::kHeight/2;}
        const int rx=std::clamp(inner_w/5,18,70),ry=std::clamp(inner_h/4,14,50);
        const int px=inner_x+std::clamp(cx,0,World::kWidth-1)*inner_w/World::kWidth-rx/2;
        const int py=inner_y+std::clamp(cy,0,World::kHeight-1)*inner_h/World::kHeight-ry/2;
        fb_.rect({px,py,rx,ry},{250,239,199},1);
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
        text(fb_,fb_.width()/2-114,408,"SCROLL SPEED",pale,2);const auto sb=scroll_speed_buttons();button(sb[0],"SLOW",true,scroll_speed_px_<700);button(sb[1],"NORMAL",true,scroll_speed_px_>=700&&scroll_speed_px_<1150);button(sb[2],"FAST",true,scroll_speed_px_>=1150);
        text(fb_,fb_.width()/2-300,520,"RIGHT OR MIDDLE DRAG PANS THE MAP",pale,2);text(fb_,fb_.width()/2-300,546,"WHEEL ZOOMS  F FLAT VIEW  SPACE PAUSE",pale,2);button(settings_back(),"BACK",true,settings_back().contains(mx_,my_));
    }

    void handle_settings_click(int x,int y) {
        if(settings_back().contains(x,y)){screen_=Screen::Menu;dirty_=true;return;}
        const auto rb=resolution_buttons();for(int i=0;i<3;++i)if(rb[i].contains(x,y)){resolution_index_=i;requested_w_=kResolutions[i][0];requested_h_=kResolutions[i][1];resize_pending_=true;dirty_=true;return;}
        if(edge_scroll_rect().contains(x,y)){edge_scroll_=!edge_scroll_;dirty_=true;return;}
        const auto sb=scroll_speed_buttons();if(sb[0].contains(x,y))scroll_speed_px_=550.0;else if(sb[1].contains(x,y))scroll_speed_px_=900.0;else if(sb[2].contains(x,y))scroll_speed_px_=1400.0;else return;dirty_=true;
    }
