#pragma once
#include <cstdint>
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#include <emmintrin.h>
#define RW_BLEND_SSE2 1
#define RW_BLEND_TARGET __attribute__((target("sse2")))
#else
#define RW_BLEND_SSE2 0
#define RW_BLEND_TARGET
#endif
namespace rendering {
#if RW_BLEND_SSE2
RW_BLEND_TARGET inline int blendVector(uint32_t*dst,const uint32_t*src,int count){
 const __m128i zero=_mm_setzero_si128(),one=_mm_set1_epi16(1),full=_mm_set1_epi16(255),rgb=_mm_set1_epi32(0xffffff);
 int i=0;for(;i+4<=count;i+=4){
  __m128i s=_mm_loadu_si128((const __m128i*)(src+i)),d=_mm_and_si128(_mm_loadu_si128((const __m128i*)(dst+i)),rgb);
  __m128i sl=_mm_unpacklo_epi8(s,zero),sh=_mm_unpackhi_epi8(s,zero);
  __m128i al=_mm_shufflehi_epi16(_mm_shufflelo_epi16(sl,255),255),ah=_mm_shufflehi_epi16(_mm_shufflelo_epi16(sh,255),255);
  __m128i lo=_mm_mullo_epi16(_mm_unpacklo_epi8(d,zero),_mm_sub_epi16(full,al)),hi=_mm_mullo_epi16(_mm_unpackhi_epi8(d,zero),_mm_sub_epi16(full,ah));
  lo=_mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(lo,one),_mm_srli_epi16(lo,8)),8);
  hi=_mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(hi,one),_mm_srli_epi16(hi,8)),8);
  __m128i result=_mm_and_si128(_mm_add_epi8(_mm_packus_epi16(lo,hi),s),rgb);
  _mm_storeu_si128((__m128i*)(dst+i),result);
 }return i;
}
#endif
}
