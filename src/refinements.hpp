#pragma once
#include "presentation.hpp"
#include <cstdio>
#include <cmath>
namespace refinements {
inline int retryDelay(int failures){return failures<=1?30:failures==2?60:failures==3?120:300;}
inline std::string forecastTime(const std::string&s,bool clock24){int y,m,d,h,n;if(std::sscanf(s.c_str(),"%d-%d-%dT%d:%d",&y,&m,&d,&h,&n)!=5||m<1||m>12||d<1||d>31||h<0||h>23||n<0||n>59)return s;const char*months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};return std::string(months[m-1])+" "+std::to_string(d)+", "+std::to_string(y)+" | "+presentation::clockText(h,n,clock24)+(clock24?"":h>=12?" PM":" AM");}
inline std::string compass(double degrees){if(!std::isfinite(degrees))return "Unavailable";int d=(int)std::round(std::fmod(std::fmod(degrees,360.)+360.,360.))%360;const char*names[]={"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};return std::string(names[((int)std::floor((d+11.25)/22.5))%16])+" ("+std::to_string(d)+" degrees)";}
}
