#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
namespace weather_graphics {
inline double noise(double x,double y,unsigned seed){int ix=(int)std::floor(x),iy=(int)std::floor(y);double fx=x-ix,fy=y-iy;fx=fx*fx*(3-2*fx);fy=fy*fy*(3-2*fy);auto value=[&](int a,int b){uint32_t n=uint32_t(a)*374761393u+uint32_t(b)*668265263u+seed*2246822519u;n=(n^(n>>13))*1274126177u;n^=n>>16;return (n&65535)/65535.;};return (value(ix,iy)*(1-fx)+value(ix+1,iy)*fx)*(1-fy)+(value(ix,iy+1)*(1-fx)+value(ix+1,iy+1)*fx)*fy;}
struct CloudSample{unsigned alpha,light;};
struct CloudMask {
 int width,height;std::vector<unsigned char> opacity,lighting;
 CloudMask(int w,int h,int variant):width(w),height(h),opacity(w*h),lighting(w*h){
 const double lobes[][4]={{.14,.62,.19,.19},{.30,.48,.23,.29},{.49,.35,.20,.31},{.65,.48,.25,.28},{.84,.61,.18,.20},{.51,.67,.38,.20}};
 for(int y=0;y<h;++y)for(int x=0;x<w;++x){double u=(x+.5)/w,v=(y+.5)/h,a=0;double grain=noise(u*13,v*10,17+variant)*.48+noise(u*27,v*22,31+variant)*.29+noise(u*59,v*47,71+variant)*.15+noise(u*107,v*81,113+variant)*.08;
 for(int k=0;k<6;++k){auto&l=lobes[k];double dx=(u-l[0]-.025*std::sin(variant*2+k))/l[2],dy=(v-l[1]-.035*std::cos(variant+k*2))/l[3];double edge=std::max(0.,std::min(1.,(1.10-std::sqrt(dx*dx+dy*dy)+(grain-.5)*.16)/.36));edge=edge*edge*(3-2*edge);a=1-(1-a)*(1-edge*.85);}
 double fringe=std::min(1.,std::min(u,1-u)*20)*std::min(1.,std::min(v,1-v)*14);
 double texture=grain;
 size_t at=(size_t)y*w+x;opacity[at]=(unsigned char)(std::max(0.,std::min(1.,a*fringe*(.85+.15*texture)))*218);
 lighting[at]=(unsigned char)(std::max(0.,std::min(1.,.82-v*.45+texture*.22))*255);
 }}
 unsigned sample(const std::vector<unsigned char>&pixels,unsigned x,unsigned y)const{
 // Fixed-point bilinear filtering removes the old nearest-neighbor cloud blocks.
 unsigned ix=std::min((unsigned)width-1,x>>8),iy=std::min((unsigned)height-1,y>>8),nx=std::min((unsigned)width-1,ix+1),ny=std::min((unsigned)height-1,iy+1),fx=x&255,fy=y&255;
 unsigned a=pixels[iy*width+ix]*(256-fx)+pixels[iy*width+nx]*fx,b=pixels[ny*width+ix]*(256-fx)+pixels[ny*width+nx]*fx;return (a*(256-fy)+b*fy)>>16;
 }
 CloudSample at(unsigned x,unsigned y)const{return {sample(opacity,x,y),sample(lighting,x,y)};}
};
inline uint32_t blend(uint32_t dst,uint32_t src,unsigned a){return ((((dst&0xff00ff)*(256-a)+(src&0xff00ff)*a)>>8)&0xff00ff)|((((dst&0xff00)*(256-a)+(src&0xff00)*a)>>8)&0xff00);}
}
