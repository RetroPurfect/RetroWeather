#pragma once
#include "weather.hpp"
#include <algorithm>
#include <string>
namespace crt_profile {
inline int aspect(bool enabled,int normal){return enabled?0:normal;}
inline int margin(bool enabled,int normal){return enabled?0:normal;}
// CRT output fills the entire raster; only cached foreground content is inset.
struct SafeContent {
 double &x,&y,&offsetX,&offsetY;double oldX,oldY,oldOffsetX,oldOffsetY;
 SafeContent(double&sx,double&sy,double&ox,double&oy):x(sx),y(sy),offsetX(ox),offsetY(oy),oldX(sx),oldY(sy),oldOffsetX(ox),oldOffsetY(oy){offsetX+=oldX*640*.02;offsetY+=oldY*480*.02;x*=.96;y*=.96;}
 ~SafeContent(){x=oldX;y=oldY;offsetX=oldOffsetX;offsetY=oldOffsetY;}
 SafeContent(const SafeContent&)=delete;SafeContent&operator=(const SafeContent&)=delete;
};
inline int part(unsigned elapsed,unsigned dwell){return dwell>1&&elapsed>=dwell/2?1:0;}
inline int firstHour(int part){return part?6:0;}
inline std::string shortCondition(int code){if(code==1)return "Mostly clear";if(code==2)return "Partly cloudy";if(code==56||code==57)return "Ice drizzle";if(code==66||code==67)return "Freezing rain";if(code>=95&&code<=99)return "T-storms";return conditions(code);}
}
