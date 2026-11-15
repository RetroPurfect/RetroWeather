#pragma once
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>
#include <cmath>
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#include <emmintrin.h>
#define RW_SCALE_SSE2 1
#define RW_SCALE_TARGET __attribute__((target("sse2")))
#else
#define RW_SCALE_SSE2 0
#endif
namespace rendering {
inline uint32_t mixRGB(uint32_t a,uint32_t b,unsigned t){uint32_t rb=((a&0xff00ff)*(256-t)+(b&0xff00ff)*t)>>8,g=((a&0xff00)*(256-t)+(b&0xff00)*t)>>8;return (rb&0xff00ff)|(g&0xff00);}

struct ScaleTap{int a,b;unsigned t;};
struct SmoothScale {
 bool vectorAllowed=false;std::vector<uint16_t> weights;
 void setVectorAllowed(bool value){vectorAllowed=value;}
 bool simdActive()const{return vectorAllowed&&RW_SCALE_SSE2;}
 int sw=0,sh=0,dw=0,dh=0;std::vector<ScaleTap> xs,ys;std::vector<uint32_t> rows[2];int sourceRows[2]={-1,-1};
 static std::vector<ScaleTap> axis(int src,int dst){std::vector<ScaleTap> out(dst);for(int i=0;i<dst;++i){double p=std::max(0.,std::min(src-1.,(i+.5)*src/dst-.5));int a=(int)std::floor(p);out[i]={a,std::min(src-1,a+1),(unsigned)std::min(256.,std::floor((p-a)*256+.5))};}return out;}
 bool prepare(int a,int b,int c,int d){if(a<=0||b<=0||c<=0||d<=0)return false;if(sw==a&&sh==b&&dw==c&&dh==d)return true;sw=a;sh=b;dw=c;dh=d;xs=axis(sw,dw);ys=axis(sh,dh);for(auto&r:rows)r.assign(dw,0);weights.resize((size_t)dw*4);for(int x=0;x<dw;++x)for(int c=0;c<4;++c)weights[x*4+c]=(uint16_t)xs[x].t;return true;}
 int row(const uint32_t*src,int y,int avoid){for(int i=0;i<2;++i)if(sourceRows[i]==y)return i;int slot=avoid==0?1:0;const uint32_t*s=src+(size_t)y*sw;uint32_t*d=rows[slot].data();for(int x=0;x<dw;++x){auto t=xs[x];d[x]=mixRGB(s[t.a],s[t.b],t.t);}sourceRows[slot]=y;return slot;}
 bool drawScalar(const uint32_t*src,uint32_t*dst){if(!src||!dst||sw<=0||sh<=0||dw<=0||dh<=0)return false;sourceRows[0]=sourceRows[1]=-1;for(int y=0;y<dh;++y){auto t=ys[y];int a=row(src,t.a,-1),b=row(src,t.b,a);auto*p=rows[a].data();auto*q=rows[b].data();uint32_t*d=dst+(size_t)y*dw;for(int x=0;x<dw;++x)d[x]=mixRGB(p[x],q[x],t.t);}return true;}
#if RW_SCALE_SSE2
 RW_SCALE_TARGET int vectorRow(const uint32_t*src,int y,int avoid){
 for(int i=0;i<2;++i)if(sourceRows[i]==y)return i;int slot=avoid==0?1:0;const uint32_t*s=src+(size_t)y*sw;uint32_t*d=rows[slot].data();
 __m128i zero=_mm_setzero_si128(),full=_mm_set1_epi16(256),rgb=_mm_set1_epi32(0xffffff);int x=0;
 for(;x+1<dw;x+=2){auto p=xs[x],q=xs[x+1];__m128i a=_mm_set_epi32(0,0,(int)s[q.a],(int)s[p.a]),b=_mm_set_epi32(0,0,(int)s[q.b],(int)s[p.b]);a=_mm_unpacklo_epi8(a,zero);b=_mm_unpacklo_epi8(b,zero);__m128i t=_mm_loadu_si128((const __m128i*)(weights.data()+x*4));__m128i sum=_mm_add_epi16(_mm_mullo_epi16(a,_mm_sub_epi16(full,t)),_mm_mullo_epi16(b,t));sum=_mm_srli_epi16(sum,8);sum=_mm_and_si128(_mm_packus_epi16(sum,zero),rgb);_mm_storel_epi64((__m128i*)(d+x),sum);}
 for(;x<dw;++x){auto t=xs[x];d[x]=mixRGB(s[t.a],s[t.b],t.t);}sourceRows[slot]=y;return slot;
 }
 RW_SCALE_TARGET bool drawVector(const uint32_t*src,uint32_t*dst){
 if(!src||!dst||sw<=0||sh<=0||dw<=0||dh<=0)return false;sourceRows[0]=sourceRows[1]=-1;
 __m128i zero=_mm_setzero_si128(),rgb=_mm_set1_epi32(0xffffff);
 for(int y=0;y<dh;++y){auto tap=ys[y];int a=vectorRow(src,tap.a,-1),b=vectorRow(src,tap.b,a);const uint32_t*p=rows[a].data(),*q=rows[b].data();uint32_t*d=dst+(size_t)y*dw;
 if(tap.t==0||tap.a==tap.b){std::copy(p,p+dw,d);continue;}if(tap.t==256){std::copy(q,q+dw,d);continue;}
 __m128i t=_mm_set1_epi16((short)tap.t),inv=_mm_set1_epi16((short)(256-tap.t));int x=0;
 for(;x+3<dw;x+=4){__m128i av=_mm_loadu_si128((const __m128i*)(p+x)),bv=_mm_loadu_si128((const __m128i*)(q+x));__m128i lo=_mm_add_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(av,zero),inv),_mm_mullo_epi16(_mm_unpacklo_epi8(bv,zero),t)),hi=_mm_add_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(av,zero),inv),_mm_mullo_epi16(_mm_unpackhi_epi8(bv,zero),t));__m128i out=_mm_and_si128(_mm_packus_epi16(_mm_srli_epi16(lo,8),_mm_srli_epi16(hi,8)),rgb);_mm_storeu_si128((__m128i*)(d+x),out);}
 for(;x<dw;++x)d[x]=mixRGB(p[x],q[x],tap.t);
 }return true;
 }
