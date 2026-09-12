#include "world.hpp"
#include <algorithm>
#include <stdexcept>

namespace egypt {
World::World() : tiles_(static_cast<std::size_t>(kWidth * kHeight)) { generate(); }
std::size_t World::index(int x,int y) const { return static_cast<std::size_t>(y*kWidth+x); }
bool World::in_bounds(int x,int y) const { return x>=0 && y>=0 && x<kWidth && y<kHeight; }
const Tile& World::tile(int x,int y) const { if(!in_bounds(x,y)) throw std::out_of_range("tile"); return tiles_[index(x,y)]; }
Tile& World::tile(int x,int y) { if(!in_bounds(x,y)) throw std::out_of_range("tile"); return tiles_[index(x,y)]; }

void World::generate(){
    for(int y=0;y<kHeight;++y){
        const int center=34 + ((y/5)%3)-1 + ((y/13)%2);
        for(int x=0;x<kWidth;++x){
            Tile& t=tiles_[index(x,y)]; t={};
            const int d=x-center;
            if(d>=-2 && d<=2) t.terrain=Terrain::Water;
            else if(d>=-7 && d<=7) t.terrain=Terrain::Floodplain;
            else t.terrain=Terrain::Desert;
            if((d==-3 || d==3) && ((x*11+y*7)%5==0)) t.terrain=Terrain::Reeds;
            if(d<=-8 && d>=-13 && ((x*17+y*13)%31<4)) t.terrain=Terrain::Clay;
        }
    }
}

int World::cost(Structure s){
    switch(s){case Structure::Road:return 2;case Structure::House:return 10;case Structure::ClayPit:return 35;case Structure::Potter:return 55;default:return 0;}
}
bool World::can_place(Structure s,int x,int y) const{
    if(!in_bounds(x,y) || s==Structure::Empty) return false;
    const Tile& t=tile(x,y); if(t.structure!=Structure::Empty) return false;
    if(s==Structure::Road) return t.terrain!=Terrain::Water && t.terrain!=Terrain::Reeds;
    if(s==Structure::ClayPit) return t.terrain==Terrain::Clay;
    if(s==Structure::House || s==Structure::Potter) return t.terrain==Terrain::Desert || t.terrain==Terrain::Floodplain;
    return false;
}
bool World::place(Structure s,int x,int y){
    if(!can_place(s,x,y)) return false;
    const int c=cost(s); if(treasury_<c) return false;
    tile(x,y).structure=s; treasury_-=c; return true;
}
bool World::bulldoze(int x,int y){
    if(!in_bounds(x,y)) return false;
    Tile& t=tile(x,y);
    if(t.structure==Structure::Empty) return false;
    t.structure=Structure::Empty; t.population=0; return true;
}
bool World::has_road_access(int x,int y) const{
    static constexpr int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
    for(int i=0;i<4;++i){int nx=x+dx[i],ny=y+dy[i]; if(in_bounds(nx,ny)&&tile(nx,ny).structure==Structure::Road)return true;}
    return false;
}
void World::tick(){
    ++ticks_;
    for(int y=0;y<kHeight;++y) for(int x=0;x<kWidth;++x){
        Tile& t=tile(x,y); if(t.structure!=Structure::House) continue;
        if(has_road_access(x,y)){ if(t.population<8) ++t.population; }
        else if(t.population>0) --t.population;
    }
}
int World::population() const { int p=0; for(const Tile& t:tiles_) p+=t.population; return p; }
const char* World::terrain_name(Terrain t){switch(t){case Terrain::Desert:return "DESERT";case Terrain::Floodplain:return "FLOODPLAIN";case Terrain::Water:return "NILE";case Terrain::Clay:return "CLAY";case Terrain::Reeds:return "REEDS";} return "?";}
const char* World::structure_name(Structure s){switch(s){case Structure::Empty:return "EMPTY";case Structure::Road:return "ROAD";case Structure::House:return "HOUSE";case Structure::ClayPit:return "CLAY PIT";case Structure::Potter:return "POTTER";} return "?";}
}
