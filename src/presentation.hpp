#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <array>
namespace presentation {
struct Viewport {
 int x=0,y=0,width=0,height=0;
 bool map(int px,int py,int& cx,int& cy)const {
  if(width<=0||height<=0||px<x||py<y||px>=x+width||py>=y+height)return false;
  cx=(int)((int64_t)(px-x)*640/width);cy=(int)((int64_t)(py-y)*480/height);return true;
 }
};
inline Viewport fit(int clientWidth,int clientHeight,int top){
 Viewport v;int available=clientHeight-top;if(clientWidth<=0||available<=0)return v;
 v.width=clientWidth;v.height=(int)((int64_t)v.width*3/4);
 if(v.height>available){v.height=available;v.width=(int)((int64_t)v.height*4/3);}
 v.x=(clientWidth-v.width)/2;v.y=top+(available-v.height)/2;return v;
}
inline Viewport safeFit(int width,int height,int top,int margin){
 Viewport outer=fit(width,height,top);margin=margin<0?0:margin>10?10:margin;
 int insetX=outer.width*margin/100,insetY=outer.height*margin/100;
 Viewport inner=fit(outer.width-insetX*2,outer.height-insetY*2,0);
 inner.x+=outer.x+insetX;inner.y+=outer.y+insetY;return inner;
}

inline const char* aspectName(int mode){static const char* names[]={"Auto","4:3","16:9","16:10","5:4","21:9"};return names[mode>=0&&mode<6?mode:0];}
inline Viewport aspectFit(int width,int height,int mode,int margin){
 Viewport v;if(width<=0||height<=0)return v;static const int nums[]={0,4,16,16,5,21},dens[]={0,3,9,10,4,9};if(mode<0||mode>5)mode=0;
 v.width=width;v.height=height;if(mode){v.height=(int)((int64_t)width*dens[mode]/nums[mode]);if(v.height>height){v.height=height;v.width=(int)((int64_t)height*nums[mode]/dens[mode]);}}
 v.x=(width-v.width)/2;v.y=(height-v.height)/2;margin=margin<0?0:margin>10?10:margin;int ix=v.width*margin/100,iy=v.height*margin/100;v.x+=ix;v.y+=iy;v.width-=ix*2;v.height-=iy*2;return v;
}
struct Frame {
 int width=640,height=480;
 int bodyOffset()const{return (height-480)/2;}
 bool wide()const{return width>=760;}
 bool map(const Viewport&v,int px,int py,int&x,int&y)const{if(v.width<=0||v.height<=0||px<v.x||py<v.y||px>=v.x+v.width||py>=v.y+v.height)return false;x=(int)((int64_t)(px-v.x)*width/v.width);y=(int)((int64_t)(py-v.y)*height/v.height);return true;}
};
inline std::array<int,5> forecastColumns(int width){return {{36,width*27/100,width*52/100,width*69/100,width-32}};}
inline Frame frameFor(const Viewport&v){Frame f;if(v.width<=0||v.height<=0)return f;if((int64_t)v.width*3>=int64_t(v.height)*4)f.width=(int)((int64_t)v.width*480/v.height);else f.height=(int)((int64_t)v.height*640/v.width);return f;}
struct CursorIdle {
 uint32_t last=0;bool hidden=false;
 void reset(uint32_t now){last=now;hidden=false;}
 void update(uint32_t now,bool moved,bool eligible){if(moved||!eligible)reset(now);else hidden=(uint32_t)(now-last)>=12000u;}
};
inline std::string clockText(int hour,int minute,bool format24){if(!format24){hour%=12;if(hour==0)hour=12;}char result[12];std::sprintf(result,"%02d:%02d",hour,minute);return result;}
}
