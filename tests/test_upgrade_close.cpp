#include "../src/upgrade_close.hpp"
#include "../src/saver.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace upgrade_close;assert(process("RetroWeather.exe")&&process("RETROWEATHER.SCR")&&process("RetroWeatherXP.exe"));for(auto n:{"RetroWeatherClose.exe","notepad.exe","RetroWeather_Setup_v1.1_Test19.exe","OtherRetroWeather.exe"})assert(!process(n));assert(window("RetroWeatherPreview")&&window("RetroWeatherSaver")&&window("#32770"));assert(!window("Button")&&!window("Edit"));assert(saver::key(false,'M',0)==saver::ToggleAudio);assert(saver::key(false,'M',1u<<30)==saver::IgnoreRepeat);assert(saver::key(true,'M',0)==saver::Exit);assert(saver::key(false,27,0)==saver::Exit);assert(saver::key(false,'A',0)==saver::Exit);std::cout<<"PASS: installer target process/window filtering and screensaver M-key toggle, held-key suppression and other-key exits.\n";}
