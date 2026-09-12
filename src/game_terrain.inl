    static int floor_div(int value,int divisor) {
        if(value>=0)return value/divisor;
        return -(((-value)+divisor-1)/divisor);
    }

    static int positive_mod(int value,int modulus) {
        const int r=value%modulus;
        return r<0?r+modulus:r;
    }

    Terrain presentation_terrain(int x,int y) const {
        if(world_.in_bounds(x,y))return world_.tile(x,y).terrain;

        // Presentation-only continuation of the generated landscape.  This is
        // deliberately NOT simulation state: it exists so the finite logical
        // grid never appears as a giant diamond-shaped board in normal play.
        const int center=34+positive_mod(floor_div(y,5),3)-1+positive_mod(floor_div(y,13),2);
        const int d=x-center;
        Terrain terrain=Terrain::Desert;
        if(d>=-2&&d<=2)terrain=Terrain::Water;
        else if(d>=-7&&d<=7)terrain=Terrain::Floodplain;
        if((d==-3||d==3)&&positive_mod(x*11+y*7,5)==0)terrain=Terrain::Reeds;
        if(d<=-8&&d>=-13&&positive_mod(x*17+y*13,31)<4)terrain=Terrain::Clay;
        return terrain;
    }

    Color terrain_vertex_color(int x,int y,int corner) const {
        std::array<std::array<int,2>,4> cells{};
        switch(corner){
            case 0: cells={{{x,y},{x-1,y},{x,y-1},{x-1,y-1}}}; break; // top
            case 1: cells={{{x,y},{x-1,y},{x,y+1},{x-1,y+1}}}; break; // right
            case 2: cells={{{x,y},{x+1,y},{x,y+1},{x+1,y+1}}}; break; // bottom
            default: cells={{{x,y},{x+1,y},{x,y-1},{x+1,y-1}}}; break; // left
        }
        int r=0,g=0,b=0;
        for(const auto& c:cells){
            const Terrain terrain=presentation_terrain(c[0],c[1]);
            const Color sample=terrain_color(terrain,c[0],c[1]);
            r+=sample.r;g+=sample.g;b+=sample.b;
        }
        return {static_cast<std::uint8_t>(r/4),static_cast<std::uint8_t>(g/4),static_cast<std::uint8_t>(b/4)};
    }

    void draw_ground_tile(int x,int y,Terrain terrain,IsoPoint p,int tw,int th,int water_phase) {
        const Color top=terrain_vertex_color(x,y,0);
        const Color right=terrain_vertex_color(x,y,1);
        const Color bottom=terrain_vertex_color(x,y,2);
        const Color left=terrain_vertex_color(x,y,3);
        fb_.diamond_gradient(p,tw,th,top,right,bottom,left);
        draw_terrain_detail(terrain,x,y,p,tw,th);
        if(terrain==Terrain::Water&&((x+y+water_phase)&3)==0){
            fb_.line(p.x-tw/5,p.y-1,p.x+tw/5,p.y-1,{73,143,163});
        }
        if(terrain==Terrain::Reeds){
            for(int k=-2;k<=2;++k)fb_.line(p.x+k*3,p.y,p.x+k*3+1,p.y-10,{38,83,43});
        }
    }
