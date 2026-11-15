#include "../src/presentation.hpp"
#include "../src/features.hpp"
#include "../src/options.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace options;
 assert(std::abs(temperature(32,true))<1e-9);assert(temperature(-40,true)==-40);assert(temperature(212,true)==100);assert(temperature(32,false)==32);
 assert(std::abs(wind(60,1)-96.56064)<1e-8);assert(std::abs(wind(60,2)-26.8224)<1e-8);assert(std::abs(wind(60,3)-52.138574514)<1e-8);assert(wind(60,0)==60);assert(rounded(-.6)=="-1");
 assert(icon(0)==Sun&&icon(1)==PartCloud&&icon(3)==Cloud&&icon(45)==Fog&&icon(61)==Rain&&icon(73)==Snow&&icon(85)==Snow&&icon(95)==Storm&&icon(-1)==Unknown);
 std::array<int,7> durations={{0,0,0,0,0,60,0}};assert(pageSeconds(5,12,durations)==60&&pageSeconds(0,12,durations)==12&&pageSeconds(7,12,durations)==12);
 features::Cycle cycle;cycle.page=5;for(int i=0;i<59;++i)cycle.tick(pageSeconds(cycle.page,12,durations),0);assert(cycle.page==5);cycle.tick(pageSeconds(cycle.page,12,durations),0);assert(cycle.page==4);
 for(Rect work:std::vector<Rect>{{0,0,640,450},{0,0,1920,1040},{-1920,0,1920,1080},{0,0,3840,2100}})for(Rect request:std::vector<Rect>{{-9000,-2000,1000,800},{4000,3000,8000,5000},{100,80,640,480}}){Rect r=fitWindow(request,work);assert(r.x>=work.x&&r.y>=work.y&&r.x+r.width<=work.x+work.width&&r.y+r.height<=work.y+work.height);}
 assert(std::abs(cpuPercent(20000000,10000,1)-20)<1e-9);assert(std::abs(cpuPercent(20000000,10000,8)-2.5)<1e-9);assert(cpuPercent(100,0,1)==0);
 std::cout<<"PASS: offline unit conversions, icon classification, per-page durations, disconnected-monitor window clamping, CPU sample arithmetic and viewport geometry.\n";
}
