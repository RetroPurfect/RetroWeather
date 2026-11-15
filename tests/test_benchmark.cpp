#include "../src/benchmark.hpp"
#include "../src/benchmark_weather.hpp"
#include <cassert>
#include <iostream>
int main(){
 assert(benchmark::total()==305000);assert(benchmark::page(0,0)==-1&&benchmark::page(0,14000)==-1&&benchmark::page(10,10000)==-1);for(int stage=1;stage<=9;++stage){unsigned mask=0;for(unsigned ms=0;ms<benchmark::duration(stage);ms+=3000)mask|=1u<<benchmark::page(stage,ms);assert(mask==511&&benchmark::duration(stage)-benchmark::settling>=17000);}benchmark::Timeline t;t.begin(0xfffffff0u);assert(!t.ready(0xfffffff0u+14999u));assert(t.ready(0xfffffff0u+15000u));unsigned now=0xfffffff0u;for(int i=0;i<11;++i){assert(t.stage==i&&!t.done);now+=benchmark::duration(i);assert(t.ready(now));t.next(now);}assert(t.done&&t.stage==11&&!t.ready(now));
 benchmark::Frames f;double stages[5]={1,2,3,4,5};f.add(99,stages,true,640,480,0);assert(f.transient==1&&f.frames==0&&f.transientMax==99);for(int i=1;i<=100;++i)f.add(i,stages,false,640,480,0);assert(f.frames==100&&f.elapsedSum==5050&&f.percentile(.95)==95&&f.maximum==100);f.add(1,stages,false,1280,960,1);assert(f.geometryChanges==1);for(int i=0;i<10000;++i)f.add(2,stages,false,1280,960,1);assert(f.times.size()==4096&&f.frames==10101);
 for(int cancelled=0;cancelled<11;++cancelled){benchmark_weather::Session session;json data={{"timezone","America/Detroit"},{"current",{{"time","2026-11-14T23:00"}}}};auto original=data.dump();int background=3;for(int stage=0;stage<=cancelled;++stage){session.enter(stage==0?0:std::min(8,stage-1),data,background,1794690000);if(stage==0)background=0;}assert(session.leave(data,background));assert(data.dump()==original&&background==3&&!session.active);}
 std::cout<<"PASS: full 305-second stage schedule and tick wrap, warm/cold separation, percentiles, mixed geometry, bounded sample storage and cancellation restoration at every stage.\n";
}
