#pragma once
#include <cmath>
#include <cstddef>
#include "software_blend.hpp"
namespace precipitation {
// Rain streaks always travel downwards. Clip rows before touching the DIB;
// fractional horizontal coverage softens edges without GDI pens or allocations.
inline void streak(uint32_t*dst,int width,int height,double x1,double y1,double x2,double y2,uint32_t color,double thickness){
 if(!dst||width<=0||height<=0||!std::isfinite(x1)||!std::isfinite(x2)||!std::isfinite(y1)||!std::isfinite(y2)||y2<=y1||!std::isfinite(thickness)||thickness<=0)return;
 if(y2<=0||y1>=height)return;
 double slope=(x2-x1)/(y2-y1),radius=std::min(3.,std::max(.5,thickness*.5));
 if(!std::isfinite(slope))return;
 int first=(int)std::max(0.,std::floor(y1)),last=(int)std::min(double(height),std::ceil(y2));
 for(int y=first;y<last;++y){double x=x1+(y+.5-y1)*slope;if(!std::isfinite(x))continue;if(x+radius+1<0||x-radius-1>=width)continue;
  double left=std::max(0.,std::floor(x-radius)),right=std::min(double(width-1),std::ceil(x+radius));
  for(int xx=(int)left;xx<=right;++xx){double coverage=std::max(0.,std::min(1.,radius+.5-std::abs(xx+.5-x)));unsigned alpha=(unsigned)(coverage*256);size_t at=(size_t)y*width+xx;dst[at]=rendering::blendPixel(dst[at],color,alpha);}
 }
}
}
