#include "../src/raster_cache.hpp"
#include "../src/cloud_sprite.hpp"
#include "../src/frame_budget.hpp"
#include <cassert>
#include <iostream>
#include <chrono>
int main(){using namespace rendering;
for(int cost:{0,1,2,4,8,16,50,500})for(bool legacy:{false,true}){int delay=budgetDelay(cost,legacy);assert(cost+delay>=(legacy?200:67));assert(!cost||cost/double(cost+delay)<=.100001);}
FrameMeter meter;for(unsigned t=100;t<=2200;t+=100)meter.frame(t);assert(meter.ready&&meter.fps==10);meter.reset();assert(!meter.ready);for(unsigned i=0;i<22;++i)meter.frame(0xffffff00u+i*100);assert(meter.ready&&meter.fps==10);
const int w=17,h=11;std::vector<uint32_t> black(w*h),white(w*h);for(int y=0;y<h;++y)for(int x=0;x<w;++x){unsigned alpha=(x+y)%3?255:((x+y)*23)%256;uint32_t rgb=(x*13<<16)|(y*21<<8)|123;uint32_t p=premultiply(rgb,alpha);black[y*w+x]=over(0,p);white[y*w+x]=over(0xffffff,p);}
RasterCache cached;cached.fromBackgrounds(black.data(),white.data(),w,h);std::vector<uint32_t>a(w*h,0xffffff);cached.composite(a.data(),w,h);assert(a==white);
std::vector<uint32_t>full(w*h,0x345678),partial=full;cached.composite(full.data(),w,h);cached.composite(partial.data(),w,h,0,0,{0,0,8,h});cached.composite(partial.data(),w,h,0,0,{8,0,w,h});assert(full==partial);
// Transparent caches must not darken their destination or grow hidden rectangles.
RasterCache transparent;transparent.build(w,h,[](int,int){return 0u;});assert(transparent.pixels.empty()&&transparent.runs.empty());auto before=partial;transparent.composite(partial.data(),w,h,-200,200);assert(partial==before);
weather_graphics::CloudMask mask(128,64,2);weather_graphics::CloudSprite cloud;cloud.prepare(mask,160,80,1,2,0x203044,0xaabbcc,70);auto original=cloud.raster.pixels;cloud.prepare(mask,160,80,1,2,0x203044,0xaabbcc,70);assert(original==cloud.raster.pixels);
std::vector<uint32_t> target(640*480,0x243449);cloud.raster.composite(target.data(),640,480,-150,-70);cloud.raster.composite(target.data(),640,480,630,470);
std::cout<<"PASS: conservative render budget and frame cap, measured FPS/tick wrap, dual-background foreground extraction, disjoint partial composition, empty caches and clipped cached cloud sprites.\n";
}
