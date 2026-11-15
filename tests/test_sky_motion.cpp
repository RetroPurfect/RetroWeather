#include "../src/sky_motion.hpp"
#include <cassert>
#include <set>
#include <iostream>
int main(){std::set<int>positions;for(unsigned t=0;t<450;++t){auto p=sky_motion::orbit(t);assert(std::abs(p.x)<=2&&std::abs(p.y)<=2);positions.insert((p.x+2)*5+p.y+2);}assert(positions.size()==10);auto a=sky_motion::celestial(0,640),b=sky_motion::celestial(60,640);assert(a.x!=b.x&&a.y!=b.y);for(int t=0;t<10000;++t){auto p=sky_motion::celestial(t,640);assert(p.x>400&&p.x<550&&p.y>=71&&p.y<=95);}
assert(sky_motion::sun(1,0)==0);assert((sky_motion::sun(0,0)>>24)==255);assert((sky_motion::sun(.999,0)>>24)==0);assert(sky_motion::moon(1,0,.5)==0);
auto bright=[](uint32_t p){return p&255;};assert(bright(sky_motion::moon(.5,0,.25))>bright(sky_motion::moon(-.5,0,.25)));assert(bright(sky_motion::moon(-.5,0,.75))>bright(sky_motion::moon(.5,0,.75)));assert(bright(sky_motion::moon(0,0,.5))>bright(sky_motion::moon(0,0,0)));
std::cout<<"PASS: bounded periodic display shifts, continuous celestial movement, transparent glow boundary and waxing/waning moon orientation.\n";}
