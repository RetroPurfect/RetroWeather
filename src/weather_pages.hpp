#pragma once
#include "weather.hpp"
#include <cstdio>
#include <vector>
namespace weather_pages {
inline const json& section(const json&j,const char*k){static const json empty=json::object();return j.contains(k)&&j[k].is_object()?j[k]:empty;}
inline const json& at(const json&j,const char*k,int i){static const json missing;return i>=0&&j.contains(k)&&j[k].is_array()&&i<(int)j[k].size()?j[k][i]:missing;}
inline std::string value(const json&v,const std::string&unit="",int decimals=0,double scale=1){if(!v.is_number()||!std::isfinite(v.get<double>()))return "--";char b[64];std::snprintf(b,sizeof(b),"%.*f",decimals,v.get<double>()*scale);return std::string(b)+unit;}
inline int code(const json&v){return v.is_number_integer()?v.get<int>():-1;}
inline int dayIndex(const json&daily,const std::string&date){if(!daily.contains("time")||!daily["time"].is_array())return -1;for(size_t i=0;i<daily["time"].size();++i)if(daily["time"][i].is_string()&&daily["time"][i].get<std::string>()==date)return (int)i;return -1;}
inline std::vector<int> hours(const json&hourly,const std::string&start,int count=12){std::vector<int> r;if(!hourly.contains("time")||!hourly["time"].is_array())return r;for(size_t i=0;i<hourly["time"].size()&&(int)r.size()<count;++i){auto&t=hourly["time"][i];if(t.is_string()&&t.get<std::string>()>=start)r.push_back((int)i);}return r;}
struct Night{bool found=false;double low=0,chance=0,amount=0;bool haveLow=false,haveChance=false,haveAmount=false;int weather=-1;};
inline Night night(const json&h,const std::string&start,const std::string&end){Night n;if(!h.contains("time")||!h["time"].is_array())return n;for(size_t i=0;i<h["time"].size();++i){auto&t=h["time"][i];if(!t.is_string()||t.get<std::string>()<start||t.get<std::string>()>=end)continue;n.found=true;const auto&temp=at(h,"temperature_2m",i);if(temp.is_number()){double v=temp.get<double>();if(!n.haveLow||v<n.low)n.low=v;n.haveLow=true;}const auto&p=at(h,"precipitation_probability",i);if(n.weather<0)n.weather=code(at(h,"weather_code",i));if(p.is_number()){double v=p.get<double>();if(!n.haveChance||v>n.chance){n.chance=v;n.weather=code(at(h,"weather_code",i));}n.haveChance=true;}const auto&a=at(h,"precipitation",i);if(a.is_number()){n.amount+=a.get<double>();n.haveAmount=true;}}return n;}
inline const char* uvLabel(double n){return n<3?"Low":n<6?"Moderate":n<8?"High":n<11?"Very high":"Extreme";}
}
