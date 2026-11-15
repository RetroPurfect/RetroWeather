#include "../src/presentation.hpp"
#include "../src/features.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace features;
 PageConfig p;assert(p.parse("5,3,0,1,2,4,6,7,8",(1<<5)|(1<<3)));assert(p.enabled()==std::vector<int>({5,3}));assert(p.serialize()=="5,3,0,1,2,4,6,7,8");
 Cycle c;c.configure(p);assert(c.page==5);c.tick(2,3);assert(c.page==5);c.tick(2,3);assert(c.page==3);c.paused=true;for(int i=0;i<100;++i)c.tick(2,3);assert(c.page==3&&c.elapsed==0);c.next();assert(c.page==5);c.paused=false;c.tick(1,3);assert(c.page==3);c.tick(1,3);assert(c.page==pausePage);c.tick(1,3);c.tick(1,3);assert(c.page==pausePage);c.tick(1,3);assert(c.page==5);
 PageConfig invalid;auto original=invalid.order;assert(!invalid.parse("0,0,1,2,3,4,5",127));assert(invalid.order==original);assert(invalid.parse("8,7,6,5,4,3,2,1,0",0));assert(invalid.enabled()==std::vector<int>({0}));
 PageConfig legacy;assert(legacy.parse("5,6,0,1,2,3,4",96));assert(legacy.order==std::vector<int>({5,3,0,1,2,4,6,7,8}));assert(legacy.mask==(32|8|448));assert(!legacy.parse("7,0,1,2,3,4,5",127));
 assert(parseZips("01234,1234,99999,01234,abcde")==std::vector<std::string>({"01234","99999"}));assert(joinZips(parseZips("01234,99999"))=="01234,99999");
 assert(solarTime("2026-11-13T00:05",false)=="12:05 AM");assert(solarTime("2026-11-13T18:09",false)=="06:09 PM");assert(solarTime("2026-11-13T18:09",true)=="18:09");assert(solarTime("not a timestamp",true)=="Not reported");assert(solarTime("2026-11-13T25:99",true)=="Not reported");
 for(auto size:std::vector<std::pair<int,int>>{{640,480},{1920,1080},{3840,2160},{1080,1920}})for(int margin:{0,2,5,8}){auto v=presentation::safeFit(size.first,size.second,0,margin);assert(v.x>=0&&v.y>=0&&v.x+v.width<=size.first&&v.y+v.height<=size.second);int x,y;assert(v.map(v.x+v.width/2,v.y+v.height/2,x,y));assert(x>=319&&x<=320&&y>=239&&y<=240);assert(!v.map(v.x-1,v.y,x,y));}
 std::cout<<"PASS: page selection/order/disabled pages/pause cycles, malformed preferences, ZIP persistence, solar times and CRT/1080p/4K safe-margin mapping.\n";
}
