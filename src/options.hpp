#pragma once
#include "weather.hpp"
#include <array>
namespace options {
inline double temperature(double f,bool celsius){return celsius?(f-32.)*5./9.:f;}
inline double wind(double mph,int unit){static const double factors[]={1.,1.609344,.44704,.8689762419};return mph*factors[unit>=0&&unit<4?unit:0];}
inline const char* windLabel(int unit){static const char*labels[]={"mph","km/h","m/s","knots"};return labels[unit>=0&&unit<4?unit:0];}
inline const char* temperatureLabel(bool c){return c?"\xC2\xB0" "C":"\xC2\xB0" "F";}
inline std::string rounded(double n){return std::isfinite(n)?std::to_string((int)std::round(n)):"--";}
template<size_t N> inline int pageSeconds(int page,int fallback,const std::array<int,N>&values){return page>=0&&page<(int)N&&values[page]>0?values[page]:fallback;}
inline double cpuPercent(uint64_t ticks100ns,uint32_t elapsedMs,unsigned processors){return elapsedMs?ticks100ns/10000./elapsedMs*100./std::max(1u,processors):0.;}
enum Icon {Unknown,Sun,PartCloud,Cloud,Fog,Rain,Snow,Storm};
inline Icon icon(int code){if(code==0)return Sun;if(code==1||code==2)return PartCloud;if(code==3)return Cloud;if(code==45||code==48)return Fog;if(code>=51&&code<=67)return Rain;if((code>=71&&code<=77)||code==85||code==86)return Snow;if(code>=80&&code<=82)return Rain;if(code>=95&&code<=99)return Storm;return Unknown;}
struct Rect{int x,y,width,height;};
inline Rect fitWindow(Rect desired,Rect work){desired.width=std::max(1,std::min(desired.width,work.width));desired.height=std::max(1,std::min(desired.height,work.height));desired.x=std::max(work.x,std::min(desired.x,work.x+work.width-desired.width));desired.y=std::max(work.y,std::min(desired.y,work.y+work.height-desired.height));return desired;}
}
