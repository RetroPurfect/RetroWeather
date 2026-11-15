#pragma once
#include <windows.h>
#include <cstdint>
#include <algorithm>
struct BackBufferStats{uint64_t allocations=0,liveBytes=0,peakBytes=0;unsigned live=0;};
inline BackBufferStats&backBufferStats(){static BackBufferStats stats;return stats;}
struct BackBuffer{
 HDC dc=0;HBITMAP bitmap=0;HGDIOBJ previous=0;uint32_t*pixels=0;int width=0,height=0;
 BackBuffer()=default;BackBuffer(const BackBuffer&)=delete;BackBuffer&operator=(const BackBuffer&)=delete;
 bool ensure(HDC reference,int w,int h){if(w<=0||h<=0)return false;if(dc&&width==w&&height==h)return true;HDC next=CreateCompatibleDC(reference);BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;void*bits=0;HBITMAP image=CreateDIBSection(reference,&info,DIB_RGB_COLORS,&bits,0,0);if(!next||!image||!bits){if(next)DeleteDC(next);if(image)DeleteObject(image);return false;}auto&stats=backBufferStats();++stats.allocations;++stats.live;stats.liveBytes+=(uint64_t)w*h*4;stats.peakBytes=std::max(stats.peakBytes,stats.liveBytes);clear();dc=next;bitmap=image;pixels=(uint32_t*)bits;previous=SelectObject(dc,bitmap);width=w;height=h;SetBkMode(dc,TRANSPARENT);return true;}
 void clear(){if(dc){auto&stats=backBufferStats();--stats.live;stats.liveBytes-=(uint64_t)width*height*4;SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);}dc=0;bitmap=0;previous=0;pixels=0;width=height=0;}
 ~BackBuffer(){clear();}
};
