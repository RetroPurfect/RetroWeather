#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <fstream>
#include <memory>
#include "pcm_wave.hpp"
class WeatherAudio {
 struct Voice{HWAVEOUT device=0;WAVEHDR header={};std::vector<char> pcm;bool prepared=false;
  ~Voice(){if(device){waveOutReset(device);if(prepared)waveOutUnprepareHeader(device,&header,sizeof(header));waveOutClose(device);}}
 };
 std::unique_ptr<Voice> voice,old;std::string current,wanted,directory;bool allowed=false,paused=false;DWORD lastAttempt=0;std::string failedTrack;DWORD fadeStart=0;double mix=1;
 void gain(Voice*v,double amount){if(v&&v->device){unsigned g=(unsigned)(65535.*std::max(0,std::min(100,volume))/100.*amount);volumeResult=waveOutSetVolume(v->device,g|(g<<16));}}
 std::unique_ptr<Voice> open(const std::string&path,bool hold){std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("Bundled weather sound is missing");auto size=f.tellg();if(size<44||size>2*1024*1024)throw std::runtime_error("Weather sound file size is invalid");f.seekg(0);std::vector<unsigned char>b((std::istreambuf_iterator<char>(f)),{});std::unique_ptr<Voice> v(new Voice);v->pcm=pcm_wave::decode(b);WAVEFORMATEX fmt={};fmt.wFormatTag=WAVE_FORMAT_PCM;fmt.nChannels=1;fmt.nSamplesPerSec=22050;fmt.wBitsPerSample=16;fmt.nBlockAlign=2;fmt.nAvgBytesPerSec=44100;if(waveOutOpen(&v->device,WAVE_MAPPER,&fmt,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR)throw std::runtime_error("Windows audio device is unavailable");gain(v.get(),0);if(hold)waveOutPause(v->device);v->header.lpData=v->pcm.data();v->header.dwBufferLength=(DWORD)v->pcm.size();v->header.dwFlags=WHDR_BEGINLOOP|WHDR_ENDLOOP;v->header.dwLoops=0xffffffffu;if(waveOutPrepareHeader(v->device,&v->header,sizeof(WAVEHDR))!=MMSYSERR_NOERROR)throw std::runtime_error("Could not prepare weather audio");v->prepared=true;if(waveOutWrite(v->device,&v->header,sizeof(WAVEHDR))!=MMSYSERR_NOERROR)throw std::runtime_error("Could not start weather audio");return v;}
public:
 MMRESULT volumeResult=MMSYSERR_NOERROR;int volume=25;bool enabled=true;std::string message="Silent on basic backgrounds";
 size_t bufferedBytes()const{return (voice?voice->pcm.capacity():0)+(old?old->pcm.capacity():0);}
 bool isActive()const{return !!voice;}bool isPaused()const{return !!voice&&paused;}bool transitioning()const{return !!old;}
 void stop(){old.reset();voice.reset();current.clear();mix=1;paused=false;}
 void request(const std::string&dir,const std::string&track,bool canPlay,bool hold){directory=dir;wanted=track;allowed=canPlay;if(!enabled||!allowed||wanted.empty()){stop();message=!enabled?"Off":!allowed?"Silent on basic backgrounds":"Waiting for weather";return;}if(current==wanted&&voice){setPaused(hold);return;}if(!voice&&failedTrack==wanted&&(DWORD)(GetTickCount()-lastAttempt)<30000)return;lastAttempt=GetTickCount();failedTrack=wanted;old.reset();old=std::move(voice);try{voice=open(directory+"\\"+wanted,hold);}catch(const std::exception&e){if(old){old.reset();try{voice=open(directory+"\\"+wanted,hold);}catch(const std::exception&retry){stop();message=retry.what();return;}}else{stop();message=e.what();return;}}failedTrack.clear();current=wanted;paused=hold;mix=old?0:1;fadeStart=GetTickCount();applyVolume();message.clear();}
 void applyVolume(){
 // Some waveOut drivers share the mixer volume across handles. Write the
 // incoming voice last so an outgoing zero cannot silence the active loop.
 gain(old.get(),1-mix);gain(voice.get(),mix);
 }
 void setEnabled(bool value,bool hold=false){enabled=value;failedTrack.clear();if(!value)stop();else request(directory,wanted,allowed,hold);}
 void setPaused(bool hold){if(paused==hold)return;bool ok=true;for(Voice*v:{voice.get(),old.get()})if(v&&v->device){MMRESULT r=hold?waveOutPause(v->device):waveOutRestart(v->device);if(r!=MMSYSERR_NOERROR)ok=false;}if(ok)paused=hold;else message="Weather audio pause/resume failed";}
 bool tick(){if(!old)return false;mix=std::min(1.,(DWORD)(GetTickCount()-fadeStart)/1600.);applyVolume();if(mix>=1){old.reset();applyVolume();}return !!old;}
 void poll(){} // Hardware loops the PCM buffer; no playback polling or decode thread.
 std::string title()const{if(!enabled)return "Off";if(!voice)return message;std::string n=current.substr(3,current.size()-7);for(char&c:n)if(c=='_')c=' ';return (paused?"Paused - ":"")+n;}
};
