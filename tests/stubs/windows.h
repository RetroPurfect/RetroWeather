#pragma once
#include <cwchar>
#include <string>
using HWND=void*;using HANDLE=void*;
struct WIN32_FIND_DATAW{unsigned dwFileAttributes;wchar_t cFileName[260];};
#define FILE_ATTRIBUTE_DIRECTORY 16
#define INVALID_HANDLE_VALUE ((void*)-1)
#define CP_UTF8 65001
#define SW_SHOWNORMAL 1
inline unsigned GetTickCount(){return 1;}inline unsigned GetCurrentProcessId(){return 2;}
inline HANDLE FindFirstFileW(const wchar_t*,WIN32_FIND_DATAW*f){std::wcscpy(f->cFileName,L"one.wav");f->dwFileAttributes=0;return (void*)1;}
inline bool FindNextFileW(HANDLE,WIN32_FIND_DATAW*){return false;}inline void FindClose(HANDLE){}
inline int WideCharToMultiByte(unsigned,unsigned,const wchar_t*,int,char*b,int,void*,void*){if(b){b[0]='x';b[1]=0;}return 2;}
inline void ShellExecuteW(HWND,const wchar_t*,const wchar_t*,void*,void*,int){}
