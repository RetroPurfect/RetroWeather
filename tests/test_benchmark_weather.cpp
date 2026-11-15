#include "../src/benchmark_weather.hpp"
#include <cassert>
#include <set>
#include <iostream>
int main(){for(int i=0;i<9;++i){auto d=benchmark_weather::sample(i,1794690000);validateWeather(d);assert(d["current"]["weather_code"]==benchmark_weather::code(i));assert(d["daily"]["time"].size()==7);}for(bool drizzle:{false,true})for(double time:{0.,1.,1000.}){std::set<int>cells;for(unsigned i=0;i<180;++i){auto d=benchmark_weather::rain(i,time,drizzle);assert(d.x>=-40&&d.x<680&&d.y>=-20&&d.y<500&&d.depth>=.45&&d.depth<1.15);cells.insert(int((d.x+40)/120)+6*int((d.y+20)/104));}assert(cells.size()>=27);}std::cout<<"PASS: all nine validated sample datasets and decorrelated rain distribution.\n";}
