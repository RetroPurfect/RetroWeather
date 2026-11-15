#include "../src/memory_audit.hpp"
#include "../src/raster_cache.hpp"
#include <cassert>
#include <iostream>
#include <vector>
int main(){memory_audit::History h;for(unsigned i=0;i<100;++i){memory_audit::Point p;p.elapsed=i;p.page=i%7;p.builds=i+1;h.push(p);}assert(h.count==12);for(unsigned i=0;i<12;++i)assert(h.at(i).elapsed==88+i);assert(memory_audit::stripHeight(1080)==128&&memory_audit::stripHeight(2160)==128&&memory_audit::stripHeight(40)==40);assert(uint64_t(1920)*128*4<1024*1024);
for(int height:{1,127,128,129,480,1080,2160}){int width=71;std::vector<uint32_t>black(width*height),white(width*height);for(int y=0;y<height;++y)for(int x=0;x<width;++x){unsigned a=(x*53+y*27)%256;uint32_t src=rendering::premultiply((x*57%256<<16)|(y*17%256<<8)|53,a);black[y*width+x]=rendering::over(0,src);white[y*width+x]=rendering::over(0xffffff,src);}rendering::RasterCache full,bands;full.fromBackgrounds(black.data(),white.data(),width,height);bands.begin(width,height);int band=memory_audit::stripHeight(height);for(int first=0;first<height;first+=band)bands.appendBackgroundStrip(black.data(),white.data()+first*width,first,std::min(band,height-first));bands.finish();assert(full.pixels==bands.pixels&&full.runs.size()==bands.runs.size());for(size_t i=0;i<full.runs.size();++i){const auto&a=full.runs[i];const auto&b=bands.runs[i];assert(a.x==b.x&&a.y==b.y&&a.count==b.count&&a.offset==b.offset);}std::vector<uint32_t>a(width*height,0x123456),b=a;full.composite(a.data(),width,height);bands.composite(b.data(),width,height);assert(a==b);}
bool rejected=false;rendering::RasterCache bad;bad.begin(10,20);try{bad.appendBackgroundStrip(nullptr,nullptr,1,1);}catch(...){rejected=true;}assert(rejected);
std::cout<<"PASS: bounded 12-event memory history, sub-1MB 1080p strip scratch, full/strip foreground extraction equality through 4K height, boundaries and invalid strip order.\n";
}
