    void draw_road(int x,int y,IsoPoint p,int tw,int th) {
        const int level=world_.road_level(x,y);
        Color road{139,111,76};
        int scale_num=3,scale_den=4;
        if(level==1){road={162,132,88};scale_num=4;scale_den=5;}
        else if(level>=2){road={188,157,103};scale_num=9;scale_den=10;}
        fb_.diamond(p,tw*scale_num/scale_den,th*scale_num/scale_den,road,road);
        if(level>=1)fb_.line(p.x-tw/5,p.y,p.x+tw/5,p.y,{113,89,61});
        if(level>=2){fb_.pixel(p.x-tw/6,p.y-2,{226,198,145});fb_.pixel(p.x+tw/7,p.y+2,{226,198,145});}
    }

    void draw_flat_structure(const Tile& tile,IsoPoint p,int tw,int th) {
        if(tile.structure==Structure::Empty)return;
        if(tile.structure==Structure::Road){fb_.diamond(p,tw*3/4,th*3/4,{211,190,150},{166,140,102});return;}
        const Color c=flat_structure_color(tile.structure);
        fb_.diamond(p,tw*4/5,th*4/5,c,{244,231,195});
        fb_.fill_rect({p.x-5,p.y-3,10,6},{246,237,212});
    }

    void draw_structure_at(int x,int y,const Tile& tile,IsoPoint p,int tw,int th) {
        if(flat_mode_){draw_flat_structure(tile,p,tw,th);return;}
        switch(tile.structure) {
            case Structure::Empty: break;
            case Structure::Road: draw_road(x,y,p,tw,th); break;
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
                fb_.diamond({p.x,p.y+2},tw*2/3,th/2,{91,54,43},{66,42,33});
                fb_.fill_rect({p.x-7,p.y-5,14,3},{183,113,72});
                if(tile.clay_stock>0)fb_.fill_rect({p.x+8,p.y-7,6,5},{207,128,75});
                break;
            case Structure::Potter:
                draw_block(p,tw*3/4,th*3/4,24,{165,122,79},{111,71,48},{133,82,51});
                fb_.fill_rect({p.x+8,p.y-39,5,18},{72,54,45});
                if(tile.pottery_stock>0)fb_.fill_rect({p.x-15,p.y-13,7,8},{174,83,58});
                break;
        }
    }

    void draw_person(IsoPoint p,Color clothes,Color skin,int offset=0,bool carrying=false) {
        const int x=p.x+offset,y=p.y-8;
        fb_.fill_rect({x-2,y-7,5,5},skin);
        fb_.fill_rect({x-2,y-2,5,8},clothes);
        fb_.pixel(x-3,y+6,{52,39,31});fb_.pixel(x+3,y+6,{52,39,31});
        if(carrying) fb_.fill_rect({x+4,y-1,4,4},{180,55,42});
    }

    Color role_color(WorkerRole role) const {
        switch(role){
            case WorkerRole::Hunter:return {79,101,49};
            case WorkerRole::Farmer:return {74,126,65};
            case WorkerRole::ClayWorker:return {163,91,56};
            case WorkerRole::Potter:return {174,72,52};
            case WorkerRole::GranaryWorker:return {77,106,137};
            case WorkerRole::MarketWorker:return {143,79,126};
        }
        return {186,145,91};
    }

    void draw_cart(const GoodsAgent& g) {
        const IsoPoint p=cam_.project(g.x,g.y);
        Color cargo{198,131,72};
        if(g.resource==Resource::Pottery)cargo={171,79,57};
        else if(g.resource==Resource::Food)cargo={211,178,75};
        fb_.fill_rect({p.x-8,p.y-10,16,7},{102,70,44});
        fb_.fill_rect({p.x-5,p.y-15,10,6},cargo);
        fb_.fill_rect({p.x-7,p.y-2,4,4},{45,38,32});
        fb_.fill_rect({p.x+4,p.y-2,4,4},{45,38,32});
        draw_person({p.x-13,p.y+2},{169,132,83},{171,112,76},0,false);
    }

    void draw_wildlife(const WildlifeAgent& animal) {
        const IsoPoint p=cam_.project(animal.x,animal.y);
        if(animal.kind==WildlifeKind::Gazelle){
            fb_.fill_rect({p.x-6,p.y-9,11,5},{165,125,78});
            fb_.fill_rect({p.x+4,p.y-12,5,4},{181,143,93});
            fb_.line(p.x-4,p.y-4,p.x-5,p.y+2,{74,58,42});
            fb_.line(p.x+2,p.y-4,p.x+3,p.y+2,{74,58,42});
            fb_.line(p.x+6,p.y-12,p.x+5,p.y-16,{79,61,42});
            fb_.line(p.x+8,p.y-12,p.x+9,p.y-16,{79,61,42});
        }
    }

    void draw_agents() {
        for(const auto& animal:world_.wildlife())draw_wildlife(animal);
        for(const auto& g:world_.goods_agents())draw_cart(g);
        for(const auto& a:world_.immigrants()) {
            const IsoPoint p=cam_.project(a.x,a.y);
            for(int i=0;i<a.group_size;++i) draw_person(p,{185,122,76},{166,111,77},(i-1)*5,false);
        }
        for(const auto& w:world_.workers()) {
            const IsoPoint p=cam_.project(w.x,w.y);
            const bool work_clothes=w.state!=WorkerState::CommutingToJob&&w.state!=WorkerState::RestingAtHome;
            const Color clothes=work_clothes?role_color(w.role):Color{186,145,91};
            draw_person(p,clothes,{171,112,76},0,w.payload_food>0);
        }
    }
