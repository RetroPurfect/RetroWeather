#pragma once
#include <windows.h>
#include <string>
#include <cstdint>
class VideoRecorder {
 struct Impl;Impl*impl=0;
public:
 struct Options{int width=0,height=0,volume=25;unsigned bitrate=900000;bool audio=true;};
 struct Status{bool ready=false,done=true,cleanupFailed=false;unsigned queued=0,written=0,dropped=0,sceneMask=0,pageMask[9]={};uint64_t bytes=0,lastFrame=0;long error=0;};
 static bool available();
 bool start(const std::wstring&path,const std::string&sounds,const Options&options);
 bool enqueue(const uint32_t*pixels,int width,int height,uint64_t frame,unsigned scene,uint64_t sceneStartFrame);
 void finish();Status status()const;~VideoRecorder();
 VideoRecorder()=default;VideoRecorder(const VideoRecorder&)=delete;VideoRecorder&operator=(const VideoRecorder&)=delete;
};
