#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace benchmark {
static const int stages=11;static const unsigned settling=5000;
inline int page(int stage,unsigned elapsed){return stage>0&&stage<10?(elapsed/3000)%9:-1;}
inline unsigned duration(int stage){return stage==0?15000:stage==10?20000:30000;}
inline const char* name(int stage){static const char*n[]={"Basic Blue baseline","Clear day","Clear night","Partly cloudy","Overcast","Drizzle","Rain","Thunderstorm","Snow","Fog","Playback controls"};return stage>=0&&stage<stages?n[stage]:"Finished";}
inline unsigned total(){unsigned n=0;for(int i=0;i<stages;++i)n+=duration(i);return n;}
struct Timeline {int stage=0;unsigned started=0;bool done=false;void begin(unsigned now){stage=0;started=now;done=false;}unsigned elapsed(unsigned now)const{return now-started;}bool ready(unsigned now)const{return !done&&elapsed(now)>=duration(stage);}void next(unsigned now){if(++stage>=stages)done=true;started=now;}};
struct Frames {
 unsigned frames=0,transient=0,geometryChanges=0;int width=0,height=0,tier=-1;double elapsedSum=0,maximum=0,transientMax=0;double sums[5]={};std::vector<double> times;
 void add(double ms,const double*stages,bool cold,int w,int h,int quality){if(cold){++transient;transientMax=std::max(transientMax,ms);return;}if(frames&&(width!=w||height!=h||tier!=quality))++geometryChanges;width=w;height=h;tier=quality;++frames;elapsedSum+=ms;maximum=std::max(maximum,ms);for(int i=0;i<5;++i)sums[i]+=stages[i];if(times.size()<4096)times.push_back(ms);}
 double percentile(double p)const{if(times.empty())return 0;auto v=times;std::sort(v.begin(),v.end());size_t rank=(size_t)std::ceil(v.size()*std::max(0.,std::min(1.,p)));return v[rank?rank-1:0];}
};
}
