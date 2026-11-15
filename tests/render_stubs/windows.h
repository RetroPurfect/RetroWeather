#pragma once
#include <map>
#include <cassert>
#include <vector>
#include <cstdint>
using HDC=void*;using HBITMAP=void*;using HGDIOBJ=void*;
#define TRANSPARENT 1
namespace fakegdi{static int dcCount=0,bitmapCount=0,dcCreates=0,bitmapCreates=0,fail=0,next=10;static std::map<void*,void*> selected;}
inline HDC CreateCompatibleDC(HDC){if(fakegdi::fail==1)return 0;void*p=(void*)(long long)++fakegdi::next;fakegdi::selected[p]=(void*)1;++fakegdi::dcCount;++fakegdi::dcCreates;return p;}
inline HBITMAP CreateCompatibleBitmap(HDC,int w,int h){assert(w>0&&h>0);if(fakegdi::fail==2)return 0;++fakegdi::bitmapCount;++fakegdi::bitmapCreates;return (void*)(long long)++fakegdi::next;}
inline HGDIOBJ SelectObject(HDC dc,HGDIOBJ o){assert(fakegdi::selected.count(dc));void*old=fakegdi::selected[dc];fakegdi::selected[dc]=o;return old;}
inline void DeleteObject(HGDIOBJ o){for(auto it:fakegdi::selected)assert(it.second!=o);--fakegdi::bitmapCount;}
inline void DeleteDC(HDC dc){assert(fakegdi::selected[dc]==(void*)1);fakegdi::selected.erase(dc);--fakegdi::dcCount;}
inline void SetBkMode(HDC,int){}

#define BI_RGB 0
#define DIB_RGB_COLORS 0
using DWORD=unsigned long;
struct BITMAPINFOHEADER{unsigned biSize;int biWidth,biHeight;unsigned short biPlanes,biBitCount;unsigned biCompression;};
struct BITMAPINFO{BITMAPINFOHEADER bmiHeader;};
inline HBITMAP CreateDIBSection(HDC dc,const BITMAPINFO*info,unsigned,void**bits,void*,unsigned){assert(info->bmiHeader.biBitCount==32&&info->bmiHeader.biHeight<0);HBITMAP image=CreateCompatibleBitmap(dc,info->bmiHeader.biWidth,-info->bmiHeader.biHeight);static std::vector<uint32_t> storage(1920*1080);*bits=image?storage.data():0;return image;}
