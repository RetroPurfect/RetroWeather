#pragma once
#include <cstdint>
#include <algorithm>
typedef uint32_t DWORD;typedef uint16_t WORD;typedef unsigned UINT;typedef char* LPSTR;
inline DWORD& clockMilliseconds(){static DWORD t=100;return t;}
inline DWORD GetTickCount(){return clockMilliseconds();}
