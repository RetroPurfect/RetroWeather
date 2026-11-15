#pragma once
#include <string>
#include <cstdint>
#include <cstdlib>
#include <cctype>
#include <cerrno>
namespace saver {
enum KeyAction{Exit,ToggleAudio,IgnoreRepeat};
inline KeyAction key(bool system,unsigned key,uint32_t flags){return !system&&key=='M'?(flags&(1u<<30)?IgnoreRepeat:ToggleAudio):Exit;}
enum Mode{Player,Configure,Preview,Run,Invalid};
struct Options{Mode mode;uintptr_t parent;};
inline Options parse(std::string args,bool scr){
 size_t a=args.find_first_not_of(" \t");if(a==std::string::npos)return {scr?Configure:Player,0};args=args.substr(a);
 if(args.size()<2||(args[0]!='/'&&args[0]!='-'))return {Invalid,0};char c=(char)std::tolower((unsigned char)args[1]);
 if(args.size()>2&&args[2]!=':'&&args[2]!=' '&&args[2]!='\t')return {Invalid,0};
 size_t n=args.find_first_not_of(" :\t",2);uintptr_t parent=0;
 if(n!=std::string::npos){std::string value=args.substr(n);if(value.empty()||value[0]<'0'||value[0]>'9')return {Invalid,0};char*end=0;errno=0;unsigned long long h=std::strtoull(value.c_str(),&end,10);while(end&&(*end==' '||*end=='\t'))++end;if(errno==ERANGE||!end||*end||h>UINTPTR_MAX)return {Invalid,0};parent=(uintptr_t)h;}
 if(c=='s')return n==std::string::npos?Options{Run,0}:Options{Invalid,0};if(c=='p')return parent?Options{Preview,parent}:Options{Invalid,0};if(c=='c')return {Configure,parent};return {Invalid,0};
}
inline bool moved(int x,int y,int originX,int originY){return std::abs(x-originX)>4||std::abs(y-originY)>4;}
}
