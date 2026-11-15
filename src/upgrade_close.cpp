#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <tlhelp32.h>
#include <set>
#include "upgrade_close.hpp"
// Installer-only helper. Requests normal window closure; never terminates processes.
static std::set<DWORD> targets;
static bool scan(){targets.clear();HANDLE h=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(h==INVALID_HANDLE_VALUE)return false;PROCESSENTRY32 e={};e.dwSize=sizeof(e);if(!Process32First(h,&e)){CloseHandle(h);return false;}do{if(upgrade_close::process(e.szExeFile))targets.insert(e.th32ProcessID);}while(Process32Next(h,&e));CloseHandle(h);return true;}
static BOOL CALLBACK closeWindow(HWND h,LPARAM){DWORD pid=0;GetWindowThreadProcessId(h,&pid);if(!targets.count(pid))return TRUE;char name[128]={};GetClassNameA(h,name,sizeof(name));if(!upgrade_close::window(name))return TRUE;
 // Let modal dialogs cancel/finalize before asking their disabled owner to close.
 if(IsWindowEnabled(h))PostMessage(h,WM_CLOSE,0,0);return TRUE;}
static BOOL CALLBACK topWindow(HWND h,LPARAM){closeWindow(h,0);EnumChildWindows(h,closeWindow,0);return TRUE;}
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){DWORD began=GetTickCount();do{if(!scan())return 2;if(targets.empty())return 0;EnumWindows(topWindow,0);Sleep(200);}while((DWORD)(GetTickCount()-began)<15000);if(!scan())return 2;return targets.empty()?0:1;}
