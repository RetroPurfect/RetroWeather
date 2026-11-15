#include "../src/recording_plan.hpp"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
int main(int argc,char**argv){
 for(int w:{640,735,1920,3840,5120})for(int h:{360,480,835,1080,2160,2880})for(int index=0;index<5;++index){auto p=recording_plan::fit(w,h,index),m=recording_plan::maximum(index);assert(p.width>=2&&p.height>=2&&!(p.width&1)&&!(p.height&1)&&p.width<=m.width&&p.height<=m.height);assert(p.width<=w&&p.height<=h);assert(std::abs(double(p.width)/p.height-double(w)/h)/(double(w)/h)<.025);}
 assert(recording_plan::fit(3840,2160,4).width==3840&&recording_plan::fit(3840,2160,4).height==2160);assert(recording_plan::fit(1920,1080,4).width==1920);assert(recording_plan::fit(1920,1080,2).width==1280);assert(recording_plan::fit(1920,1080,0).height==360);assert(recording_plan::fit(0,0,0).width==0);assert(recording_plan::resolutionIndex(-1)==2&&recording_plan::resolutionIndex(5)==2);
 unsigned previous=0;for(int i=0;i<5;++i){auto m=recording_plan::maximum(i);auto rate=recording_plan::bitrate(m.width,m.height);assert(rate>previous);previous=rate;}assert(recording_plan::bitrate(3840,2160)==8000000&&recording_plan::bitrate(640,360)==300000);
 assert(recording_plan::totalMs==144000);for(unsigned stage=0;stage<9;++stage){unsigned pages=0;for(unsigned ms=stage*16000;ms<(stage+1)*16000;ms+=100){assert(recording_plan::stage(ms)==stage);pages|=1u<<recording_plan::page(ms);}assert(pages==511);}
 std::vector<unsigned char>yuv;std::vector<uint32_t>p(4,0);recording_plan::nv12(p.data(),2,2,yuv);assert(yuv.size()==6&&yuv[0]==16&&yuv[4]==128&&yuv[5]==128);p.assign(4,0xffffff);recording_plan::nv12(p.data(),2,2,yuv);assert(yuv[0]==235&&yuv[4]==128&&yuv[5]==128);p.assign(4,0xff0000);recording_plan::nv12(p.data(),2,2,yuv);assert(yuv[0]>=81&&yuv[0]<=82&&yuv[4]==90&&yuv[5]==240);
 std::vector<char>pcm={char(0xe8),3,char(0x18),char(0xfc)};assert(recording_plan::pcm44100(pcm,0,100)==1000&&recording_plan::pcm44100(pcm,1,100)==0&&recording_plan::pcm44100(pcm,2,100)==-1000&&recording_plan::pcm44100(pcm,4,50)==500&&recording_plan::pcm44100(pcm,100,0)==0);
 if(argc==2){const int w=640,h=360;std::ofstream video(std::string(argv[1])+".nv12",std::ios::binary),audio(std::string(argv[1])+".pcm",std::ios::binary);p.resize(w*h);for(int frame=0;frame<30;++frame){for(int y=0;y<h;++y)for(int x=0;x<w;++x)p[y*w+x]=y<h/2?(x<w/2?0xff0000:0x00ff00):(x<w/2?0x0000ff:0xffffff);recording_plan::nv12(p.data(),w,h,yuv);video.write((const char*)yuv.data(),yuv.size());for(int i=0;i<4410;++i){int16_t sample=recording_plan::pcm44100(pcm,frame*4410+i,50);audio.write((const char*)&sample,2);}}}
 std::cout<<"PASS: aspect-preserving even recording sizes, full scene/page replay coverage, five resolution tiers without upscaling, pixel-based bitrates, NV12 color/row layout and looped 22.05-to-44.1kHz audio generation.\n";
}
