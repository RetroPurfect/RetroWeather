#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include "video_recorder.hpp"
#include "recording_plan.hpp"
#include "pcm_wave.hpp"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <objbase.h>
#include <deque>
#include <fstream>
#include <memory>
#include <cstring>
namespace {
template<class T>struct Com{T*p=0;~Com(){if(p)p->Release();}T*operator->(){return p;}void reset(){if(p)p->Release();p=0;}};
void check(HRESULT h){if(FAILED(h))throw h;}
HMODULE systemDll(const wchar_t*name){wchar_t dir[MAX_PATH];if(!GetSystemDirectoryW(dir,MAX_PATH))return 0;return LoadLibraryW((std::wstring(dir)+L"\\"+name).c_str());}
uint64_t fileSize(const std::wstring&path){WIN32_FILE_ATTRIBUTE_DATA d={};if(!GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&d))return 0;return (uint64_t(d.nFileSizeHigh)<<32)|d.nFileSizeLow;}
struct Api{
 HMODULE plat=0,rw=0;bool started=false;
 decltype(&MFCreateFile) createFile=0;decltype(&MFStartup) startup=0;decltype(&MFShutdown) shutdown=0;decltype(&MFCreateMediaType) type=0;decltype(&MFCreateAttributes) attributes=0;decltype(&MFCreateSample) sample=0;decltype(&MFCreateMemoryBuffer) buffer=0;decltype(&MFCreateSinkWriterFromURL) writer=0;
 void load(){plat=systemDll(L"mfplat.dll");rw=systemDll(L"mfreadwrite.dll");if(!plat||!rw)throw HRESULT(E_NOTIMPL);
 createFile=(decltype(createFile))GetProcAddress(plat,"MFCreateFile");startup=(decltype(startup))GetProcAddress(plat,"MFStartup");shutdown=(decltype(shutdown))GetProcAddress(plat,"MFShutdown");type=(decltype(type))GetProcAddress(plat,"MFCreateMediaType");attributes=(decltype(attributes))GetProcAddress(plat,"MFCreateAttributes");sample=(decltype(sample))GetProcAddress(plat,"MFCreateSample");buffer=(decltype(buffer))GetProcAddress(plat,"MFCreateMemoryBuffer");writer=(decltype(writer))GetProcAddress(rw,"MFCreateSinkWriterFromURL");if(!createFile||!startup||!shutdown||!type||!attributes||!sample||!buffer||!writer)throw HRESULT(E_NOTIMPL);check(startup(MF_VERSION,MFSTARTUP_FULL));started=true;}
 ~Api(){if(started)shutdown();if(rw)FreeLibrary(rw);if(plat)FreeLibrary(plat);}
 void write(IMFSinkWriter*s,DWORD stream,const void*data,DWORD size,LONGLONG at,LONGLONG duration){Com<IMFMediaBuffer>b;Com<IMFSample>p;check(buffer(size,&b.p));BYTE*dest=0;check(b->Lock(&dest,0,0));std::memcpy(dest,data,size);check(b->Unlock());check(b->SetCurrentLength(size));check(sample(&p.p));check(p->AddBuffer(b.p));check(p->SetSampleTime(at));check(p->SetSampleDuration(duration));check(s->WriteSample(stream,p.p));}
};
struct Packet{std::vector<uint32_t>rgb;uint64_t frame=0,start=0;unsigned scene=0;};
struct Lock{CRITICAL_SECTION*cs;Lock(CRITICAL_SECTION&c):cs(&c){EnterCriticalSection(cs);}~Lock(){LeaveCriticalSection(cs);}};
const char*tracks[]={"01_Clear_Day.wav","02_Clear_Night.wav","03_Cloudy.wav","03_Cloudy.wav","04_Rain.wav","04_Rain.wav","06_Thunderstorm.wav","05_Snow.wav","07_Fog.wav"};
}
struct VideoRecorder::Impl {
 mutable CRITICAL_SECTION cs;HANDLE event=0,thread=0;bool stopping=false;Status state;Options options;std::wstring path;std::string sounds;std::deque<Packet>queue;std::vector<int>xmap;int sourceWidth=0;
 Impl(){InitializeCriticalSection(&cs);event=CreateEventW(0,FALSE,FALSE,0);state.done=false;}
 ~Impl(){if(thread){finish();WaitForSingleObject(thread,INFINITE);CloseHandle(thread);}if(event)CloseHandle(event);DeleteCriticalSection(&cs);}
 void finish(){Lock l(cs);stopping=true;SetEvent(event);}
 static DWORD WINAPI entry(void*p){((Impl*)p)->run();return 0;}
 void run(){SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);HRESULT init=CoInitializeEx(0,COINIT_MULTITHREADED);HRESULT error=SUCCEEDED(init)?S_OK:init;bool valid=false;
 try{check(error);Api api;api.load();Com<IMFByteStream>file;Com<IMFSinkWriter>w;Com<IMFAttributes>attrs;check(api.attributes(&attrs.p,4));check(attrs->SetGUID(MF_TRANSCODE_CONTAINERTYPE,MFTranscodeContainerType_MPEG4));check(api.createFile(MF_ACCESSMODE_WRITE,MF_OPENMODE_DELETE_IF_EXIST,MF_FILEFLAGS_NONE,path.c_str(),&file.p));check(attrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS,TRUE));check(attrs->SetUINT32(MF_SINK_WRITER_DISABLE_THROTTLING,TRUE));check(api.writer(path.c_str(),file.p,attrs.p,&w.p));Com<IMFMediaType>out,in;check(api.type(&out.p));check(out->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Video));check(out->SetGUID(MF_MT_SUBTYPE,MFVideoFormat_H264));check(out->SetUINT32(MF_MT_AVG_BITRATE,options.bitrate));check(out->SetUINT32(MF_MT_MPEG2_PROFILE,66));check(out->SetUINT32(MF_MT_YUV_MATRIX,MFVideoTransferMatrix_BT601));check(out->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE,MFNominalRange_16_235));check(out->SetUINT32(MF_MT_INTERLACE_MODE,MFVideoInterlace_Progressive));check(MFSetAttributeSize(out.p,MF_MT_FRAME_SIZE,options.width,options.height));check(MFSetAttributeRatio(out.p,MF_MT_FRAME_RATE,10,1));check(MFSetAttributeRatio(out.p,MF_MT_PIXEL_ASPECT_RATIO,1,1));DWORD video=0,audio=0;check(w->AddStream(out.p,&video));check(api.type(&in.p));check(in->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Video));check(in->SetGUID(MF_MT_SUBTYPE,MFVideoFormat_NV12));check(in->SetUINT32(MF_MT_INTERLACE_MODE,MFVideoInterlace_Progressive));check(in->SetUINT32(MF_MT_DEFAULT_STRIDE,options.width));check(in->SetUINT32(MF_MT_YUV_MATRIX,MFVideoTransferMatrix_BT601));check(in->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE,MFNominalRange_16_235));check(MFSetAttributeSize(in.p,MF_MT_FRAME_SIZE,options.width,options.height));check(MFSetAttributeRatio(in.p,MF_MT_FRAME_RATE,10,1));check(MFSetAttributeRatio(in.p,MF_MT_PIXEL_ASPECT_RATIO,1,1));check(w->SetInputMediaType(video,in.p,0));
 std::vector<char>pcm[9];if(options.audio){out.reset();in.reset();check(api.type(&out.p));check(out->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Audio));check(out->SetGUID(MF_MT_SUBTYPE,MFAudioFormat_AAC));check(out->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,1));check(out->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,44100));check(out->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,16));check(out->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND,12000));check(out->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE,0));check(w->AddStream(out.p,&audio));check(api.type(&in.p));check(in->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Audio));check(in->SetGUID(MF_MT_SUBTYPE,MFAudioFormat_PCM));check(in->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,1));check(in->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,44100));check(in->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,16));check(in->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,2));check(in->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND,88200));check(w->SetInputMediaType(audio,in.p,0));for(int i=0;i<9;++i){std::ifstream f(sounds+"\\"+tracks[i],std::ios::binary);std::vector<unsigned char>b((std::istreambuf_iterator<char>(f)),{});pcm[i]=pcm_wave::decode(b);}}
 check(w->BeginWriting());{Lock l(cs);state.ready=true;}
 std::vector<unsigned char>nv12;std::vector<int16_t>samples(4410);uint64_t audioAt=0,previousStart=0;unsigned previousScene=0;
 for(;;){Packet p;{Lock l(cs);if(queue.empty()){if(stopping)break;}else{p=std::move(queue.front());queue.pop_front();}}if(p.rgb.empty()){WaitForSingleObject(event,100);continue;}
 recording_plan::nv12(p.rgb.data(),options.width,options.height,nv12);api.write(w.p,video,nv12.data(),(DWORD)nv12.size(),p.frame*1000000LL,1000000LL);
 if(options.audio){uint64_t end=(p.frame+1)*4410,boundary=p.start*4410;while(audioAt<end){unsigned count=(unsigned)std::min<uint64_t>(4410,end-audioAt);for(unsigned i=0;i<count;++i){uint64_t at=audioAt+i;bool current=at>=boundary;unsigned index=current?p.scene:previousScene;uint64_t beginning=(current?p.start:previousStart)*4410;int16_t value=recording_plan::pcm44100(pcm[index],at>=beginning?at-beginning:0,options.volume);samples[i]=value;}
 api.write(w.p,audio,samples.data(),count*2,audioAt*10000000LL/44100,count*10000000LL/44100);audioAt+=count;}previousScene=p.scene;previousStart=p.start;}
 uint64_t bytes=fileSize(path);QWORD streamLength=0;if(SUCCEEDED(file->GetLength(&streamLength)))bytes=streamLength;{Lock l(cs);++state.written;state.sceneMask|=1u<<p.scene;state.pageMask[p.scene]|=1u<<recording_plan::page((unsigned)(p.frame*100));state.lastFrame=p.frame;state.bytes=bytes;}

 }
 check(w->Finalize());w.reset();file.reset();valid=true;
 }catch(HRESULT hr){error=hr;}catch(...){error=E_FAIL;}
 if(SUCCEEDED(init))CoUninitialize();uint64_t bytes=fileSize(path);{Lock l(cs);if(FAILED(state.error)){error=state.error;valid=false;}if(!state.written||!bytes)valid=false;if(!valid){if(!DeleteFileW(path.c_str())&&GetLastError()!=ERROR_FILE_NOT_FOUND){state.cleanupFailed=true;bytes=fileSize(path);}else bytes=0;if(SUCCEEDED(error))error=E_FAIL;}state.bytes=bytes;state.error=error;state.ready=false;state.done=true;}
 }
};
bool VideoRecorder::available(){OSVERSIONINFOA v={};v.dwOSVersionInfoSize=sizeof(v);if(!GetVersionExA(&v)||v.dwMajorVersion<6||(v.dwMajorVersion==6&&v.dwMinorVersion<1))return false;wchar_t dir[MAX_PATH];if(!GetSystemDirectoryW(dir,MAX_PATH))return false;for(const wchar_t*n:{L"mfplat.dll",L"mfreadwrite.dll"})if(GetFileAttributesW((std::wstring(dir)+L"\\"+n).c_str())==INVALID_FILE_ATTRIBUTES)return false;return true;}
bool VideoRecorder::start(const std::wstring&path,const std::string&sounds,const Options&o){delete impl;impl=0;impl=new Impl;impl->path=path;impl->sounds=sounds;impl->options=o;if(!impl->event||o.width<2||o.height<2||(o.width&1)||(o.height&1)||o.width>3840||o.height>2160){impl->state.error=E_INVALIDARG;impl->state.done=true;return false;}impl->thread=CreateThread(0,0,Impl::entry,impl,0,0);if(!impl->thread){impl->state.error=HRESULT_FROM_WIN32(GetLastError());impl->state.done=true;return false;}return true;}
void VideoRecorder::finish(){if(impl)impl->finish();}
VideoRecorder::Status VideoRecorder::status()const{if(!impl)return Status();Lock l(impl->cs);return impl->state;}
VideoRecorder::~VideoRecorder(){delete impl;}
bool VideoRecorder::enqueue(const uint32_t*pixels,int width,int height,uint64_t frame,unsigned scene,uint64_t start){if(!impl||!pixels||width<1||height<1||scene>8)return false;Lock l(impl->cs);if(impl->stopping||!impl->state.ready)return false;if(impl->queue.size()>=2){++impl->state.dropped;return false;}try{if(impl->sourceWidth!=width){impl->xmap.resize(impl->options.width);for(int x=0;x<impl->options.width;++x)impl->xmap[x]=(int)((uint64_t)x*width/impl->options.width);impl->sourceWidth=width;}Packet p;p.frame=frame;p.scene=scene;p.start=start;p.rgb.resize((size_t)impl->options.width*impl->options.height);for(int y=0;y<impl->options.height;++y){const uint32_t*row=pixels+(size_t)((uint64_t)y*height/impl->options.height)*width;for(int x=0;x<impl->options.width;++x)p.rgb[(size_t)y*impl->options.width+x]=row[impl->xmap[x]]&0xffffff;}impl->queue.push_back(std::move(p));++impl->state.queued;SetEvent(impl->event);return true;}catch(...){impl->state.error=E_OUTOFMEMORY;impl->stopping=true;impl->queue.clear();SetEvent(impl->event);return false;}}
