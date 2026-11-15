#include "../src/weather_audio.hpp"
#include <cassert>
#include <iostream>
int main(int argc,char**argv){assert(argc==2);std::string dir=argv[1];WeatherAudio a;
a.request(dir,"01_Clear_Day.wav",false,false);assert(!a.isActive()&&openVoices()==0);
a.setEnabled(false);a.request(dir,"01_Clear_Day.wav",true,false);assert(!a.isActive());a.setEnabled(true,true);assert(a.isPaused()&&openVoices()==1);
a.setPaused(false);assert(a.isActive()&&!a.isPaused());a.request(dir,"02_Clear_Night.wav",true,false);assert(a.transitioning()&&openVoices()==2);clockMilliseconds()+=800;assert(a.tick());clockMilliseconds()+=800;assert(!a.tick()&&openVoices()==1);
a.request(dir,"04_Rain.wav",false,false);assert(openVoices()==0&&!a.isActive());a.request(dir,"04_Rain.wav",true,true);assert(a.isPaused());a.setEnabled(false);assert(!a.isActive()&&openVoices()==0&&a.bufferedBytes()==0);a.poll();assert(openVoices()==0);
a.setEnabled(true);assert(a.isActive());singleDevice()=true;a.request(dir,"05_Snow.wav",true,false);assert(a.isActive()&&openVoices()==1&&!a.transitioning());a.stop();assert(openVoices()==0);
a.request(dir,"missing.wav",true,false);assert(!a.isActive()&&openVoices()==0&&!a.message.empty());
singleDevice()=false;sharedMixer()=true;
for(bool reset:{false,true}){resetVolumeOnClose()=reset;WeatherAudio shared;shared.volume=50;shared.request(dir,"01_Clear_Day.wav",true,false);assert(sharedVolume()==0x7fff7fff);shared.request(dir,"04_Rain.wav",true,false);clockMilliseconds()+=800;assert(shared.tick());assert(sharedVolume()!=0);clockMilliseconds()+=800;assert(!shared.tick());assert(openVoices()==1&&sharedVolume()==0x7fff7fff);
shared.request(dir,"04_Rain.wav",true,false);assert(sharedVolume()==0x7fff7fff);shared.volume=25;shared.applyVolume();assert(sharedVolume()==0x3fff3fff);shared.request(dir,"06_Thunderstorm.wav",true,false);clockMilliseconds()+=1600;assert(!shared.tick()&&sharedVolume()==0x3fff3fff);shared.setPaused(true);assert(shared.isPaused());shared.setPaused(false);assert(!shared.isPaused());shared.stop();assert(openVoices()==0);}
sharedMixer()=false;resetVolumeOnClose()=false;
WeatherAudio toggle;toggle.enabled=false;toggle.request(dir,"04_Rain.wav",true,false);assert(toggle.bufferedBytes()==0&&openVoices()==0);for(int i=0;i<8;++i){toggle.setEnabled(true);assert(toggle.isActive()&&openVoices()==1&&toggle.bufferedBytes()>0);toggle.setEnabled(false);assert(!toggle.isActive()&&openVoices()==0&&toggle.bufferedBytes()==0);}

std::cout<<"PASS: basic themes silent; Off closes devices; paused starts; seamless hardware loops; two-voice crossfade cleanup; single-device fallback; missing-file cleanup; shared-mixer fade handoff and driver close resetting volume.\n";}
