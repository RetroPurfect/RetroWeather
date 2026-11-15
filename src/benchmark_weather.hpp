#pragma once
#include "weather.hpp"
#include "calendar.hpp"
namespace benchmark_weather {
inline int code(int index){const int codes[]={0,0,2,3,51,63,95,73,45};return index>=0&&index<9?codes[index]:-1;}
inline json sample(int index,int64_t now){int c=code(index);double temp=index==7?28.:64.;std::string date=calendar::dateString(calendar::fromEpoch(now));json daily;for(const char*k:{"time","weather_code","temperature_2m_max","temperature_2m_min","precipitation_probability_max","sunrise","sunset","daylight_duration","precipitation_sum","snowfall_sum","uv_index_max"})daily[k]=json::array();for(int i=0;i<7;++i){std::string day=calendar::dateString(calendar::fromEpoch(now+i*86400));daily["time"].push_back(day);daily["weather_code"].push_back(i?code((index+i)%9):c);daily["temperature_2m_max"].push_back(temp+5+i%3*2);daily["temperature_2m_min"].push_back(temp-6-i%3);daily["precipitation_probability_max"].push_back(index>=4&&index<=7?80-i*4:5+i*7);daily["sunrise"].push_back(day+"T06:30");daily["sunset"].push_back(day+"T18:30");daily["daylight_duration"].push_back(43200);daily["precipitation_sum"].push_back(index>=4?4.2:0);daily["snowfall_sum"].push_back(index==7?3.1:0);daily["uv_index_max"].push_back(index>=3?2.1:5.7);}
json hourly;for(const char*k:{"time","temperature_2m","dew_point_2m","visibility","weather_code","precipitation_probability","precipitation","is_day","wind_speed_10m"})hourly[k]=json::array();int64_t first=now-now%3600;for(int i=0;i<48;++i){auto dt=calendar::fromEpoch(first+i*3600);char time[7];std::snprintf(time,sizeof(time),"%02d:00",dt.hour);hourly["time"].push_back(calendar::dateString(dt)+"T"+time);hourly["temperature_2m"].push_back(temp+std::sin(i*.5)*6);hourly["dew_point_2m"].push_back(temp-9);hourly["visibility"].push_back(index==8?500:16000);hourly["weather_code"].push_back(i?code((index+i/3)%9):c);hourly["precipitation_probability"].push_back((i*13+index*9)%101);hourly["precipitation"].push_back(index>=4?.4:0.);hourly["is_day"].push_back(dt.hour>=6&&dt.hour<18?1:0);hourly["wind_speed_10m"].push_back(8+i%6);}
return json{{"city","Weather Preview"},{"state","Simulated"},{"country","Sample weather"},{"zip","TEST"},{"timezone","UTC"},{"utc_offset_seconds",0},{"fetched",now},{"current",{{"time",date+"T12:00"},{"temperature_2m",temp},{"apparent_temperature",temp-2},{"relative_humidity_2m",index>=4?90:55},{"wind_speed_10m",8},{"wind_gusts_10m",15},{"wind_direction_10m",270},{"cloud_cover",index>=3?95:25},{"pressure_msl",1015},{"weather_code",c},{"is_day",index==1?0:1}}},{"daily",daily},{"hourly",hourly}};}

struct Session {
 bool active=false;int index=-1,originalBackground=0;json live;
 bool enter(int next,json&data,int&background,int64_t now){if(next<0||next>=9)return false;if(!active){live=data;originalBackground=background;}active=true;index=next;data=sample(next,now);background=4;return true;}
 bool leave(json&data,int&background){if(!active)return false;data.swap(live);live=json();background=originalBackground;active=false;index=-1;return true;}
};
inline uint32_t hash(unsigned n){n^=n>>16;n*=0x7feb352d;n^=n>>15;n*=0x846ca68b;n^=n>>16;return n;}
inline double unit(unsigned n){return (hash(n)&0xffffff)/16777216.;}
struct Drop{double x,y,depth;};
inline Drop rain(unsigned i,double t,bool drizzle){double z=.45+unit(i*7+3)*.7;double y=std::fmod(unit(i*7+1)*520+t*(drizzle?85:125)*(z+.5),520.)-20;double x=std::fmod(unit(i*7+2)*720-t*(drizzle?9:25)*z,720.);if(x<0)x+=720.;return {x-40,y,z};}
}
