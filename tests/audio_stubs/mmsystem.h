#pragma once
#include "windows.h"
typedef unsigned MMRESULT;
struct MockWave{bool paused=false;DWORD volume=0;};typedef MockWave* HWAVEOUT;
struct WAVEFORMATEX{WORD wFormatTag,nChannels;DWORD nSamplesPerSec,nAvgBytesPerSec;WORD nBlockAlign,wBitsPerSample,cbSize;};
struct WAVEHDR{LPSTR lpData;DWORD dwBufferLength,dwBytesRecorded;uintptr_t dwUser;DWORD dwFlags,dwLoops;WAVEHDR*lpNext;uintptr_t reserved;};
#define WAVE_FORMAT_PCM 1
#define WAVE_MAPPER ((UINT)-1)
#define CALLBACK_NULL 0
#define WHDR_BEGINLOOP 4
#define WHDR_ENDLOOP 8
#define MMSYSERR_NOERROR 0
inline int& openVoices(){static int n=0;return n;}
inline bool& singleDevice(){static bool b=false;return b;}
inline bool& sharedMixer(){static bool b=false;return b;}
inline DWORD& sharedVolume(){static DWORD value=0;return value;}
inline bool& resetVolumeOnClose(){static bool b=false;return b;}
inline MMRESULT waveOutOpen(HWAVEOUT*v,UINT,WAVEFORMATEX*f,uintptr_t,uintptr_t,DWORD){if(singleDevice()&&openVoices())return 4;if(f->nSamplesPerSec!=22050||f->nChannels!=1||f->wBitsPerSample!=16)return 1;*v=new MockWave;++openVoices();return 0;}
inline MMRESULT waveOutReset(HWAVEOUT){return 0;}
inline MMRESULT waveOutUnprepareHeader(HWAVEOUT,WAVEHDR*,unsigned){return 0;}
inline MMRESULT waveOutClose(HWAVEOUT v){if(sharedMixer()&&resetVolumeOnClose())sharedVolume()=0;delete v;--openVoices();return 0;}
inline MMRESULT waveOutSetVolume(HWAVEOUT v,DWORD g){v->volume=g;if(sharedMixer())sharedVolume()=g;return 0;}
inline MMRESULT waveOutPause(HWAVEOUT v){v->paused=true;return 0;}
inline MMRESULT waveOutRestart(HWAVEOUT v){v->paused=false;return 0;}
inline MMRESULT waveOutPrepareHeader(HWAVEOUT,WAVEHDR*,unsigned){return 0;}
inline MMRESULT waveOutWrite(HWAVEOUT v,WAVEHDR*h,unsigned){return h->dwLoops==0xffffffffu&&h->lpData&&h->dwBufferLength&&h->dwFlags==(WHDR_BEGINLOOP|WHDR_ENDLOOP)?0:1;}
