#include "../src/weather_theme.hpp"
#include "../src/backup.hpp"
#include "../src/pcm_wave.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace weather_theme;
for(int c:{0})assert(classify(c)==Clear);for(int c:{1,2})assert(classify(c)==Partly);assert(classify(3)==Cloudy);
for(int c:{45,48})assert(classify(c)==Fog);for(int c:{51,53,55,56,57,61,63,65,66,67,80,81,82})assert(classify(c)==Rain);
for(int c:{71,73,75,77,85,86})assert(classify(c)==Snow);for(int c:{95,96,99})assert(classify(c)==Storm);for(int c:{-1,4,52,72,90,100})assert(classify(c)==Unknown);
assert(std::string(track(Clear,true))=="01_Clear_Day.wav"&&std::string(track(Clear,false))=="02_Clear_Night.wav");assert(std::string(track(Unknown,true)).empty());
assert(minutes("2026-11-14T07:12")==432&&minutes("2026-11-14T24:00")==-1&&minutes("bad")==-1);
assert(weather_theme::daylight(432,432,1000)&&!weather_theme::daylight(431,432,1000)&&!weather_theme::daylight(1000,432,1000));assert(weather_theme::daylight(30,1380,60)&&!weather_theme::daylight(300,1380,60));
json j={{"format","RetroWeatherSettings"},{"version",2},{"settings",{{"Background","4"},{"AudioEnabled","0"},{"AudioVolume","75"},{"PagesPaused","1"},{"SaverAudio","0"}}},{"locations",json::object()}};assert(backup::validate(j)==j);
auto old=j;old["version"]=1;old["settings"]={{"Background","7"},{"MusicVolume","25"},{"MusicEnabled","1"},{"MusicPaused","1"},{"SaverMusic","0"},{"ScenicAnimation","2"},{"MusicMode","2"},{"MusicTrack","old.wav"}};auto m=backup::validate(old);assert(m["version"]==2&&m["settings"]["Background"]=="4"&&m["settings"]["AudioVolume"]=="25"&&m["settings"]["PagesPaused"]=="1"&&!m["settings"].contains("MusicTrack"));
for(auto key:{"AudioVolume","Background"}){auto b=j;b["settings"][key]="999";bool threw=false;try{backup::validate(b);}catch(...){threw=true;}assert(threw);}
bool bad=false;try{pcm_wave::decode(std::vector<unsigned char>(44));}catch(...){bad=true;}assert(bad);
std::cout<<"PASS: WMO conditions, day/night sound selection, sunrise/sunset boundaries, removed-option migration, schema 2 roundtrip and schema 1 migration.\n";
}
