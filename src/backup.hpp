#pragma once
#include "locations.hpp"
#include "presentation.hpp"
#include "features.hpp"
#include <map>
namespace backup {
inline std::map<std::string,std::pair<int,int>> ranges(){std::map<std::string,std::pair<int,int>> r={{"PageSeconds",{1,3600}},{"PauseSeconds",{0,3600}},{"Clock24",{0,1}},{"AudioVolume",{0,100}},{"AudioEnabled",{0,1}},{"PagesPaused",{0,1}},{"Background",{0,4}},{"AspectMode",{0,5}},{"StartFullscreen",{0,1}},{"Celsius",{0,1}},{"WindUnit",{0,3}},{"CrtMode",{0,1}},{"SafeMargin",{0,10}},{"TextPercent",{90,110}},{"PageMask",{1,511}},{"SaverAudio",{0,1}},{"WindowSaved",{0,1}},{"WindowWidth96",{100,32768}},{"WindowHeight96",{100,32768}},{"WindowX96",{-100000,100000}},{"WindowY96",{-100000,100000}},{"WindowMaximized",{0,1}}};for(int i=0;i<9;++i)r["PageOverride"+std::to_string(i)]={0,3600};return r;}
inline std::vector<std::string> strings(){return {"ZIP","SavedZips","PageOrder","WindowMonitor","FullscreenMonitor"};}
inline json validate(const json&input){
 json j=input;
 if(j.is_object()&&j.value("format",std::string())=="RetroWeatherSettings"&&j.contains("version")&&j["version"].is_number_integer()&&j["version"]==1&&j.contains("settings")&&j["settings"].is_object()){
 auto& st=j["settings"];for(auto row:std::vector<std::pair<std::string,std::string>>{{"MusicVolume","AudioVolume"},{"MusicEnabled","AudioEnabled"},{"MusicPaused","PagesPaused"},{"SaverMusic","SaverAudio"}}){if(st.contains(row.first)){if(!st.contains(row.second))st[row.second]=st[row.first];st.erase(row.first);}}
 // Legacy scenic and playlist settings have no equivalent in the five-choice UI.
 for(auto key:{"ScenicAnimation","ScenicSchema","MusicMode","MusicTrack"})st.erase(key);
 if(st.contains("Background")&&st["Background"].is_string()){auto v=st["Background"].get<std::string>();if(v=="5"||v=="6"||v=="7")st["Background"]="4";}
 j["version"]=2;
 }

 if(!j.is_object()||j.value("format",std::string())!="RetroWeatherSettings"||!j.contains("version")||!j["version"].is_number_integer()||j["version"]!=2)throw std::runtime_error("Not a supported Retro Weather settings backup.");
 if(!j.contains("settings")||!j["settings"].is_object()||!j.contains("locations")||!j["locations"].is_object())throw std::runtime_error("Backup is incomplete.");
 if(j["settings"].contains("ScenicTransparency")){auto v=j["settings"]["ScenicTransparency"];if(!v.is_string()||(v!="0"&&v!="25"&&v!="50"&&v!="75"&&v!="100"))throw std::runtime_error("Invalid legacy transparency.");j["settings"].erase("ScenicTransparency");}
 auto r=ranges();auto names=strings();for(auto it=j["settings"].begin();it!=j["settings"].end();++it){if(!it.value().is_string())throw std::runtime_error("Invalid setting value.");auto v=it.value().get<std::string>();if(v.size()>2048||v.find('\0')!=std::string::npos||v.find('\r')!=std::string::npos||v.find('\n')!=std::string::npos)throw std::runtime_error("Invalid setting text.");auto at=r.find(it.key());if(at!=r.end()){size_t used=0;long n=std::stol(v,&used);if(used!=v.size()||n<at->second.first||n>at->second.second)throw std::runtime_error("Setting out of range: "+it.key());}else if(std::find(names.begin(),names.end(),it.key())==names.end())throw std::runtime_error("Unknown backup setting: "+it.key());}
 auto st=j["settings"];for(auto key:{"ZIP","SavedZips"})if(st.contains(key)){std::string v=st[key];if(std::string(key)=="ZIP"){if(!v.empty()&&!validZip(v))throw std::runtime_error("Invalid backup ZIP.");}else if(features::joinZips(features::parseZips(v))!=v)throw std::runtime_error("Invalid saved ZIP list.");}
 if(st.contains("PageOrder")){auto v=st["PageOrder"].get<std::string>();features::PageConfig p;if(!p.parse(v,511))throw std::runtime_error("Invalid page order.");}
 if(j["locations"].size()>10||locations::sanitize(j["locations"])!=j["locations"])throw std::runtime_error("Invalid location metadata.");
 for(const auto&loc:j["locations"])if((loc.contains("latitude")||loc.contains("longitude"))&&!locations::coordinates(loc))throw std::runtime_error("Invalid location coordinates.");return j;
}
}
