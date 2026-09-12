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
        const int hw=tw/2, hh=th/2;
        const IsoPoint bt{p.x,p.y-hh},br{p.x+hw,p.y},bb{p.x,p.y+hh},bl{p.x-hw,p.y};
        const IsoPoint tt{bt.x,bt.y-height},tr{br.x,br.y-height},tb{bb.x,bb.y-height},tl{bl.x,bl.y-height};
        fb_.quad(bl,bb,tb,tl,left); fb_.quad(bb,br,tr,tb,right); fb_.quad(tt,tr,tb,tl,top);
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
        } else if(tile.housing_level==2) {
            draw_block(p,tw*9/10,th*9/10,31,{229,201,144},{157,101,65},{185,119,67});
            fb_.fill_rect({p.x-5,p.y-23,10,16},{63,44,32});
            fb_.fill_rect({p.x-19,p.y-29,6,7},{52,94,113});
            fb_.fill_rect({p.x+13,p.y-29,6,7},{52,94,113});
            fb_.fill_rect({p.x-16,p.y-38,32,5},{237,211,157});
        } else {
            draw_block(p,tw,th,37,{237,215,170},{170,111,72},{199,132,76});
            fb_.fill_rect({p.x-6,p.y-27,12,19},{57,42,32});
            fb_.fill_rect({p.x-21,p.y-34,7,8},{55,99,120});
            fb_.fill_rect({p.x+14,p.y-34,7,8},{55,99,120});
            fb_.fill_rect({p.x-20,p.y-45,40,6},{244,225,185});
            fb_.fill_rect({p.x-3,p.y-53,6,8},{54,112,68});
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
        if(tile.pottery_stock>0)fb_.fill_rect({p.x+12,p.y-12,7,8},{168,81,59});
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