#endif
 bool draw(const uint32_t*src,uint32_t*dst){
#if RW_SCALE_SSE2
 if(vectorAllowed)return drawVector(src,dst);
#endif
 return drawScalar(src,dst);
 }
 void clear(){sw=sh=dw=dh=0;std::vector<uint16_t>().swap(weights);std::vector<ScaleTap>().swap(xs);std::vector<ScaleTap>().swap(ys);for(auto&r:rows)std::vector<uint32_t>().swap(r);}
 size_t bytes()const{return xs.capacity()*sizeof(ScaleTap)+ys.capacity()*sizeof(ScaleTap)+(rows[0].capacity()+rows[1].capacity())*4+weights.capacity()*2;}
};
struct ScaleChoice {
 int sw=0,sh=0,dw=0,dh=0,counts[2]={0,0},selected=0;double costs[2][3]={{0}};
 void configure(int a,int b,int c,int d){if(sw==a&&sh==b&&dw==c&&dh==d)return;sw=a;sh=b;dw=c;dh=d;counts[0]=counts[1]=0;selected=0;}
 bool complete()const{return counts[0]==3&&counts[1]==3;}
 int method()const{return complete()?selected:counts[0]<=counts[1]?0:1;}
 double median(int m)const{if(counts[m]!=3)return -1;auto a=costs[m][0],b=costs[m][1],c=costs[m][2];return a+b+c-std::min(a,std::min(b,c))-std::max(a,std::max(b,c));}
 void record(int method,double ms){if(method<0||method>1||ms<0||complete()||counts[method]>=3)return;costs[method][counts[method]++]=ms;if(complete())selected=median(1)<median(0)*.95?1:0;}
};
}
