#pragma once
#include "json.hpp"
#include <string>
#include <stdexcept>
#include <cmath>
using nlohmann::json;
inline bool validZip(const std::string& z){if(z.size()!=5)return false;for(char c:z)if(c<'0'||c>'9')return false;return true;}
inline std::string conditions(int c){
 if(c==0)return "Clear";if(c==1)return "Mostly clear";if(c==2)return "Partly cloudy";if(c==3)return "Overcast";
 if(c==45||c==48)return "Fog";if(c==56||c==57)return "Freezing drizzle";if(c>=51&&c<=55)return "Drizzle";if(c==66||c==67)return "Freezing rain";if(c==65)return "Heavy rain";if(c>=61&&c<=63)return "Rain";
 if(c>=71&&c<=77)return "Snow";if(c>=80&&c<=82)return "Rain showers";if(c==85||c==86)return "Snow showers";
 if(c>=95&&c<=99)return "Thunderstorms";return "Unavailable";
}
inline std::string number(const json& j,const char* key,const std::string& unit=""){
 if(!j.contains(key)||!j[key].is_number())return "--";return std::to_string((int)std::round(j[key].get<double>()))+unit;
}
inline void validateWeather(const json& j,int days=5){
 if(!j.contains("current")||!j["current"].is_object()||!j["current"].contains("temperature_2m")||!j["current"]["temperature_2m"].is_number())throw std::runtime_error("Weather response has no current temperature.");
 if(!j.contains("daily")||!j["daily"].is_object())throw std::runtime_error("Weather response has no daily forecast.");
 for(auto k:{"city","state","state_name","country","zip"})if(j.contains(k)&&!j[k].is_string())throw std::runtime_error("Invalid location metadata.");
 if(j.contains("fetched")&&!j["fetched"].is_number_integer())throw std::runtime_error("Invalid cache timestamp.");
 if(j["current"].contains("time")&&!j["current"]["time"].is_string())throw std::runtime_error("Invalid forecast timestamp.");
 const auto& d=j["daily"];for(auto k:{"time","temperature_2m_max","temperature_2m_min","weather_code","precipitation_probability_max"})if(!d.contains(k)||!d[k].is_array()||d[k].size()<(size_t)days)throw std::runtime_error("Incomplete daily forecast.");
 for(int i=0;i<days;++i)if(!d["time"][i].is_string())throw std::runtime_error("Invalid forecast date.");
}
