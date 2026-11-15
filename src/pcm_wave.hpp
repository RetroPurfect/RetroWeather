#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
namespace pcm_wave {
inline uint32_t u32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
inline unsigned u16(const unsigned char*p){return p[0]|unsigned(p[1])<<8;}
inline std::vector<char> decode(const std::vector<unsigned char>&b){if(b.size()<44||b.size()>2*1024*1024||std::string((const char*)b.data(),4)!="RIFF"||std::string((const char*)b.data()+8,4)!="WAVE"||u32(b.data()+4)!=b.size()-8)throw std::runtime_error("Invalid weather sound file");bool format=false;std::vector<char> pcm;for(size_t at=12;at+8<=b.size();){uint32_t n=u32(b.data()+at+4);size_t start=at+8;if(n>b.size()-start)throw std::runtime_error("Truncated weather sound file");std::string tag((const char*)b.data()+at,4);if(tag=="fmt "){if(n<16||u16(&b[start])!=1||u16(&b[start+2])!=1||u32(&b[start+4])!=22050||u32(&b[start+8])!=44100||u16(&b[start+12])!=2||u16(&b[start+14])!=16)throw std::runtime_error("Weather sound must be mono 16-bit 22050 Hz PCM");format=true;}if(tag=="data"){if(n==0||n%2)throw std::runtime_error("Invalid PCM length");pcm.assign(b.begin()+start,b.begin()+start+n);}at=start+n+(n&1);}if(!format||pcm.empty())throw std::runtime_error("Incomplete weather sound file");return pcm;}
}
