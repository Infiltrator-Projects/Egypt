    void draw_cloud_shadows() {
        const int span=fb_.width()+700;
        const int x=static_cast<int>(std::fmod(atmosphere_time_*32.0,static_cast<double>(span)))-420;
        const int y=fb_.height()/3;
        const Color shadow{44,46,39};
        fb_.blend_rect({x,y,430,92},shadow,24);
        fb_.blend_rect({x+80,y-44,320,72},shadow,18);
        fb_.blend_rect({x+160,y+70,360,78},shadow,16);
        const int x2=(x+fb_.width()/2+380)%span-350;
        fb_.blend_rect({x2,y+160,360,82},shadow,16);
        fb_.blend_rect({x2+90,y+128,260,62},shadow,12);
    }

    void draw_minimap() {
        const Rect r=minimap_rect();
        fb_.blend_rect(r,{37,28,20},220);fb_.rect(r,gold,2);
        const int inner_x=r.x+2,inner_y=r.y+2,inner_w=r.w-4,inner_h=r.h-4;
        for(int y=0;y<World::kHeight;++y){
            const int sy0=inner_y+y*inner_h/World::kHeight;
            const int sy1=inner_y+(y+1)*inner_h/World::kHeight;
            for(int x=0;x<World::kWidth;++x){
                const int sx0=inner_x+x*inner_w/World::kWidth;
                const int sx1=inner_x+(x+1)*inner_w/World::kWidth;
                const Tile& t=world_.tile(x,y);
                Color c=terrain_color(t.terrain,x,y);
                if(t.structure!=Structure::Empty)c=flat_structure_color(t.structure);
                fb_.fill_rect({sx0,sy0,std::max(1,sx1-sx0),std::max(1,sy1-sy0)},c);
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
        text(fb_,fb_.width()/2-114,408,"SCROLL SPEED",pale,2);const auto sb=scroll_speed_buttons();button(sb[0],"SLOW",true,scroll_speed_px_<500);button(sb[1],"NORMAL",true,scroll_speed_px_>=500&&scroll_speed_px_<850);button(sb[2],"FAST",true,scroll_speed_px_>=850);
        text(fb_,fb_.width()/2-300,520,"RIGHT OR MIDDLE DRAG PANS THE MAP",pale,2);text(fb_,fb_.width()/2-300,546,"WHEEL ZOOMS  F FLAT VIEW  SPACE PAUSE",pale,2);button(settings_back(),"BACK",true,settings_back().contains(mx_,my_));
    }

    void handle_settings_click(int x,int y) {
        if(settings_back().contains(x,y)){screen_=Screen::Menu;dirty_=true;return;}
        const auto rb=resolution_buttons();for(int i=0;i<3;++i)if(rb[i].contains(x,y)){resolution_index_=i;requested_w_=kResolutions[i][0];requested_h_=kResolutions[i][1];resize_pending_=true;dirty_=true;return;}
        if(edge_scroll_rect().contains(x,y)){edge_scroll_=!edge_scroll_;dirty_=true;return;}
        const auto sb=scroll_speed_buttons();if(sb[0].contains(x,y))scroll_speed_px_=360.0;else if(sb[1].contains(x,y))scroll_speed_px_=620.0;else if(sb[2].contains(x,y))scroll_speed_px_=980.0;else return;dirty_=true;
    }
