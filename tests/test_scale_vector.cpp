#include "../src/smooth_scale.hpp"
#include <cassert>
#include <iostream>
#include <vector>
int main(){
#if RW_SCALE_SSE2
 if(!__builtin_cpu_supports("sse2")){std::cout<<"SKIP: host lacks SSE2; scalar suites remain active.\n";return 0;}
 uint32_t rng=131071;std::vector<std::array<int,4>> cases={{{1,1,7,9}},{{2,2,3,5}},{{17,19,1,1}},{{1279,659,1920,989}},{{1280,720,3840,2160}}};for(int i=0;i<100;++i){rng=rng*1664525u+1013904223u;int sw=1+rng%91;rng=rng*1664525u+1013904223u;int sh=1+rng%63;rng=rng*1664525u+1013904223u;int dw=1+rng%103;rng=rng*1664525u+1013904223u;int dh=1+rng%89;cases.push_back({{sw,sh,dw,dh}});}
 uint64_t pixels=0;rendering::SmoothScale a,b;for(auto dims:cases){int sw=dims[0],sh=dims[1],dw=dims[2],dh=dims[3];std::vector<uint32_t> src(sw*sh+2,0xcafebabe),scalar(dw*dh+2,0xdeadbeef),vector=scalar;for(int i=1;i<=sw*sh;++i){rng=rng*1664525u+1013904223u;src[i]=rng;}assert(a.prepare(sw,sh,dw,dh)&&b.prepare(sw,sh,dw,dh));a.setVectorAllowed(false);b.setVectorAllowed(true);assert(!a.simdActive()&&b.simdActive());assert(a.draw(src.data()+1,scalar.data()+1)&&b.draw(src.data()+1,vector.data()+1));assert(scalar==vector);assert(vector.front()==0xdeadbeef&&vector.back()==0xdeadbeef);pixels+=(uint64_t)dw*dh;std::fill(src.begin()+1,src.end()-1,0xff123456);b.draw(src.data()+1,vector.data()+1);for(int i=1;i<=dw*dh;++i)assert(vector[i]==0x123456);b.clear();assert(!b.draw(nullptr,nullptr)&&b.bytes()==0);}
 std::cout<<"PASS: guarded SSE2/scalar scaling pixel equality across "<<cases.size()<<" geometries, "<<pixels<<" pixels, odd/unaligned/edge/downscale/1080p/4K and changed-source cases.\n";
#else
 std::cout<<"SKIP: vector kernel not compiled on this architecture.\n";
#endif
}
