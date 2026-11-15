#include "../src/weather_pages.hpp"
#include "../src/benchmark_weather.hpp"
#include <cassert>
#include <iostream>
int main(){
 using namespace weather_pages;json empty=json::object();assert(value(at(empty,"missing",0))=="--");assert(hours(empty,"2026").empty());assert(dayIndex(empty,"2026")==-1);assert(value(json(),"%") == "--");assert(value(25.4," in",2,1/25.4)=="1.00 in");
 json h={{"time",{"2026-11-15T17:00","2026-11-15T18:00","2026-11-15T23:00","2026-11-16T05:00","2026-11-16T06:00"}},{"temperature_2m",{65,60,nullptr,50,48}},{"precipitation_probability",{0,10,80,20,99}},{"weather_code",{0,2,63,3,95}},{"precipitation",{0,1,2,3,99}}};
 auto n=night(h,"2026-11-15T18:00","2026-11-16T06:00");assert(n.found&&n.haveLow&&n.low==50&&n.haveChance&&n.chance==80&&n.weather==63&&n.amount==6);assert(!night(empty,"a","z").found);assert(hours(h,"2026-11-15T18:00",2)==std::vector<int>({1,2}));assert(at(h,"temperature_2m",2).is_null());assert(at(h,"time",-1).is_null());
 for(int scene=0;scene<9;++scene){auto d=benchmark_weather::sample(scene,1794690000);validateWeather(d,7);assert(d["hourly"]["time"].size()==48);assert(hours(d["hourly"],d["hourly"]["time"][0]).size()==12);d["daily"]["time"].erase(d["daily"]["time"].end()-1);bool threw=false;try{validateWeather(d,7);}catch(...){threw=true;}assert(threw);}
 assert(uvLabel(2)==std::string("Low")&&uvLabel(11)==std::string("Extreme"));assert(conditions(66)=="Freezing rain"&&conditions(65)=="Heavy rain");
 std::cout<<"PASS: sparse/missing/null weather values, unit conversion, overnight date boundaries and aggregation, hourly selection, seven-day live validation and richer WMO labels.\n";
}
