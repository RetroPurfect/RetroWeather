#include "../src/presentation.hpp"
#include <cassert>
#include <iostream>
#include <vector>
int main(){using namespace presentation;
 assert(clockText(0,5,false)=="12:05");assert(clockText(12,5,false)=="12:05");assert(clockText(23,59,false)=="11:59");assert(clockText(0,5,true)=="00:05");assert(clockText(23,59,true)=="23:59");
 for(auto size:std::vector<std::pair<int,int>>{{640,480},{800,600},{1024,768},{1280,720},{1280,1024},{1920,1080},{1080,1920},{3840,2160},{1,1},{0,0}})for(int top:{0,38}){
  Viewport v=fit(size.first,size.second,top);if(v.width<=0||v.height<=0)continue;
  assert(v.x>=0&&v.y>=top&&v.x+v.width<=size.first&&v.y+v.height<=size.second);
  assert(std::abs(v.width*3-v.height*4)<=3);
  int x,y;assert(v.map(v.x,v.y,x,y)&&x==0&&y==0);assert(!v.map(v.x-1,v.y,x,y));assert(!v.map(v.x+v.width,v.y,x,y));
  if(v.width>=640){int px=v.x+v.width*500/640,py=v.y+v.height*450/480;assert(v.map(px,py,x,y));}
 }
 Viewport v=fit(1920,1080,0);assert(v.x==240&&v.y==0&&v.width==1440&&v.height==1080);
 CursorIdle c;c.reset(1000);c.update(12999,false,true);assert(!c.hidden);c.update(13000,false,true);assert(c.hidden);c.update(13001,true,true);assert(!c.hidden);c.update(25001,false,true);assert(c.hidden);c.update(25002,false,false);assert(!c.hidden);
 c.reset(0xfffffff0u);c.update((uint32_t)(0xfffffff0u+11999u),false,true);assert(!c.hidden);c.update((uint32_t)(0xfffffff0u+12000u),false,true);assert(c.hidden);
 std::cout<<"PASS: 12/24h clock, proportional viewport and control mapping at varied resolutions, black-bar rejection, 12-second cursor idle/movement/inactive behavior and tick wraparound.\n";
}
