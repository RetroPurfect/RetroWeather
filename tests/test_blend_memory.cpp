#include "../src/raster_cache.hpp"
#include "../src/graphics_quality.hpp"
#include "../src/cloud_sprite.hpp"
#include <cassert>
#include <iostream>
static uint32_t reference(uint32_t dst,uint32_t p){unsigned a=p>>24;if(!a)return dst;uint32_t r=0;for(unsigned s:{0u,8u,16u})r|=((((dst>>s)&255)*(255-a)/255)+((p>>s)&255))<<s;return r;}
int main(){uint32_t rng=57213;for(unsigned alpha=0;alpha<256;++alpha)for(int i=0;i<4000;++i){rng=rng*1664525u+1013904223u;uint32_t dst=rng&0xffffff;rng=rng*1664525u+1013904223u;uint32_t p=rendering::premultiply(rng&0xffffff,alpha);assert(rendering::over(dst,p)==reference(dst,p));}
rendering::RasterCache r;r.build(1024,512,[](int,int){return 0xffabcdeeu;});size_t before=r.bytes();r.build(256,128,[](int,int){return 0x80102030u;});assert(r.bytes()<before/8);assert(r.bytes()<=r.pixels.size()*5+r.runs.size()*sizeof(rendering::RasterCache::Run)*2+2048);
weather_graphics::CloudMask mask(128,64,2);weather_graphics::CloudSprite cloud;assert(cloud.prepare(mask,640,320,1,2,0x304050,0x90a0b0,60));before=cloud.raster.bytes();assert(!cloud.prepare(mask,640,320,1,2,0x304050,0x90a0b0,60));assert(cloud.prepare(mask,320,160,1,2,0x304050,0x90a0b0,60));assert(cloud.raster.bytes()<before/2);
std::cout<<"PASS: 1,024,000 exact premultiplied blend comparisons; shrinking raster/cloud caches release old capacity; warm sprite hits avoid rebuild.\n";}
