#include "../src/smooth_scale.hpp"
#include "../src/render_profile.hpp"
#include <cassert>
#include <iostream>
#include <vector>
static uint32_t scalar(uint32_t a,uint32_t b,unsigned t){uint32_t c=0;for(unsigned shift:{0u,8u,16u})c|=(((((a>>shift)&255)*(256-t)+((b>>shift)&255)*t)>>8)<<shift);return c;}
int main(){for(unsigned a:{0u,0xffffffu,0x20314au,0xabcdefu})for(unsigned b:{0u,0xffffffu,0xf0af34u})for(unsigned t=0;t<=256;++t)assert(rendering::mixRGB(a,b,t)==scalar(a,b,t));
rendering::SmoothScale plan;for(auto size:std::vector<std::array<int,4>>{{{1,1,9,7}},{{2,2,3,3}},{{7,11,23,19}},{{23,19,7,11}},{{19,1,37,1}},{{1,17,1,31}}}){int sw=size[0],sh=size[1],dw=size[2],dh=size[3];std::vector<uint32_t> src(sw*sh),out(dw*dh+2,0xdeadbeef);for(int y=0;y<sh;++y)for(int x=0;x<sw;++x)src[y*sw+x]=(x*11%256<<16)|(y*13%256<<8)|((x+y)*7%256);assert(plan.prepare(sw,sh,dw,dh));assert(plan.draw(src.data(),out.data()+1));assert(out.front()==0xdeadbeef&&out.back()==0xdeadbeef);for(int y=0;y<dh;++y)for(int x=0;x<dw;++x){auto a=plan.xs[x],b=plan.ys[y];uint32_t upper=scalar(src[b.a*sw+a.a],src[b.a*sw+a.b],a.t),lower=scalar(src[b.b*sw+a.a],src[b.b*sw+a.b],a.t);assert(out[1+y*dw+x]==scalar(upper,lower,b.t));}std::fill(src.begin(),src.end(),0x123456);plan.draw(src.data(),out.data()+1);for(int i=0;i<dw*dh;++i)assert(out[i+1]==0x123456);}
assert(!plan.prepare(0,1,1,1));plan.clear();assert(plan.bytes()==0&&!plan.draw(nullptr,nullptr));
rendering::ScaleChoice q;q.configure(1280,660,1920,990);for(int i=0;i<6;++i){int m=q.method();q.record(m,m==0?9.:3.);}assert(q.complete()&&q.method()==1&&q.median(0)==9&&q.median(1)==3);q.configure(640,480,1920,1080);assert(!q.complete());for(int i=0;i<6;++i){int m=q.method();q.record(m,m==0?3.:5.);}assert(q.complete()&&q.method()==0);
rendering::RenderProfile profile;profile.observe(4,10,1,2,true);assert(profile.samples==0);profile.observe(4,10,1,2,false);profile.observe(4,10,1,2,false);assert(profile.samples==2&&profile.average[1]==10);profile.reset();assert(profile.samples==0);
std::cout<<"PASS: cached bilinear scaling against scalar reference, edge/single-pixel/resize/frame-change cases, clipped buffers, warm six-frame path selection and stage profiling.\n";}
