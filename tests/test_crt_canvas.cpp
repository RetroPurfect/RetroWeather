#include "../src/crt_profile.hpp"
#include "../src/presentation.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 for(auto size:{std::pair<int,int>{720,480},{640,480},{720,576},{1920,1080},{720,434}}){auto v=presentation::aspectFit(size.first,size.second,crt_profile::aspect(true,4),crt_profile::margin(true,8));assert(v.x==0&&v.y==0&&v.width==size.first&&v.height==size.second);}
 auto normal=presentation::aspectFit(720,480,crt_profile::aspect(false,1),crt_profile::margin(false,2));assert(normal.width==616&&normal.height==462&&normal.x==52);
 for(double sx:{1.,.75,2.})for(double sy:{1.,.6,1.5}){double x=sx,y=sy,ox=0,oy=0;{crt_profile::SafeContent safe(x,y,ox,oy);assert(std::abs(ox-12.8*sx)<1e-10&&std::abs(oy-9.6*sy)<1e-10);assert(std::abs(x-.96*sx)<1e-10&&std::abs(y-.96*sy)<1e-10);assert(ox+640*x<640*sx&&oy+480*y<480*sy);for(int top=0;top<480;top+=128){double pos=200*y+oy-top;assert(std::abs(pos-(200*.96*sy+9.6*sy-top))<1e-10);}assert(std::abs(80*x-76.8*sx)<1e-10);}assert(x==sx&&y==sy&&ox==0&&oy==0);}
 std::cout<<"PASS: CRT fills 720x480 and other complete rasters; normal pillarbox behavior retained; content-only safe transform restores scene coordinates and preserves strip/width mapping.\n";
}
