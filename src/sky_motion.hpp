#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>
namespace sky_motion {
struct Offset{int x,y;};
inline Offset orbit(unsigned seconds){static const Offset p[]={{0,0},{1,0},{2,1},{1,2},{0,2},{-1,1},{-2,0},{-1,-1},{0,-2},{1,-1}};return p[(seconds/45)%10];}
inline Offset fit(Offset p,int left,int top,int right,int bottom,int width,int height){if(right<=left||bottom<=top)return {0,0};p.x=std::max(-left,std::min(p.x,width-right));p.y=std::max(-top,std::min(p.y,height-bottom));return p;}
struct Position{double x,y;};
inline Position celestial(double seconds,double width){double a=seconds/360.;return {width*.73+std::sin(a)*width*.065,83.-std::cos(a)*12.};}
inline uint32_t sun(double x,double y){double r=std::sqrt(x*x+y*y);if(r>=1)return 0;double disc=.43;double edge=std::max(0.,std::min(1.,(disc-r)*100));double glow=std::pow(std::max(0.,1-r),3)*.24;unsigned a=(unsigned)(255*std::max(edge,glow));return (a<<24)|0xffe69e;}
inline uint32_t moon(double x,double y,double phase){double r2=x*x+y*y;if(r2>=1)return 0;double z=std::sqrt(1-r2),a=phase*6.283185307179586;double lit=x*std::sin(a)-z*std::cos(a);double intensity=lit>0?.48+.5*std::sqrt(lit):.065;
 const double craters[][3]={{-.32,-.24,.17},{.25,.33,.22},{.51,-.38,.12},{-.47,.4,.1},{.05,-.6,.09}};
 for(auto&c:craters){double dx=x-c[0],dy=y-c[1],q=(dx*dx+dy*dy)/(c[2]*c[2]);if(q<1)intensity*=.76+.2*q;}
 intensity*=.8+.2*z;unsigned v=(unsigned)(230*std::max(0.,std::min(1.,intensity)));unsigned alpha=(unsigned)(255*std::min(1.,(1-r2)*70));return (alpha<<24)|(v<<16)|((v+std::min(8u,255-v))<<8)|(v+std::min(12u,255-v));}
}
