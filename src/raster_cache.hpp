#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include "dirty_tiles.hpp"
#include "blend_span.hpp"
namespace rendering {
// Exact /255 in parallel 16-bit lanes; inputs are valid premultiplied RGB.
inline uint32_t over(uint32_t dst,uint32_t premul){unsigned a=premul>>24;if(!a)return dst;if(a==255)return premul&0xffffff;unsigned inv=255-a;uint32_t rb=(dst&0xff00ff)*inv,g=(dst&0xff00)*inv;rb=((rb+0x10001+((rb>>8)&0xff00ff))>>8)&0xff00ff;g=((g+0x100+((g>>8)&0xff00))>>8)&0xff00;return (premul&0xffffff)+rb+g;}
inline uint32_t premultiply(uint32_t rgb,unsigned a){return (a<<24)|((((rgb>>16)&255)*a/255)<<16)|((((rgb>>8)&255)*a/255)<<8)|((rgb&255)*a/255);}
struct RasterCache {
 struct Run{int y,x,count;size_t offset;};int width=0,height=0;std::vector<Run> runs;std::vector<uint32_t> pixels;
 void clear(){width=height=appendedRows=0;std::vector<Run>().swap(runs);std::vector<uint32_t>().swap(pixels);}
 int appendedRows=0;
 void begin(int w,int h){if(w<0||h<0)throw std::logic_error("Invalid raster extent");if((int64_t)w*h<(int64_t)width*height/2)clear();width=w;height=h;runs.clear();pixels.clear();appendedRows=0;}
 template<class F>void append(int first,int count,F pixel){if(first!=appendedRows||count<0||first>height-count)throw std::logic_error("Invalid raster strip");for(int y=first;y<first+count;++y){int begin=-1;size_t offset=0;for(int x=0;x<=width;++x){uint32_t p=x<width?pixel(x,y):0;if(p>>24){if(begin<0){begin=x;offset=pixels.size();}pixels.push_back(p);}else if(begin>=0){runs.push_back({y,begin,x-begin,offset});begin=-1;}}}appendedRows+=count;}
 void finish(){if(appendedRows!=height)throw std::logic_error("Incomplete raster strips");if(pixels.capacity()>pixels.size()+pixels.size()/4+256)std::vector<uint32_t>(pixels).swap(pixels);if(runs.capacity()>runs.size()+runs.size()/4+32)std::vector<Run>(runs).swap(runs);}
 template<class F>void build(int w,int h,F pixel){begin(w,h);append(0,h,pixel);finish();}
 static uint32_t extract(uint32_t black,uint32_t white){unsigned difference=255;for(unsigned s:{0u,8u,16u}){int d=int((white>>s)&255)-int((black>>s)&255);difference=std::min(difference,(unsigned)std::max(0,d));}unsigned a=255-difference;return a?(a<<24)|(black&0xffffff):0;}
 void appendBackgroundStrip(const uint32_t*black,const uint32_t*white,int first,int count){append(first,count,[&](int x,int y){return extract(black[(size_t)y*width+x],white[(size_t)(y-first)*width+x]);});}
 void fromBackgrounds(const uint32_t*black,const uint32_t*white,int w,int h){begin(w,h);appendBackgroundStrip(black,white,0,h);finish();}
 void composite(uint32_t*dst,int dw,int dh,int ox=0,int oy=0,Rect clip={0,0,2147483647,2147483647},unsigned gain=255,bool vectorAllowed=false)const{
 clip=intersect(clip,{0,0,dw,dh});if(empty(clip))return;
 auto it=std::lower_bound(runs.begin(),runs.end(),clip.top-oy,[](const Run&r,int y){return r.y<y;});
 for(;it!=runs.end()&&it->y+oy<clip.bottom;++it){int left=std::max(clip.left,it->x+ox),right=std::min(clip.right,it->x+ox+it->count);if(left>=right)continue;uint32_t*d=dst+(size_t)(it->y+oy)*dw+left;const uint32_t*s=pixels.data()+it->offset+left-(it->x+ox);if(gain==255){int done=0;
#if RW_BLEND_SSE2
 if(vectorAllowed)done=blendVector(d,s,right-left);
#endif
 d+=done;s+=done;for(int x=left+done;x<right;++x,++d,++s)*d=over(*d,*s);}else{for(int x=left;x<right;++x,++d,++s){uint32_t value=0;for(unsigned shift:{0u,8u,16u,24u})value|=((*s>>shift)&255)*gain/255<<shift;*d=over(*d,value);}}}
 }
 size_t bytes()const{return pixels.capacity()*4+runs.capacity()*sizeof(Run);}
};
}
