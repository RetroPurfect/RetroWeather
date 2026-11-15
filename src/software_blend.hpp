#pragma once
#include <algorithm>
#include <cstdint>
namespace rendering {
inline uint32_t blendPixel(uint32_t destination,uint32_t source,unsigned opacity){unsigned inverse=256-opacity;return ((((destination&0x00ff00ff)*inverse+(source&0x00ff00ff)*opacity)>>8)&0x00ff00ff)|((((destination&0x0000ff00)*inverse+(source&0x0000ff00)*opacity)>>8)&0x0000ff00);}
inline void blendFlat(uint32_t*pixels,int width,int height,int left,int top,int right,int bottom,uint32_t source,unsigned opacity){left=std::max(0,left);top=std::max(0,top);right=std::min(width,right);bottom=std::min(height,bottom);opacity=std::min(256u,opacity);if(!pixels||opacity==0||left>=right||top>=bottom)return;for(int y=top;y<bottom;++y){uint32_t*row=pixels+y*width+left;for(int x=left;x<right;++x,++row)*row=opacity==256?source:blendPixel(*row,source,opacity);}}
inline int adaptiveDelay(int level,int renderMs,bool legacy){int base=legacy?(level==1?250:150):(level==1?100:50);int rest=std::max(0,renderMs)*(legacy?4:2);return std::max(base,std::min(5000,rest));}
}
