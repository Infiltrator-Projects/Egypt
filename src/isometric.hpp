#pragma once
#include <algorithm>
#include <cmath>

namespace egypt {
struct IsoPoint{int x=0;int y=0;};
struct IsoCamera{
    int origin_x=640;
    int origin_y=150;
    int pan_x=0;
    int pan_y=0;
    int zoom_percent=100;
    [[nodiscard]] int tile_w() const { return std::max(20, 48*zoom_percent/100); }
    [[nodiscard]] int tile_h() const { return std::max(10, 24*zoom_percent/100); }
    [[nodiscard]] IsoPoint project(int tx,int ty,int elevation=0) const {
        const int hw=tile_w()/2, hh=tile_h()/2;
        return {origin_x+pan_x+(tx-ty)*hw, origin_y+pan_y+(tx+ty)*hh-elevation};
    }
    [[nodiscard]] bool pick(int sx,int sy,int& tx,int& ty) const {
        const double hw=tile_w()/2.0, hh=tile_h()/2.0;
        if(hw<=0.0||hh<=0.0) return false;
        const double a=(sx-(origin_x+pan_x))/hw;
        const double b=(sy-(origin_y+pan_y))/hh;
        const double fx=(a+b)*0.5, fy=(b-a)*0.5;
        tx=static_cast<int>(std::floor(fx+0.5));
        ty=static_cast<int>(std::floor(fy+0.5));
        return true;
    }
};
}
