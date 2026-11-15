#include "../src/benchmark_weather.hpp"
#include "../src/sky_motion.hpp"
#include "../src/precipitation.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
int main(){
 for(int background=0;background<5;++background){benchmark_weather::Session session;assert(!session.active);int theme=background;json live={{"timezone","America/Detroit"},{"utc_offset_seconds",-18000},{"current",{{"time","2026-11-14T23:59"}}},{"daily",{{"sunrise",{"2026-11-14T07:22"}},{"sunset",{"2026-11-14T17:06"}}}}};auto before=live.dump();assert(!session.leave(live,theme));for(int i=0;i<9;++i){assert(session.enter(i,live,theme,1794690000));assert(session.active&&session.index==i&&theme==4);assert(session.live.dump()==before);}assert(!session.enter(9,live,theme,0));assert(session.leave(live,theme));assert(!session.active&&session.index==-1&&theme==background&&live.dump()==before);assert(session.live.is_null());}
 for(int width:{640,1920,3840})for(int height:{480,1080,2160})for(unsigned time=0;time<450;time+=45){for(int pad:{0,1,10}){auto d=sky_motion::fit(sky_motion::orbit(time),pad,pad,width-pad,height-pad,width,height);assert(pad+d.x>=0&&pad+d.y>=0&&width-pad+d.x<=width&&height-pad+d.y<=height);assert(std::abs(d.x)<=2&&std::abs(d.y)<=2);}}
 for(int width:{1,7,640,1600})for(int height:{1,19,480}){size_t size=(size_t)width*height;std::vector<uint32_t> pixels(size+32,0x123456);for(double x:{-100.,-.5,0.,6.,500.,1601.})for(double y:{-30.,-.5,0.,10.,490.})precipitation::streak(pixels.data()+16,width,height,x,y,x-5,y+25,0xaccdde,1.7);for(int i=0;i<16;++i)assert(pixels[i]==0x123456&&pixels[size+16+i]==0x123456);}
 std::vector<uint32_t> p(100,0);precipitation::streak(p.data(),10,10,5,1,5,8,0xffffff,1);assert(p[15]!=0&&p[5]==0&&p[95]==0);auto unchanged=p;precipitation::streak(p.data(),10,10,5,8,5,1,0xffffff,1);precipitation::streak(p.data(),10,10,NAN,1,5,8,0xffffff,1);assert(p==unchanged);
 std::vector<uint32_t> scene(1600*824);auto start=std::chrono::steady_clock::now();for(int frame=0;frame<600;++frame)for(unsigned i=0;i<180;++i){auto d=benchmark_weather::rain(i,frame/15.,false);precipitation::streak(scene.data(),1600,824,d.x*2.3,d.y*1.7,(d.x-4*d.depth)*2.3,(d.y+10+d.depth*12)*1.7,0x99bbcc,1.7);}std::cout<<"PASS: preview enter/switch/exit restores live timezone/sun data and background; safe orbit through 4K; clipped DIB rain guards, coverage and invalid input. Rain-only build-host benchmark "<<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/600<<" ms/frame (180 streaks).\n";
}
