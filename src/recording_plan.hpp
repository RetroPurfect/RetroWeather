#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace recording_plan {
static const unsigned fps=10,sceneMs=16000,scenes=9,totalMs=sceneMs*scenes;
struct Size{int width,height;};
static const int resolutionCount=5;
inline int resolutionIndex(int index){return index>=0&&index<resolutionCount?index:2;}
inline Size maximum(int index){const Size sizes[]={{640,360},{854,480},{1280,720},{1920,1080},{3840,2160}};return sizes[resolutionIndex(index)];}
inline const char* label(int index){const char*names[]={"360p / 10 FPS (lightest)","480p / 10 FPS","720p / 10 FPS","1080p / 10 FPS","2160p / 10 FPS (4K)"};return names[resolutionIndex(index)];}
inline Size fit(int w,int h,int index){if(w<1||h<1)return {0,0};auto m=maximum(index);int width=w,height=h;if(w>m.width||h>m.height){if((int64_t)w*m.height>(int64_t)h*m.width){width=m.width;height=(int)((int64_t)h*m.width/w);}else{height=m.height;width=(int)((int64_t)w*m.height/h);}}return {std::max(2,width&~1),std::max(2,height&~1)};}
inline unsigned bitrate(int width,int height){uint64_t pixels=uint64_t(std::max(0,width))*std::max(0,height);return pixels<=640*360?300000:pixels<=854*480?450000:pixels<=1280*720?900000:pixels<=1920*1080?2400000:8000000;}
inline unsigned stage(unsigned ms){return std::min(8u,ms/sceneMs);}
inline unsigned page(unsigned ms){return std::min(8u,(ms%sceneMs)/1600);}
inline void nv12(const uint32_t*rgb,int w,int h,std::vector<unsigned char>&out){out.resize((size_t)w*h*3/2);for(int y=0;y<h;++y)for(int x=0;x<w;++x){uint32_t c=rgb[(size_t)y*w+x];int r=(c>>16)&255,g=(c>>8)&255,b=c&255;out[(size_t)y*w+x]=(unsigned char)(((66*r+129*g+25*b+128)>>8)+16);}for(int y=0;y<h;y+=2)for(int x=0;x<w;x+=2){int r=0,g=0,b=0;for(int dy=0;dy<2;++dy)for(int dx=0;dx<2;++dx){uint32_t c=rgb[(size_t)(y+dy)*w+x+dx];r+=(c>>16)&255;g+=(c>>8)&255;b+=c&255;}r/=4;g/=4;b/=4;size_t at=(size_t)w*h+(y/2)*w+x;out[at]=(unsigned char)(((-38*r-74*g+112*b+128)>>8)+128);out[at+1]=(unsigned char)(((112*r-94*g-18*b+128)>>8)+128);}}
inline int16_t pcm44100(const std::vector<char>&pcm,uint64_t at,int volume){if(pcm.empty())return 0;size_t n=pcm.size()/2,index=(size_t)((at/2)%n);int16_t a=(int16_t)((unsigned char)pcm[index*2]|(unsigned char)pcm[index*2+1]<<8);size_t next=(index+1)%n;int16_t b=(int16_t)((unsigned char)pcm[next*2]|(unsigned char)pcm[next*2+1]<<8);int sample=(at&1)?(int(a)+b)/2:a;return (int16_t)(sample*std::max(0,std::min(100,volume))/100);}
}
