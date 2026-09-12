    Color terrain_color(Terrain terrain, int x, int y) const {
        Color c{};
        switch (terrain) {
            case Terrain::Desert: c={194,153,88}; break;
            case Terrain::Floodplain: c={108,133,72}; break;
            case Terrain::Water: c={38,94,120}; break;
            case Terrain::Clay: c={143,84,59}; break;
            case Terrain::Reeds: c={67,116,65}; break;
        }
        const int jitter=((x*17+y*31)%5)-2;
        auto add=[&](std::uint8_t v){return static_cast<std::uint8_t>(std::clamp(int(v)+jitter,0,255));};
        return {add(c.r),add(c.g),add(c.b)};
    }

    void draw_terrain_detail(Terrain terrain,int x,int y,IsoPoint p,int tw,int th) {
        const int seed=(x*73+y*151+x*y*7)&255;
        if(terrain==Terrain::Desert){
            const Color dark{164,126,73},light{215,178,107};
            const int ox=(seed%11)-5,oy=((seed/11)%7)-3;
            fb_.pixel(p.x+ox,p.y+oy,(seed&1)?dark:light);
            fb_.pixel(p.x-ox/2+7,p.y-oy/2-2,dark);
            if((seed%9)==0)fb_.line(p.x-tw/7,p.y+th/8,p.x+tw/10,p.y+th/12,{177,137,77});
        }else if(terrain==Terrain::Floodplain){
            const Color grass{70,103,51};
            const int ox=(seed%15)-7;
            fb_.line(p.x+ox,p.y+3,p.x+ox+1,p.y-3,grass);
            if((seed%3)==0)fb_.line(p.x-ox/2,p.y+1,p.x-ox/2+1,p.y-5,{83,112,55});
        }else if(terrain==Terrain::Clay){
            fb_.line(p.x-tw/8,p.y+2,p.x+tw/10,p.y-2,{112,63,48});
            if((seed&3)==0)fb_.pixel(p.x+7,p.y+3,{181,104,68});
        }
    }

    Color flat_structure_color(Structure s) const {
        switch (s) {
            case Structure::Road: return {205,184,145};
            case Structure::House: return {238,222,184};
            case Structure::Farm: return {92,151,70};
            case Structure::Granary: return {216,176,94};
            case Structure::Market: return {194,90,71};
            case Structure::Well: return {74,155,184};
            case Structure::HuntingLodge: return {107,90,55};
            case Structure::ClayPit: return {145,78,55};
            case Structure::Potter: return {191,119,75};
            default: return {0,0,0};
        }
    }

    void draw_block(IsoPoint p, int tw, int th, int height, Color top, Color left, Color right) {
        fb_.diamond({p.x+5,p.y+4},tw*4/5,th/2,{99,76,52},{99,76,52});
        const int hw=tw/2, hh=th/2;
        const IsoPoint bt{p.x,p.y-hh},br{p.x+hw,p.y},bb{p.x,p.y+hh},bl{p.x-hw,p.y};
        const IsoPoint tt{bt.x,bt.y-height},tr{br.x,br.y-height},tb{bb.x,bb.y-height},tl{bl.x,bl.y-height};
        fb_.quad(bl,bb,tb,tl,left); fb_.quad(bb,br,tr,tb,right); fb_.quad(tt,tr,tb,tl,top);
        fb_.line(tl.x,tl.y,tt.x,tt.y,{246,222,171});
    }

    bool merged_anchor(int x,int y) const {
        return world_.in_bounds(x,y) &&
               world_.tile(x,y).structure==Structure::House &&
               world_.is_residence_anchor(x,y) &&
               world_.residence_tiles(x,y)>=4;
    }

    bool merged_part(int x,int y) const {
        return world_.in_bounds(x,y) &&
               world_.tile(x,y).structure==Structure::House &&
               !world_.is_residence_anchor(x,y) &&
               world_.residence_tiles(x,y)>=4;
    }

    void draw_house(const Tile& tile, IsoPoint p, int tw, int th) {
        if(tile.housing_level==0) {
            const int h=tile.population>=5?19:15;
            draw_block(p,tw*2/3,th*2/3,h,{202,161,95},{129,78,46},{151,91,49});
            fb_.fill_rect({p.x-3,p.y-h+5,6,10},{72,48,34});
            fb_.line(p.x-13,p.y-h,p.x,p.y-h-8,{119,73,43});
            fb_.line(p.x,p.y-h-8,p.x+13,p.y-h,{119,73,43});
        } else if(tile.housing_level==1) {
            draw_block(p,tw*4/5,th*4/5,25,{220,188,122},{146,91,55},{173,108,58});
            fb_.fill_rect({p.x-4,p.y-19,8,13},{67,47,35});
            fb_.fill_rect({p.x-16,p.y-24,5,6},{52,91,108});
            fb_.fill_rect({p.x+11,p.y-24,5,6},{52,91,108});
            fb_.fill_rect({p.x-12,p.y-31,24,4},{224,190,123});
        } else if(tile.housing_level==2) {
            draw_block(p,tw*9/10,th*9/10,31,{229,201,144},{157,101,65},{185,119,67});
            fb_.fill_rect({p.x-5,p.y-23,10,16},{63,44,32});
            fb_.fill_rect({p.x-19,p.y-29,6,7},{52,94,113});
            fb_.fill_rect({p.x+13,p.y-29,6,7},{52,94,113});
            fb_.fill_rect({p.x-16,p.y-38,32,5},{237,211,157});
            fb_.fill_rect({p.x-2,p.y-43,4,5},{70,116,67});
        } else {
            draw_block(p,tw,th,37,{237,215,170},{170,111,72},{199,132,76});
            fb_.fill_rect({p.x-6,p.y-27,12,19},{57,42,32});
            fb_.fill_rect({p.x-21,p.y-34,7,8},{55,99,120});
            fb_.fill_rect({p.x+14,p.y-34,7,8},{55,99,120});
            fb_.fill_rect({p.x-20,p.y-45,40,6},{244,225,185});
            fb_.fill_rect({p.x-3,p.y-53,6,8},{54,112,68});
            fb_.line(p.x-17,p.y-41,p.x-17,p.y-48,{118,82,49});
            fb_.line(p.x+17,p.y-41,p.x+17,p.y-48,{118,82,49});
        }
    }

    void draw_merged_residence(int x,int y,int tw,int th) {
        const IsoPoint a=cam_.project(x,y);
        const IsoPoint b=cam_.project(x+1,y+1);
        IsoPoint center{(a.x+b.x)/2,(a.y+b.y)/2+th/2};
        const Tile& residence=world_.tile(x,y);
        const bool courtyard=residence.housing_level>=3;
        const Color top=courtyard?Color{242,220,174}:Color{235,209,156};
        const Color left=courtyard?Color{171,108,65}:Color{160,100,62};
        const Color right=courtyard?Color{204,134,76}:Color{192,122,68};
        draw_block(center,tw*17/10,th*17/10,courtyard?48:42,top,left,right);
        fb_.fill_rect({center.x-8,center.y-31,16,22},{63,44,32});
        fb_.fill_rect({center.x-30,center.y-38,8,8},{54,101,119});
        fb_.fill_rect({center.x+22,center.y-38,8,8},{54,101,119});
        fb_.fill_rect({center.x-29,center.y-52,58,6},{239,216,167});
        if(courtyard){
            fb_.diamond({center.x,center.y-12},tw/3,th/3,{56,126,148},{233,211,155});
            fb_.fill_rect({center.x-3,center.y-65,6,13},{48,108,65});
            fb_.line(center.x-22,center.y-49,center.x-22,center.y-60,{113,77,45});
            fb_.line(center.x+22,center.y-49,center.x+22,center.y-60,{113,77,45});
        }else{
            fb_.fill_rect({center.x-3,center.y-61,6,9},{48,108,65});
        }
    }

    void draw_farm(IsoPoint p,int tw,int th) {
        fb_.diamond(p,tw*9/10,th*9/10,{103,128,61},{103,128,61});
        for(int i=-4;i<=4;++i){const int ox=i*std::max(2,tw/14);fb_.line(p.x+ox-tw/5,p.y+th/6,p.x+ox+tw/5,p.y-th/6,{61,86,39});}
        fb_.fill_rect({p.x-4,p.y-15,8,14},{148,111,62});
        fb_.line(p.x-5,p.y-15,p.x,p.y-21,{113,74,43});
        fb_.line(p.x,p.y-21,p.x+5,p.y-15,{113,74,43});
    }

    void draw_granary(const Tile& tile,IsoPoint p,int tw,int th) {
        draw_block(p,tw*4/5,th*4/5,31,{216,184,117},{128,82,53},{157,99,58});
        fb_.fill_rect({p.x-15,p.y-37,30,5},{240,208,143});
        fb_.fill_rect({p.x-5,p.y-23,10,15},{75,53,37});
        const int sacks=std::min<int>(5,(tile.food_stock+19)/20);
        for(int i=0;i<sacks;++i)fb_.fill_rect({p.x-18+i*8,p.y-45,6,5},{222,177,72});
        fb_.line(p.x-20,p.y-34,p.x,p.y-46,{145,91,51});
        fb_.line(p.x,p.y-46,p.x+20,p.y-34,{145,91,51});
    }

    void draw_market(const Tile& tile,IsoPoint p,int tw,int th) {
        fb_.diamond({p.x+3,p.y+3},tw*4/5,th/2,{91,68,47},{91,68,47});
        fb_.diamond(p,tw*4/5,th*4/5,{156,112,68},{156,112,68});
        const int y=p.y-25;fb_.line(p.x-16,p.y-3,p.x-16,y,{79,55,41});fb_.line(p.x+16,p.y-3,p.x+16,y,{79,55,41});
        fb_.fill_rect({p.x-21,y-5,42,7},{157,52,42});fb_.fill_rect({p.x-21,y+2,42,4},{221,185,102});
        for(int i=0;i<std::min<int>(5,tile.food_stock/4+1);++i)fb_.fill_rect({p.x-14+i*7,p.y-8,4,4},{199,158,55});
        if(tile.pottery_stock>0)fb_.fill_rect({p.x+12,p.y-12,7,8},{168,81,59});
    }

    void draw_well(IsoPoint p,int tw,int th) {
        fb_.diamond({p.x+3,p.y+3},tw*2/3,th/3,{91,70,49},{91,70,49});
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
        fb_.line(p.x+15,p.y-28,p.x+21,p.y-38,{72,55,38});
        if(tile.food_stock>0) fb_.fill_rect({p.x+14,p.y-13,8,6},{163,49,38});
    }
