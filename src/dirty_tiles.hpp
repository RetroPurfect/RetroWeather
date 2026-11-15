#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
namespace rendering {
struct Rect{int left,top,right,bottom;};
inline Rect intersect(Rect a,Rect b){return {std::max(a.left,b.left),std::max(a.top,b.top),std::min(a.right,b.right),std::min(a.bottom,b.bottom)};}
inline bool empty(Rect r){return r.left>=r.right||r.top>=r.bottom;}
inline int projectEdge(int n,int out,int in){return (int)(((int64_t)n*out+in-1)/in);}
inline std::vector<Rect> changedTiles(const uint32_t*before,const uint32_t*after,int sw,int sh,int dw,int dh){std::vector<Rect> out;if(!before||!after||sw<=0||sh<=0||dw<=0||dh<=0)return out;const int tile=16;for(int y=0;y<sh;y+=tile){int run=-1;for(int x=0;x<sw;x+=tile){bool changed=false;for(int yy=y;yy<std::min(y+tile,sh)&&!changed;++yy)for(int xx=x;xx<std::min(x+tile,sw);++xx)if((before[yy*sw+xx]&0xffffff)!=(after[yy*sw+xx]&0xffffff)){changed=true;break;}if(changed&&run<0)run=x;if((!changed||x+tile>=sw)&&run>=0){int end=changed?std::min(x+tile,sw):x;out.push_back({projectEdge(run,dw,sw),projectEdge(y,dh,sh),projectEdge(end,dw,sw),projectEdge(std::min(y+tile,sh),dh,sh)});run=-1;}}}return out;}
inline int64_t area(const std::vector<Rect>&rects){int64_t n=0;for(auto r:rects)if(!empty(r))n+=(int64_t)(r.right-r.left)*(r.bottom-r.top);return n;}
inline void restoreNearest(uint32_t*dest,int dw,int dh,const uint32_t*scene,int sw,int sh,const std::vector<Rect>&rects){if(!dest||!scene||dw<=0||dh<=0||sw<=0||sh<=0)return;std::vector<int> xs(dw);for(int x=0;x<dw;++x)xs[x]=(int)((int64_t)x*sw/dw);for(auto r:rects){r=intersect(r,{0,0,dw,dh});for(int y=r.top;y<r.bottom;++y){const uint32_t*row=scene+(int)((int64_t)y*sh/dh)*sw;uint32_t*out=dest+y*dw;for(int x=r.left;x<r.right;++x)out[x]=row[xs[x]];}}}
}
