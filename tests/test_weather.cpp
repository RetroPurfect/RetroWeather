#include "../src/weather.hpp"
#include <cassert>
#include <iostream>
int main(){
 assert(validZip("01234"));assert(!validZip("1234"));assert(!validZip("12x45"));assert(!validZip("123456"));assert(!validZip("12\"45"));
 assert(conditions(0)=="Clear");assert(conditions(95)=="Thunderstorms");assert(conditions(-1)=="Unavailable");
 json j={{"current",{{"temperature_2m",42.6}}},{"daily",{{"time",{"2026-11-13","2026-11-14","2026-11-15","2026-11-16","2026-11-17"}},{"weather_code",{0,1,2,3,95}},{"temperature_2m_max",{45,46,47,48,49}},{"temperature_2m_min",{30,31,32,33,34}},{"precipitation_probability_max",{10,20,30,40,50}}}}};
 validateWeather(j);assert(number(j["current"],"temperature_2m"," F")=="43 F");assert(number(j["current"],"missing")=="--");
 j["current"]["temperature_2m"]=nullptr;bool rejected=false;try{validateWeather(j);}catch(...){rejected=true;}assert(rejected);
 j["current"]["temperature_2m"]=42;j["daily"]["time"]={"one"};rejected=false;try{validateWeather(j);}catch(...){rejected=true;}assert(rejected);
 std::cout<<"PASS: ZIP validation, leading zero, weather codes, rounding, missing fields, invalid current data, short forecast.\n";
}
