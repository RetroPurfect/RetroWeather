#include "../src/smooth_scale.hpp"
#include <chrono>
#include <iostream>
#include <vector>
int main(){
#if RW_SCALE_SSE2
 if(!__builtin_cpu_supports("sse2"))return 0;const int sw=1279,sh=659,dw=1920,dh=989;std::vector<uint32_t> src(sw*sh),dst(dw*dh);uint32_t rng=12345;for(auto&c:src){rng=rng*1664525u+1013904223u;c=rng;}rendering::SmoothScale plan;plan.prepare(sw,sh,dw,dh);volatile uint32_t checksum=0;double ms[2];for(int mode=0;mode<2;++mode){plan.setVectorAllowed(mode!=0);plan.draw(src.data(),dst.data());auto start=std::chrono::steady_clock::now();for(int i=0;i<40;++i){plan.draw(src.data(),dst.data());checksum+=dst[(i*19789)%dst.size()];}ms[mode]=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/40;}
 std::cout<<"Build-host scaler-only benchmark, 1279x659 -> 1920x989, 40 warm frames/path\nScalar: "<<ms[0]<<" ms/frame\nSSE2: "<<ms[1]<<" ms/frame\nRatio: "<<ms[0]/ms[1]<<"x; plan memory: "<<plan.bytes()<<" bytes; checksum: "<<checksum<<"\nThese are Linux build-host measurements, not Windows app CPU/FPS results.\n";
#endif
}
