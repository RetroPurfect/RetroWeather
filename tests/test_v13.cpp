#include "../src/software_blend.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
int main(){for(unsigned dest:{0u,0xffffffu,0x123456u,0xabcdefu})for(unsigned source:{0u,0xffffffu,0x674223u})for(unsigned alpha:{0u,64u,128u,192u,256u}){auto actual=rendering::blendPixel(dest,source,alpha);for(unsigned shift:{0u,8u,16u}){unsigned expected=(((dest>>shift)&255)*(256-alpha)+((source>>shift)&255)*alpha)/256;assert(((actual>>shift)&255)==expected);}}
std::vector<uint32_t> pixels(16,0xffffff);rendering::blendFlat(pixels.data(),4,4,-5,-3,2,2,0,128);for(int y=0;y<4;++y)for(int x=0;x<4;++x)assert(pixels[y*4+x]==(x<2&&y<2?0x7f7f7fu:0xffffffu));auto before=pixels;rendering::blendFlat(pixels.data(),4,4,10,10,14,14,0,128);assert(pixels==before);rendering::blendFlat(pixels.data(),4,4,0,0,4,4,0,0);assert(pixels==before);
for(bool legacy:{false,true})for(int level:{1,2})for(int duration:{0,10,50,100,500,1000}){int delay=rendering::adaptiveDelay(level,duration,legacy);assert(delay>0);if(duration>0)assert(duration/(double)(duration+delay)<=(legacy?.2:1./3.)+.000001);}
assert(rendering::adaptiveDelay(1,0,true)>=250);assert(rendering::adaptiveDelay(1,500,true)==2000);std::cout<<"PASS: packed RGB blend against per-channel reference; clipped/empty/transparent rectangles; adaptive post-render rest at legacy/standard budgets.\n";
}
