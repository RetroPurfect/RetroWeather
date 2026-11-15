#include "../src/refinements.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace refinements;assert(forecastTime("2026-11-13T22:45",false)=="Nov 13, 2026 | 10:45 PM");assert(forecastTime("2026-11-13T00:05",false)=="Nov 13, 2026 | 12:05 AM");assert(forecastTime("2026-11-13T22:45",true)=="Nov 13, 2026 | 22:45");assert(forecastTime("Unavailable",false)=="Unavailable");assert(compass(319)=="NW (319 degrees)");assert(compass(360)=="N (0 degrees)");assert(compass(-90)=="W (270 degrees)");assert(retryDelay(1)==30&&retryDelay(2)==60&&retryDelay(3)==120&&retryDelay(20)==300);std::cout<<"PASS: local forecast timestamp 12/24h, compass wrapping, bounded retry backoff.\n";}
