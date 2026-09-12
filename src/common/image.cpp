#include "image.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>

namespace egypt::common {
namespace {
constexpr std::array<int,64> kZigZag = {
  0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
  12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
  35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
  58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};
constexpr int kQYBase[64] = {
 16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,
 14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,
 18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,
 49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99
};
constexpr int kQCBase[64] = {
 17,18,24,47,99,99,99,99,18,21,26,66,99,99,99,99,
 24,26,56,99,99,99,99,99,47,66,99,99,99,99,99,99,
 99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
 99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99
};

std::uint16_t le16(const std::vector<std::uint8_t>& b, std::size_t p) {
    return static_cast<std::uint16_t>(b[p]) | static_cast<std::uint16_t>(b[p+1] << 8U);
}
std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t p) {
    return static_cast<std::uint32_t>(b[p]) |
           (static_cast<std::uint32_t>(b[p+1]) << 8U) |
           (static_cast<std::uint32_t>(b[p+2]) << 16U) |
           (static_cast<std::uint32_t>(b[p+3]) << 24U);
}

bool read_svarint(const std::uint8_t*& p, const std::uint8_t* end, int& out) {
    std::uint32_t z=0; unsigned shift=0;
    while (p < end && shift <= 28U) {
        const std::uint8_t byte=*p++;
        z |= static_cast<std::uint32_t>(byte & 0x7fU) << shift;
        if ((byte & 0x80U)==0) {
            const std::int32_t v = static_cast<std::int32_t>((z >> 1U) ^ (0U - (z & 1U)));
            out=v; return true;
        }
        shift += 7U;
    }
    return false;
}

std::array<int,64> quant(const int* base, int quality) {
    quality=std::clamp(quality,1,100);
    const int scale=quality<50 ? 5000/quality : 200-2*quality;
    std::array<int,64> q{};
    for (int i=0;i<64;++i) q[i]=std::clamp((base[i]*scale+50)/100,1,255);
    return q;
}

struct Plane { int w{},h{}; std::vector<float> p; };

bool decode_plane(const std::uint8_t* begin,const std::uint8_t* end,int w,int h,
                  const std::array<int,64>& qt, Plane& plane) {
    const int pw=(w+7)&~7, ph=(h+7)&~7;
    std::vector<float> padded(static_cast<std::size_t>(pw)*ph,128.0f);
    double C[8][8]{};
    constexpr double pi=3.14159265358979323846;
    for(int u=0;u<8;++u){
        const double a=u==0?1.0/std::sqrt(8.0):std::sqrt(2.0/8.0);
        for(int x=0;x<8;++x) C[u][x]=a*std::cos(pi*(2*x+1)*u/16.0);
    }
    const std::uint8_t* ptr=begin;
    int prevdc=0;
    for(int by=0;by<ph;by+=8){
      for(int bx=0;bx<pw;bx+=8){
        int vals[64]{};
        int dcdiff=0;
        if(!read_svarint(ptr,end,dcdiff)) return false;
        prevdc += dcdiff; vals[0]=prevdc;
        int k=1;
        bool eob=false;
        while(ptr<end){
            std::uint8_t op=*ptr++;
            if(op==0xffU){eob=true;break;}
            if(op==0xfeU){k+=63;if(k>64)return false;continue;}
            k += op;
            if(k>=64)return false;
            int v=0;if(!read_svarint(ptr,end,v))return false;
            vals[k++]=v;
        }
        if(!eob) return false;
        double f[8][8]{};
        for(int i=0;i<64;++i){
            const int pos=kZigZag[i];
            f[pos/8][pos%8]=static_cast<double>(vals[i]*qt[pos]);
        }
        double tmp[8][8]{};
        for(int x=0;x<8;++x) for(int v=0;v<8;++v)
            for(int u=0;u<8;++u) tmp[x][v]+=C[u][x]*f[u][v];
        for(int x=0;x<8;++x) for(int y=0;y<8;++y){
            double s=0;for(int v=0;v<8;++v)s+=tmp[x][v]*C[v][y];
            const int px=bx+y,py=by+x;
            padded[static_cast<std::size_t>(py)*pw+px]=static_cast<float>(std::clamp(s+128.0,0.0,255.0));
        }
      }
    }
    if(ptr!=end) return false;
    plane.w=w;plane.h=h;plane.p.resize(static_cast<std::size_t>(w)*h);
    for(int y=0;y<h;++y) std::copy_n(padded.data()+static_cast<std::size_t>(y)*pw,w,plane.p.data()+static_cast<std::size_t>(y)*w);
    return true;
}

float bilinear(const Plane& p,float x,float y){
    x=std::clamp(x,0.0f,static_cast<float>(p.w-1));
    y=std::clamp(y,0.0f,static_cast<float>(p.h-1));
    int x0=static_cast<int>(x),y0=static_cast<int>(y),x1=std::min(x0+1,p.w-1),y1=std::min(y0+1,p.h-1);
    float fx=x-x0,fy=y-y0;
    auto at=[&](int xx,int yy){return p.p[static_cast<std::size_t>(yy)*p.w+xx];};
    float a=at(x0,y0)*(1-fx)+at(x1,y0)*fx;
    float b=at(x0,y1)*(1-fx)+at(x1,y1)*fx;
    return a*(1-fy)+b*fy;
}
}

Image load_e16(const std::string& path){
    std::ifstream in(path,std::ios::binary); if(!in)return{};
    std::vector<std::uint8_t>b((std::istreambuf_iterator<char>(in)),{});
    if(b.size()<22||b[0]!='E'||b[1]!='J'||b[2]!='8'||b[3]!='A')return{};
    int w=le16(b,4),h=le16(b,6),q=b[8];
    if(w<=0||h<=0||w>8192||h>8192)return{};
    std::uint32_t ly=le32(b,10),lcb=le32(b,14),lcr=le32(b,18);
    std::size_t off=22;
    if(off+static_cast<std::size_t>(ly)+lcb+lcr!=b.size())return{};
    auto qy=quant(kQYBase,q),qc=quant(kQCBase,q);
    Plane Y,Cb,Cr; int cw=(w+1)/2,ch=(h+1)/2;
    if(!decode_plane(b.data()+off,b.data()+off+ly,w,h,qy,Y)) return {};
    off += ly;
    if(!decode_plane(b.data()+off,b.data()+off+lcb,cw,ch,qc,Cb)) return {};
    off += lcb;
    if(!decode_plane(b.data()+off,b.data()+off+lcr,cw,ch,qc,Cr)) return {};
    Image img;img.width=w;img.height=h;img.pixels.resize(static_cast<std::size_t>(w)*h);
    for(int y=0;y<h;++y)for(int x=0;x<w;++x){
        float yy=Y.p[static_cast<std::size_t>(y)*w+x];
        float cb=bilinear(Cb,(x+0.5f)*0.5f-0.5f,(y+0.5f)*0.5f-0.5f)-128.0f;
        float cr=bilinear(Cr,(x+0.5f)*0.5f-0.5f,(y+0.5f)*0.5f-0.5f)-128.0f;
        int r=static_cast<int>(std::lround(yy+1.402f*cr));
        int g=static_cast<int>(std::lround(yy-0.344136f*cb-0.714136f*cr));
        int bl=static_cast<int>(std::lround(yy+1.772f*cb));
        img.pixels[static_cast<std::size_t>(y)*w+x]={static_cast<std::uint8_t>(std::clamp(r,0,255)),static_cast<std::uint8_t>(std::clamp(g,0,255)),static_cast<std::uint8_t>(std::clamp(bl,0,255))};
    }
    return img;
}
}
