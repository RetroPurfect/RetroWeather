#pragma once
#include <string>
#include <algorithm>
#include <cctype>
namespace upgrade_close {
inline bool process(std::string name){std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return (char)std::tolower(c);});return name=="retroweather.exe"||name=="retroweather.scr"||name=="retroweatherxp.exe";}
inline bool window(const std::string&name){return name=="RetroWeather"||name=="RetroWeatherXP"||name=="RetroWeatherSaver"||name=="RetroWeatherPreview"||name=="#32770";}
}
