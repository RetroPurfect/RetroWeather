#include "../src/raster_cache.hpp"
#include <cassert>
#include <iostream>
#include <chrono>
int main(){
#if RW_BLEND_SSE2
 if(!__builtin_cpu_supports("sse2"))return 0;
 uint32_t seed=12345;auto random=[&](){seed=seed*1664525u+1013904223u;return seed;};
 for(int width:{1,3,4,5,63,128,319})for(int height:{1,13}){
 rendering::RasterCache cache;cache.build(width,height,[&](int x,int y){unsigned a=(x+y)%7==0?0:random()>>24;return rendering::premultiply(random()&0xffffff,a);});
 for(int ox:{-width,-3,0,1,11})for(int oy:{-4,0,2})for(unsigned gain:{0u,128u,255u}){
 std::vector<uint32_t>a(97*19);for(auto&v:a)v=random()&0xffffff;auto b=a;
 cache.composite(a.data(),97,19,ox,oy,{2,1,93,18},gain,false);cache.composite(b.data(),97,19,ox,oy,{2,1,93,18},gain,true);assert(a==b);
 }}
 const int size=1024*1024;std::vector<uint32_t>s(size),a(size),b(size);for(int i=0;i<size;++i){s[i]=rendering::premultiply(random()&0xffffff,1+(random()%255));a[i]=random()&0xffffff;}b=a;
 for(int i=0;i<size;++i)a[i]=rendering::over(a[i],s[i]);int n=rendering::blendVector(b.data(),s.data(),size);assert(n==size&&a==b);
 auto run=[&](bool vector){auto start=std::chrono::steady_clock::now();for(int repeat=0;repeat<60;++repeat){if(vector)rendering::blendVector(b.data(),s.data(),size);else for(int i=0;i<size;++i)a[i]=rendering::over(a[i],s[i]);}return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();};
 double scalar=run(false),simd=run(true);assert(a==b);std::cout<<"PASS: exact scalar/SSE2 blend, clipping, offsets, gain and tails. 60 megapixel passes scalar="<<scalar<<" ms vector="<<simd<<" ms; speedup="<<scalar/simd<<"x (build host).\n";
#endif
}
