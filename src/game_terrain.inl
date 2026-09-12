    Color terrain_vertex_color(int x,int y,int corner) const {
        std::array<std::array<int,2>,4> cells{};
        switch(corner){
            case 0: cells={{{x,y},{x-1,y},{x,y-1},{x-1,y-1}}}; break; // top
            case 1: cells={{{x,y},{x-1,y},{x,y+1},{x-1,y+1}}}; break; // right
            case 2: cells={{{x,y},{x+1,y},{x,y+1},{x+1,y+1}}}; break; // bottom
            default: cells={{{x,y},{x+1,y},{x,y-1},{x+1,y-1}}}; break; // left
        }
        int r=0,g=0,b=0,count=0;
        for(const auto& c:cells){
            if(!world_.in_bounds(c[0],c[1])) continue;
            const Tile& t=world_.tile(c[0],c[1]);
            const Color sample=terrain_color(t.terrain,c[0],c[1]);
            r+=sample.r;g+=sample.g;b+=sample.b;++count;
        }
        if(count==0)return terrain_color(world_.tile(x,y).terrain,x,y);
        return {static_cast<std::uint8_t>(r/count),static_cast<std::uint8_t>(g/count),static_cast<std::uint8_t>(b/count)};
    }

    void draw_ground_tile(int x,int y,const Tile& tile,IsoPoint p,int tw,int th,int water_phase) {
        const Color top=terrain_vertex_color(x,y,0);
        const Color right=terrain_vertex_color(x,y,1);
        const Color bottom=terrain_vertex_color(x,y,2);
        const Color left=terrain_vertex_color(x,y,3);
        fb_.diamond_gradient(p,tw,th,top,right,bottom,left);
        draw_terrain_detail(tile.terrain,x,y,p,tw,th);
        if(tile.terrain==Terrain::Water&&((x+y+water_phase)&3)==0){
            fb_.line(p.x-tw/5,p.y-1,p.x+tw/5,p.y-1,{73,143,163});
        }
        if(tile.terrain==Terrain::Reeds){
            for(int k=-2;k<=2;++k)fb_.line(p.x+k*3,p.y,p.x+k*3+1,p.y-10,{38,83,43});
        }
    }
