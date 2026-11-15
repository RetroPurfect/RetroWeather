#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
namespace features {
static const int pageCount=9, pausePage=9;
static const unsigned allPages=511;
static const char* const pageNames[]={"Current Conditions","Seven-Day Forecast","Weather Details","Sun & Moon","US Holidays","Time & Date","Today & Tonight","Hourly Forecast","About Retro Weather"};
struct PageConfig {
 std::vector<int> order={0,6,7,1,2,3,5,4,8};unsigned mask=511;
 std::vector<int> enabled()const{std::vector<int> r;for(int n:order)if(mask&(1u<<n))r.push_back(n);if(r.empty())r.push_back(0);return r;}
 std::string serialize()const{std::string s;for(int n:order){if(!s.empty())s+=",";s+=std::to_string(n);}return s;}
 bool parse(const std::string&s,unsigned bits){std::istringstream in(s);std::string token;std::vector<int> v;while(std::getline(in,token,',')){if(token.size()!=1||token[0]<'0'||token[0]>'8')return false;int n=token[0]-'0';if(std::find(v.begin(),v.end(),n)!=v.end())return false;v.push_back(n);}if(v.size()!=9&&v.size()!=7)return false;
 if(v.size()==7){for(int n:v)if(n>6)return false;std::vector<int> migrated;for(int n:v){if(n==6)n=3;if(std::find(migrated.begin(),migrated.end(),n)==migrated.end())migrated.push_back(n);}for(int n:{6,7,8})migrated.push_back(n);v=migrated;bits=(bits&63)|((bits&64)?8:0)|448;}
 order=v;mask=(bits&allPages)?bits&allPages:1;return true;}

};
struct Cycle {
 int page=0,elapsed=0,hold=0;bool paused=false;std::vector<int> order={0,6,7,1,2,3,5,4,8};
 void configure(const PageConfig&config){order=config.enabled();restart();}
 void restart(){page=order.front();elapsed=hold=0;}
 void advance(int pause){auto it=std::find(order.begin(),order.end(),page);elapsed=0;if(it==order.end()||it+1==order.end()){if(pause>0){page=pausePage;hold=pause;}else page=order.front();}else page=*(it+1);}
 void tick(int duration,int pause){if(paused)return;if(page==pausePage){if(--hold<=0)restart();}else if(++elapsed>=duration)advance(pause);}
 void next(){if(page==pausePage)restart();else advance(0);}
};
inline std::vector<std::string> parseZips(const std::string& s){std::istringstream in(s);std::string z;std::vector<std::string> out;while(std::getline(in,z,',')){bool good=z.size()==5;for(char c:z)if(c<'0'||c>'9')good=false;if(good&&std::find(out.begin(),out.end(),z)==out.end()&&out.size()<10)out.push_back(z);}return out;}
inline std::string joinZips(const std::vector<std::string>&zips){std::string s;for(const auto&z:zips){if(!s.empty())s+=",";s+=z;}return s;}
inline std::string solarTime(const std::string&iso,bool format24){size_t pos=iso.find('T');if(pos==std::string::npos||iso.size()<pos+6)return "Not reported";int h=-1,m=-1;if(std::sscanf(iso.c_str()+pos+1,"%d:%d",&h,&m)!=2||h<0||h>23||m<0||m>59)return "Not reported";std::string s=presentation::clockText(h,m,format24);if(!format24)s+=h>=12?" PM":" AM";return s;}
}
