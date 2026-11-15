#pragma once
#include "cloud_mask.hpp"
#include "raster_cache.hpp"
namespace weather_graphics {
struct CloudSprite {
 int width=0,height=0,style=-1,variant=-1;rendering::RasterCache raster;
 bool prepare(const CloudMask&mask,int w,int h,int appearance,int v,uint32_t shade,uint32_t lit,unsigned strength){if(width==w&&height==h&&style==appearance&&variant==v)return false;width=w;height=h;style=appearance;variant=v;uint32_t colors[256];for(unsigned j=0;j<256;++j){unsigned rgb=0;for(unsigned shift:{0u,8u,16u}){int a=(shade>>shift)&255,b=(lit>>shift)&255;rgb|=(unsigned)(a+(b-a)*(int)j/255)<<shift;}colors[j]=rgb;}
 raster.build(w,h,[&](int x,int y){auto s=mask.at((unsigned)((uint64_t)x*(mask.width-1)*256/std::max(1,w-1)),(unsigned)((uint64_t)y*(mask.height-1)*256/std::max(1,h-1)));unsigned a=s.alpha*strength/100;if(appearance/10==6){double nx=(x+.5)/std::max(1,w),ny=(y+.5)/std::max(1,h);double u=ny-.5-.05*std::sin(nx*9+v);double edge=std::pow(std::sin(nx*3.141592653589793),2);a=(unsigned)(std::exp(-u*u*18)*edge*strength*1.65);s.light=170;}return rendering::premultiply(colors[s.light],a);});return true;
 }
};
}
