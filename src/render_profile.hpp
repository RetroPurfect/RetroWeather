#pragma once
#include <array>
namespace rendering {
struct RenderProfile {
 unsigned samples=0;std::array<double,4> average={{0,0,0,0}};
 void observe(double scene,double scale,double overlay,double present,bool cold){if(cold)return;std::array<double,4> now={{scene,scale,overlay,present}};for(unsigned i=0;i<4;++i)average[i]=samples?average[i]*.9+now[i]*.1:now[i];if(samples<1000000)++samples;}
 void reset(){samples=0;average={{0,0,0,0}};}
};
}
