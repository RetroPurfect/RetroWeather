#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
namespace scenic {
const double pi=3.141592653589793;
struct State{
 double time=0,nextFlash=4,flashStart=-10,flashLength=.6;uint32_t rng=7321;
 double random(double a,double b){rng=rng*1664525u+1013904223u;return a+(b-a)*(rng/4294967296.);}
 void advance(double dt){time+=std::max(0.,std::min(1.,dt));if(time>=nextFlash){flashStart=time;flashLength=random(.45,.75);nextFlash=time+random(12,23);}}
};
}
