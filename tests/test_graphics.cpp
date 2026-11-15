#include "../src/cloud_mask.hpp"
#include "../src/graphics_quality.hpp"
#include <cassert>
#include <iostream>
int main(){rendering::Quality q;q.configure(false,16,32768);assert(q.tier==1&&q.limit()==1280);rendering::Quality other;other.configure(false,2,2048);assert(q.tier==other.tier);for(int i=0;i<299;++i)assert(!q.sample(1));assert(q.sample(1)&&q.tier==2);for(int i=0;i<90;++i)assert(!q.sample(30));for(int i=0;i<30;++i)q.sample(30);assert(q.tier==1);for(int i=0;i<120;++i)q.sample(30);assert(q.tier==0);for(int i=0;i<500&&q.tier==0;++i)q.sample(3);assert(q.tier==1);q.configure(true,2,512);for(int i=0;i<1000;++i)q.sample(1);assert(q.tier==0);q.configure(false,2,512);assert(q.ceiling==1);for(int i=0;i<600;++i)q.sample(11);assert(q.tier==1);for(int i=0;i<100;++i){q.sample(25);q.sample(1);}assert(q.tier==1);
weather_graphics::CloudMask cloud(256,128,0);int partial=0;for(auto a:cloud.opacity){assert(a<=218);if(a>0&&a<200)++partial;}assert(partial>10000);
// Bilinear sampling must reproduce all texel centers and halfway interpolation.
for(unsigned y=0;y<128;y+=7)for(unsigned x=0;x<256;x+=11){assert(cloud.at(x*256,y*256).alpha==cloud.opacity[y*256+x]);if(x<255){unsigned expect=(cloud.opacity[y*256+x]+cloud.opacity[y*256+x+1])/2;assert(cloud.at(x*256+128,y*256).alpha==expect);}}
auto corner=cloud.at(0xffffffffu,0xffffffffu);assert(corner.alpha<=218);
weather_graphics::CloudMask another(256,128,1);assert(cloud.opacity!=another.opacity);
std::cout<<"PASS: automatic detail scaling with sustained-cost hysteresis, low-end ceiling, cloud edge filtering and varied cached cloud textures.\n";
}
