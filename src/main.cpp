#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include "refinements.hpp"
#include "saver.hpp"
#include "scenic.hpp"
#include "render_plan.hpp"
#include "graphics_quality.hpp"
#include "raster_cache.hpp"
#include "frame_budget.hpp"
#include "smooth_scale.hpp"
#include "render_profile.hpp"
#include "memory_audit.hpp"
#include "diagnostic_export.hpp"
#include "cloud_mask.hpp"
#include "cloud_sprite.hpp"
#include "backbuffer.hpp"
#include "software_blend.hpp"
#include "dirty_tiles.hpp"
#include "backup.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <set>
#include <cstring>
#include <psapi.h>
#include "weather.hpp"
#include "crt_profile.hpp"
#include "weather_pages.hpp"
#include "calendar.hpp"
#include "page_decoration.hpp"
#include "weather_audio.hpp"
#include "benchmark_weather.hpp"
#include "benchmark.hpp"
#include "video_recorder.hpp"
#include "recording_plan.hpp"
#include "sky_motion.hpp"
#include "precipitation.hpp"
#include "weather_theme.hpp"
#include "presentation.hpp"
#include "features.hpp"
#include "locations.hpp"
#include "options.hpp"
static bool benchmarkActive=false;static HWND benchmarkWindow=0;static std::string lastBenchmarkReport,recordingSceneName;static std::wstring benchmarkExportStem;static void benchmarkFrame(double);static void runBenchmark(HWND);
static bool settingsOperation=false,restoringSettings=false;static bool saverRun=false,saverPreview=false,powerPaused=false,resumeRefresh=false;static HWND previewParent=0;static POINT saverMouse={};static DWORD saverStarted=0;static const char*buildDate="2026-11-15";
static HWND mainWindow,locationWindow,zipEdit,applyButton;static HANDLE stopEvent,worker;
static std::string base,folder,zip,status="Weather > Add or Change Location (Ctrl+L)";
static benchmark_weather::Session previewSession;static bool&previewMode=previewSession.active;static int&previewIndex=previewSession.index;static int&previewBackground=previewSession.originalBackground;static void exitWeatherPreview();
static json data;static bool busy=false,full=false;static RECT saved;static int ticks=0,retryAt=0,retryFailures=0;static HMENU menus;static int background=0;
static int crtPart=0;static features::Cycle cycle;static features::PageConfig pageConfig;static int duration=12,pauseDuration=0;static bool clock24=false;
static WeatherAudio audio;static json zoneTable,moonTable,illuminationTable;
static bool restoreScreensaver=false;
static bool startFullscreen=false,crtMode=false,sessionUpdated=false,lastUpdateFailed=false;
static bool celsius=false;static int windUnit=0;static std::array<int,9> pageDurations={{0,0,0,0,0,0,0,0,0}};
static RECT normalWindow={};static bool placementReady=false,windowMaximized=false,wasMaximized=false,changingWindowMode=false;

static int safeMargin=0,textPercent=100,aspectMode=0;static double drawX=1,drawY=1,drawOffsetX=0,drawOffsetY=0;static int drawOriginY=0;static presentation::Frame frame;
static json locationMeta=json::object(),pendingLocation,selectedLocation;static unsigned locationSerial=0;static std::set<std::string> metadataAttempted;
static std::string lastError,lastRequestError;static DWORD lastHelperExit=0;static int lastHttp=0;
static void showLocationDialog();static void refreshLocationDialog(const std::string&message="",bool useCached=true);
static void writeFile(const std::string&,const std::string&);static void saveLocations();
static std::vector<std::string> savedZips;
static int64_t foregroundEpoch=0;static bool contentDirty=true,dirtyClipActive=false;static std::vector<rendering::Rect> dirtyRegions;static std::vector<uint32_t> previousScene;static int previousSceneWidth=0,previousSceneHeight=0;static int lastDirtyPercent=100;static bool lastPartialFrame=false;static std::map<std::pair<int,int>,HFONT> fontCache;
static bool frameDirty=true,animationPending=false,legacyRendering=false;static int lastPaintMs=0,lastAnimationDelay=0;static BackBuffer pictureBuffer,sceneBuffer,skyBuffer,foregroundScratch;static int sceneCacheBackground=-1,sceneCacheWidth=0,sceneCacheHeight=0,sceneCacheFrameWidth=0,sceneCacheFrameHeight=0;
static int drawingPixelOffsetY=0;
static int dpiOverride=0;static int sceneLevel=2;static scenic::State scene;static DWORD sceneTick=0;
static double lastPrecipitationMs=0,lastCloudSkyMs=0,lastSceneMs=0,lastScaleMs=0,lastOverlayMs=0,lastPresentMs=0,lastCacheMs=0;static bool scaleComparisonActive=false;static rendering::RenderProfile renderProfile;static rendering::SmoothScale smoothScaler;static rendering::ScaleChoice scaleChoice;static std::string scaleBackend="Not used";static bool sceneCold=false;static unsigned sceneCacheBuilds=0;
static int foregroundLeft=0,foregroundTop=0,foregroundRight=0,foregroundBottom=0;
static sky_motion::Offset displayShift(){return sky_motion::fit(sky_motion::orbit(ticks),foregroundLeft,foregroundTop,foregroundRight,foregroundBottom,pictureBuffer.width,pictureBuffer.height);}
static rendering::Quality graphicsQuality;static rendering::RasterCache foregroundCache;static rendering::FrameMeter frameMeter;static std::vector<weather_graphics::CloudMask> cloudMasks;static weather_graphics::CloudSprite cloudSprites[6];static rendering::RasterCache sunRadiance;static int celestialKey=-1;
static int sceneTier(){return crtMode?0:graphicsQuality.tier;}
static const char* sceneQualityName(){return crtMode?"Lightweight (CRT profile)":graphicsQuality.name();}
static int sceneLimit(){return crtMode?640:graphicsQuality.limit();}
static void clearCloudSprites(){for(auto&sprite:cloudSprites){sprite.raster.clear();sprite.width=sprite.height=0;sprite.style=sprite.variant=-1;}}
static size_t cloudCacheBytes(){size_t total=0;for(auto&sprite:cloudSprites)total+=sprite.raster.bytes();return total;}
static unsigned foregroundBuilds=0;static bool foregroundReady=false,frameRebuiltForeground=false;
static void updateMenus();
static void repaint(HWND h,BOOL erase=FALSE){contentDirty=true;frameDirty=true;InvalidateRect(h,0,erase);}
static int windowDpi(HWND h){typedef UINT(WINAPI*GetDpi)(HWND);static GetDpi fn=(GetDpi)GetProcAddress(GetModuleHandleA("user32.dll"),"GetDpiForWindow");if(fn&&h){UINT n=fn(h);if(n)return (int)n;}if(h&&dpiOverride>0)return dpiOverride;HDC dc=GetDC(h);int n=GetDeviceCaps(dc,LOGPIXELSY);ReleaseDC(h,dc);return n>0?n:96;}
static std::wstring wide(const std::string&s){if(s.empty())return L"";int code=CP_UTF8;int n=MultiByteToWideChar(code,MB_ERR_INVALID_CHARS,s.c_str(),-1,0,0);if(!n){code=CP_ACP;n=MultiByteToWideChar(code,0,s.c_str(),-1,0,0);}if(!n)return L"";std::vector<wchar_t>v(n);MultiByteToWideChar(code,0,s.c_str(),-1,v.data(),n);return std::wstring(v.data());}
static std::string utf8(const std::wstring&s){if(s.empty())return "";int n=WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,0,0,0,0);if(!n)return "";std::vector<char>v(n);WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,v.data(),n,0,0);return std::string(v.data());}
static void setText(HWND h,const std::string&s){SetWindowTextW(h,wide(s).c_str());}
static std::string controlText(HWND h){int n=GetWindowTextLengthW(h);std::vector<wchar_t>v(n+1);GetWindowTextW(h,v.data(),n+1);return utf8(v.data());}
static void enableDpiAwareness(){HMODULE user=GetModuleHandleA("user32.dll");typedef BOOL(WINAPI*SetContext)(HANDLE);SetContext context=(SetContext)GetProcAddress(user,"SetProcessDpiAwarenessContext");if(context&&context((HANDLE)(INT_PTR)-4))return;
 char path[MAX_PATH];GetSystemDirectoryA(path,MAX_PATH);std::string dll=std::string(path)+"\\shcore.dll";HMODULE shcore=LoadLibraryA(dll.c_str());if(shcore){typedef HRESULT(WINAPI*SetAwareness)(int);SetAwareness aware=(SetAwareness)GetProcAddress(shcore,"SetProcessDpiAwareness");HRESULT r=aware?aware(2):E_FAIL;FreeLibrary(shcore);if(SUCCEEDED(r))return;}
 typedef BOOL(WINAPI*SetAware)();SetAware aware=(SetAware)GetProcAddress(user,"SetProcessDPIAware");if(aware)aware();
}
static presentation::CursorIdle cursorIdle;static POINT lastCursor={0,0};static bool menuActive=false;
static presentation::Viewport viewport(HWND h){RECT r;GetClientRect(h,&r);return presentation::aspectFit(r.right,r.bottom,crt_profile::aspect(crtMode,aspectMode),crt_profile::margin(crtMode,safeMargin));}
static void wakeCursor(){cursorIdle.reset(GetTickCount());GetCursorPos(&lastCursor);SetCursor(LoadCursor(0,IDC_ARROW));}
static bool mayHideCursor(POINT position){
 if(!full||menuActive||IsIconic(mainWindow)||GetForegroundWindow()!=mainWindow)return false;
 POINT local=position;ScreenToClient(mainWindow,&local);RECT r;GetClientRect(mainWindow,&r);
 return PtInRect(&r,local)&&WindowFromPoint(position)==mainWindow;
}
static void pollCursor(){POINT p;if(!GetCursorPos(&p))return;bool moved=p.x!=lastCursor.x||p.y!=lastCursor.y;
 bool previous=cursorIdle.hidden;cursorIdle.update(GetTickCount(),moved,mayHideCursor(p));lastCursor=p;
 if((previous!=cursorIdle.hidden||moved)&&WindowFromPoint(p)==mainWindow)SetCursor(cursorIdle.hidden?0:LoadCursor(0,IDC_ARROW));
}
static int64_t nowEpoch(){FILETIME ft;GetSystemTimeAsFileTime(&ft);ULARGE_INTEGER n;n.LowPart=ft.dwLowDateTime;n.HighPart=ft.dwHighDateTime;return (int64_t)(n.QuadPart/10000000ULL)-11644473600LL;}
static std::string ini(){return folder+"\\settings.ini";}
static int setting(const char*key,int fallback,int lo,int hi){return std::max(lo,std::min(hi,(int)GetPrivateProfileIntA("Settings",key,fallback,ini().c_str())));}
static void saveSetting(const char*key,int v){WritePrivateProfileStringA("Settings",key,std::to_string(v).c_str(),ini().c_str());}
static void savePrefs(){if(benchmarkActive||saverRun||saverPreview||restoringSettings)return;saveSetting("PageSeconds",duration);saveSetting("PauseSeconds",pauseDuration);saveSetting("Clock24",clock24);saveSetting("AudioVolume",audio.volume);saveSetting("AudioEnabled",audio.enabled);saveSetting("Background",previewMode?previewBackground:background);WritePrivateProfileStringA("Settings","ScenicTransparency",0,ini().c_str());saveSetting("PagesPaused",cycle.paused);for(int i=0;i<features::pageCount;++i)saveSetting(("PageOverride"+std::to_string(i)).c_str(),pageDurations[i]);}
static std::string stringSetting(const char*key,const char*fallback=""){char buffer[2048];GetPrivateProfileStringA("Settings",key,fallback,buffer,sizeof(buffer),ini().c_str());return buffer;}
static void saveExtraPrefs(){if(benchmarkActive||saverRun||saverPreview||restoringSettings)return;saveSetting("AspectMode",aspectMode);saveSetting("StartFullscreen",startFullscreen);WritePrivateProfileStringA("Settings","HideControls",0,ini().c_str());saveSetting("Celsius",celsius);saveSetting("WindUnit",windUnit);saveSetting("CrtMode",crtMode);saveSetting("SafeMargin",safeMargin);saveSetting("TextPercent",textPercent);saveSetting("PageMask",pageConfig.mask);WritePrivateProfileStringA("Settings","PageOrder",pageConfig.serialize().c_str(),ini().c_str());WritePrivateProfileStringA("Settings","SavedZips",features::joinZips(savedZips).c_str(),ini().c_str());}
static void rememberZip(const std::string&code){if(!validZip(code))return;if(std::find(savedZips.begin(),savedZips.end(),code)==savedZips.end()){if(savedZips.size()>=10)savedZips.pop_back();savedZips.push_back(code);}saveExtraPrefs();}
static void saveLocations(){try{writeFile(folder+"\\locations.new",locationMeta.dump());if(!MoveFileExA((folder+"\\locations.new").c_str(),(folder+"\\locations.json").c_str(),MOVEFILE_REPLACE_EXISTING))throw std::runtime_error("write");}catch(...){lastError="Location names could not be saved to your profile.";}}
static std::string locationLabel(const std::string&code){if(locationMeta.contains(code)&&locations::valid(locationMeta[code]))return locations::label(locationMeta[code]);return code+" - City/state pending lookup, United States";}
static bool isAutostart(){HKEY key;bool enabled=false;if(RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){DWORD type=0,bytes=0;enabled=RegQueryValueExA(key,"Retro Weather",0,&type,0,&bytes)==ERROR_SUCCESS&&type==REG_SZ&&bytes>1;RegCloseKey(key);}return enabled;}
static bool setAutostart(bool enabled){HKEY key;DWORD disp;if(RegCreateKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,0,0,KEY_SET_VALUE,0,&key,&disp)!=ERROR_SUCCESS)return false;LONG result;
 if(enabled){char exe[MAX_PATH];GetModuleFileNameA(0,exe,MAX_PATH);std::string command="\""+std::string(exe)+"\"";result=RegSetValueExA(key,"Retro Weather",0,REG_SZ,(const BYTE*)command.c_str(),(DWORD)command.size()+1);}else{result=RegDeleteValueA(key,"Retro Weather");if(result==ERROR_FILE_NOT_FOUND)result=ERROR_SUCCESS;}RegCloseKey(key);return result==ERROR_SUCCESS;
}
static const char* backgroundNames[]={"Basic Blue","Basic Red","Basic Green","Basic Purple","Weather Determined"};
static HMENU makeBackgrounds(){HMENU m=CreatePopupMenu();for(int i=0;i<5;++i)AppendMenuA(m,MF_STRING|(background==i?MF_CHECKED:0),350+i,backgroundNames[i]);return m;}
static void togglePresentation(){cycle.paused=!cycle.paused;audio.setPaused(cycle.paused);}
static void appendLocations(HMENU menu){AppendMenuA(menu,MF_STRING,170,"&Add or Change Location...\tCtrl+L");AppendMenuA(menu,MF_STRING,171,"&Remove Current Location");AppendMenuA(menu,MF_STRING,103,"Refresh Weather Now\tF5");AppendMenuA(menu,MF_SEPARATOR,0,0);for(size_t i=0;i<savedZips.size();++i){std::string label=locationLabel(savedZips[i])+(savedZips[i]==zip?" (current)":"");AppendMenuW(menu,MF_STRING|(busy?MF_GRAYED:0),180+i,wide(locations::menuText(label)).c_str());}if(savedZips.empty())AppendMenuA(menu,MF_STRING|MF_GRAYED,0,"No saved locations");}
static void trackPlacement(HWND h){if(saverRun||saverPreview)return;if(!placementReady||changingWindowMode||full||IsIconic(h))return;windowMaximized=IsZoomed(h)!=FALSE;if(!windowMaximized)GetWindowRect(h,&normalWindow);}
struct MonitorSearch{std::string device;HMONITOR found=0;};
static BOOL CALLBACK findMonitor(HMONITOR mon,HDC,LPRECT,LPARAM context){MonitorSearch*s=(MonitorSearch*)context;MONITORINFOEXA mi={};mi.cbSize=sizeof(mi);if(GetMonitorInfoA(mon,(MONITORINFO*)&mi)&&s->device==mi.szDevice){s->found=mon;return FALSE;}return TRUE;}
static int monitorDpi(HMONITOR mon){char dir[MAX_PATH];GetSystemDirectoryA(dir,MAX_PATH);HMODULE sh=LoadLibraryA((std::string(dir)+"\\shcore.dll").c_str());int dpi=windowDpi(0);if(sh){typedef HRESULT(WINAPI*GetDpi)(HMONITOR,int,UINT*,UINT*);GetDpi get=(GetDpi)GetProcAddress(sh,"GetDpiForMonitor");UINT x=0,y=0;if(get&&SUCCEEDED(get(mon,0,&x,&y))&&x)dpi=(int)x;FreeLibrary(sh);}return dpi;}
static void restorePlacement(HWND h){
 bool existing=setting("WindowSaved",0,0,1)!=0;MonitorSearch search;search.device=startFullscreen?stringSetting("FullscreenMonitor",stringSetting("WindowMonitor").c_str()):stringSetting("WindowMonitor");if(existing)EnumDisplayMonitors(0,0,findMonitor,(LPARAM)&search);
 HMONITOR mon=search.found?search.found:MonitorFromWindow(h,MONITOR_DEFAULTTOPRIMARY);MONITORINFOEXA mi={};mi.cbSize=sizeof(mi);if(!GetMonitorInfoA(mon,(MONITORINFO*)&mi)){mi.rcWork.left=mi.rcWork.top=0;mi.rcWork.right=GetSystemMetrics(SM_CXSCREEN);mi.rcWork.bottom=GetSystemMetrics(SM_CYSCREEN);}int dpi=monitorDpi(mon);RECT r;GetWindowRect(h,&r);options::Rect target={(int)r.left,(int)r.top,(int)(r.right-r.left),(int)(r.bottom-r.top)};
 if(existing){target.width=MulDiv(setting("WindowWidth96",656,160,10000),dpi,96);target.height=MulDiv(setting("WindowHeight96",526,120,10000),dpi,96);target.x=mi.rcWork.left+MulDiv(setting("WindowX96",0,-20000,20000),dpi,96);target.y=mi.rcWork.top+MulDiv(setting("WindowY96",0,-20000,20000),dpi,96);if(!search.found){target.x=mi.rcWork.left;target.y=mi.rcWork.top;}}
 target=options::fitWindow(target,{(int)mi.rcWork.left,(int)mi.rcWork.top,(int)(mi.rcWork.right-mi.rcWork.left),(int)(mi.rcWork.bottom-mi.rcWork.top)});
 changingWindowMode=true;SetWindowPos(h,0,target.x,target.y,target.width,target.height,SWP_NOZORDER|SWP_NOACTIVATE);GetWindowRect(h,&normalWindow);windowMaximized=existing&&setting("WindowMaximized",0,0,1);changingWindowMode=false;placementReady=true;
}
static void savePlacement(HWND h){if(saverRun||saverPreview||restoringSettings)return;trackPlacement(h);if(!placementReady)return;MONITORINFOEXA mi={};mi.cbSize=sizeof(mi);HMONITOR mon=MonitorFromRect(&normalWindow,MONITOR_DEFAULTTONEAREST);if(!GetMonitorInfoA(mon,(MONITORINFO*)&mi))return;int dpi=monitorDpi(mon);saveSetting("WindowSaved",1);saveSetting("WindowWidth96",MulDiv(normalWindow.right-normalWindow.left,96,dpi));saveSetting("WindowHeight96",MulDiv(normalWindow.bottom-normalWindow.top,96,dpi));saveSetting("WindowX96",MulDiv(normalWindow.left-mi.rcWork.left,96,dpi));saveSetting("WindowY96",MulDiv(normalWindow.top-mi.rcWork.top,96,dpi));saveSetting("WindowMaximized",windowMaximized);WritePrivateProfileStringA("Settings","WindowMonitor",mi.szDevice,ini().c_str());MONITORINFOEXA current={};current.cbSize=sizeof(current);if(GetMonitorInfoA(MonitorFromWindow(h,MONITOR_DEFAULTTONEAREST),(MONITORINFO*)&current))WritePrivateProfileStringA("Settings","FullscreenMonitor",current.szDevice,ini().c_str());}
static std::string zoneName(){return !data.empty()&&data.contains("timezone")&&data["timezone"].is_string()?data["timezone"].get<std::string>():"PC local time (set ZIP)";}
static int zoneOffset(int64_t utc){
 std::string name=zoneName();
 if(!zoneTable.empty()&&utc>=zoneTable.value("start",0LL)&&utc<zoneTable.value("end",0LL)&&zoneTable["zones"].contains(name)){
  const auto& rows=zoneTable["zones"][name];int off=rows[0][1].get<int>();for(const auto&r:rows){if(r[0].get<int64_t>()>utc)break;off=r[1].get<int>();}return off;
 }
 if(!data.empty()&&data.contains("utc_offset_seconds")&&data["utc_offset_seconds"].is_number_integer())return data["utc_offset_seconds"].get<int>();
 SYSTEMTIME local;GetLocalTime(&local);return (int)(calendar::epoch(local.wYear,local.wMonth,local.wDay,local.wHour,local.wMinute,local.wSecond)-nowEpoch());
}
static calendar::Date localDate(int64_t utc){return calendar::fromEpoch(utc+zoneOffset(utc));}
static weather_theme::Condition weatherCondition(){try{if(!data.empty())return weather_theme::classify(data.at("current").at("weather_code").get<int>());}catch(...){}return weather_theme::Unknown;}
static bool weatherDay(){if(previewMode)return previewIndex!=1;auto d=localDate(nowEpoch());int minute=d.hour*60+d.minute;try{const auto&daily=data.at("daily");for(size_t i=0;i<daily.at("time").size();++i){std::string day=daily["time"][i];char date[16];std::sprintf(date,"%04d-%02d-%02d",d.year,d.month,d.day);if(day==date){int rise=weather_theme::minutes(daily.at("sunrise").at(i).get<std::string>()),set=weather_theme::minutes(daily.at("sunset").at(i).get<std::string>());if(rise>=0&&set>=0)return weather_theme::daylight(minute,rise,set);}}if(data.at("current").contains("is_day"))return data["current"]["is_day"].get<int>()!=0;}catch(...){}return minute>=360&&minute<1080;}
static std::vector<uint32_t> transitionPixels;static int transitionWidth=0,transitionHeight=0;static DWORD transitionStarted=0;
static weather_theme::Condition sceneCondition=weather_theme::Unknown;static bool sceneDay=true;
static void releaseTransition(){std::vector<uint32_t>().swap(transitionPixels);transitionWidth=transitionHeight=0;}
static void syncWeatherAudio(){auto next=weatherCondition();bool day=weatherDay();if(next!=sceneCondition||day!=sceneDay){if(background==4&&sceneBuffer.pixels){GdiFlush();transitionPixels.assign(sceneBuffer.pixels,sceneBuffer.pixels+(size_t)sceneBuffer.width*sceneBuffer.height);transitionWidth=sceneBuffer.width;transitionHeight=sceneBuffer.height;transitionStarted=GetTickCount();}clearCloudSprites();sceneCondition=next;sceneDay=day;sceneCacheBackground=-1;if(mainWindow)repaint(mainWindow);}audio.request(base+"\\Sounds",weather_theme::track(sceneCondition,sceneDay),background==4&&!saverPreview,cycle.paused||powerPaused);if(mainWindow&&audio.transitioning())SetTimer(mainWindow,5,100,0);}
static void inhibitScreensaver(){
 // Recover a session-only change if an earlier run crashed.
 if(setting("RestoreScreensaver",0,0,1)){SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE,TRUE,0,0);saveSetting("RestoreScreensaver",0);}
 BOOL enabled=FALSE;if(SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE,0,&enabled,0)&&enabled){saveSetting("RestoreScreensaver",1);if(SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE,FALSE,0,0))restoreScreensaver=true;else saveSetting("RestoreScreensaver",0);}
 SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED|ES_SYSTEM_REQUIRED);
}
static void releaseScreensaver(){if(restoreScreensaver){SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE,TRUE,0,0);saveSetting("RestoreScreensaver",0);}SetThreadExecutionState(ES_CONTINUOUS);}
struct Result {json value,location;std::string error,zip;int kind=0;unsigned serial=0;bool notify=false;DWORD helperExit=0;int http=0;};
static std::string readFile(const std::string& p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});}
static void writeFile(const std::string&p,const std::string&s){std::ofstream f(p,std::ios::binary);if(!(f<<s))throw std::runtime_error("Could not save the file in your profile.");}
static bool fetch(const std::string&url,const std::string&out,Result&r){
 std::string exe=base+"\\network\\curl.exe",cert=base+"\\network\\curl-ca-bundle.crt",headers=out+".headers";
 std::string cmd="\""+exe+"\" --disable --silent --show-error --fail --location --proto =https --proto-redir =https --connect-timeout 10 --max-time 25 --max-filesize 1048576 --cacert \""+cert+"\" --dump-header \""+headers+"\" --output \""+out+"\" \""+url+"\"";
 STARTUPINFOA si={};si.cb=sizeof(si);PROCESS_INFORMATION pi={};
 if(!CreateProcessA(exe.c_str(),&cmd[0],0,0,FALSE,CREATE_NO_WINDOW,0,base.c_str(),&si,&pi)){r.helperExit=GetLastError();r.error="HTTPS helper could not start (Windows error "+std::to_string(r.helperExit)+").";return false;}
 HANDLE waits[]={stopEvent,pi.hProcess};DWORD result=WaitForMultipleObjects(2,waits,FALSE,30000),exit=1;
 if(result!=WAIT_OBJECT_0+1){TerminateProcess(pi.hProcess,1);WaitForSingleObject(pi.hProcess,2000);exit=28;}else GetExitCodeProcess(pi.hProcess,&exit);
 CloseHandle(pi.hThread);CloseHandle(pi.hProcess);r.helperExit=exit;r.http=0;
 std::istringstream lines(readFile(headers));std::string line;while(std::getline(lines,line))if(line.find("HTTP/")==0)std::sscanf(line.c_str(),"HTTP/%*s %d",&r.http);DeleteFileA(headers.c_str());
 if(exit==0)return true;
 if(r.http==404&&url.find("zippopotam")!=std::string::npos)r.error="That ZIP code was not found. Check its five digits and try again.";
 else if(exit==6||exit==7)r.error="Cannot reach the weather/location service. Check your connection and try again.";
 else if(exit==28)r.error="The request timed out. Try again when the connection improves.";
 else if(exit==60||exit==77)r.error="Secure connection failed. Check the PC date and certificate bundle.";
 else if(exit==0xC000001Du)r.error="The HTTPS helper reported an illegal instruction. Check CPU compatibility.";
 else if(r.http>=400)r.error="The service returned HTTP "+std::to_string(r.http)+". Try again later.";
 else r.error="Weather/location request failed (helper exit "+std::to_string(exit)+"). Check connection and PC date.";
 return false;
}
static DWORD WINAPI loadWeather(void* ptr){
 Result*r=(Result*)ptr;std::string temp=folder+(saverRun?"\\saver-response.tmp":"\\response.tmp");
 try{
  if(!locations::coordinates(r->location)){
   if(!fetch("https://api.zippopotam.us/us/"+r->zip,temp,*r))throw std::runtime_error(r->error);
   r->location=locations::parse(r->zip,json::parse(readFile(temp)));
  }
  if(r->kind==0){
   double lat=r->location["latitude"].get<double>(),lon=r->location["longitude"].get<double>();
   std::string url="https://api.open-meteo.com/v1/forecast?latitude="+std::to_string(lat)+"&longitude="+std::to_string(lon)+"&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m,wind_direction_10m,wind_gusts_10m,cloud_cover,pressure_msl,is_day&hourly=temperature_2m,dew_point_2m,visibility,weather_code,precipitation_probability,precipitation,wind_speed_10m,is_day&daily=sunrise,sunset,daylight_duration,weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,precipitation_sum,snowfall_sum,uv_index_max&temperature_unit=fahrenheit&wind_speed_unit=mph&timezone=auto&forecast_days=7";
   if(!fetch(url,temp,*r))throw std::runtime_error(r->error);
   json w=json::parse(readFile(temp));validateWeather(w,7);w["zip"]=r->zip;w["city"]=r->location["city"];w["state"]=r->location.value("state_code",r->location["state"].get<std::string>());w["state_name"]=r->location["state"];w["country"]=r->location["country"];w["fetched"]=nowEpoch();r->value=w;
  }
 }catch(const std::exception&e){if(r->error.empty())r->error=std::string("Could not read the service response: ")+e.what();}
 DeleteFileA(temp.c_str());if(!PostMessage(mainWindow,WM_APP+1,0,(LPARAM)r))delete r;return 0;
}
static bool startRequest(const std::string&code,int kind=0,const json&known=json(),bool notify=false){
 if(busy)return false;if(!validZip(code)){lastError="Enter exactly five digits, including any leading zero.";if(notify)MessageBoxA(mainWindow,lastError.c_str(),"US ZIP code",MB_OK|MB_ICONEXCLAMATION);return false;}
 if(worker){CloseHandle(worker);worker=0;}Result*r=new Result;r->zip=code;r->kind=kind;r->serial=locationSerial;r->location=known;r->notify=notify;busy=true;
 if(kind==0)status="Updating weather...";
 worker=CreateThread(0,0,loadWeather,r,0,0);if(!worker){delete r;busy=false;lastError="Could not start the background update.";status=lastError;lastRequestError=lastError;if(kind==0){lastUpdateFailed=true;retryAt=ticks+refinements::retryDelay(++retryFailures);}if(notify)MessageBoxA(mainWindow,lastError.c_str(),"Weather Update",MB_OK|MB_ICONEXCLAMATION);}refreshLocationDialog();repaint(mainWindow);return worker!=0;
}
static void startLoad(bool notify=true){if(previewMode)exitWeatherPreview();if(validZip(zip))startRequest(zip,0,locationMeta.value(zip,json()),notify);else showLocationDialog();}
static int pixelX(int logical){return (int)std::round(logical*drawX+drawOffsetX);}
static int pixelWidth(int logical){return (int)std::round(logical*drawX);}
static int pixelY(int logical){return (int)std::round((logical+drawOriginY)*drawY+drawOffsetY)-drawingPixelOffsetY;}
static void clearFonts(){for(auto item:fontCache)DeleteObject(item.second);fontCache.clear();}
static HFONT drawingFont(int logical,bool forceBold=false){int height=std::max(1,(int)std::round(logical*drawY)),quality=crtMode||legacyRendering?NONANTIALIASED_QUALITY:ANTIALIASED_QUALITY;int weight=!forceBold&&!legacyRendering&&!crtMode&&logical<=18?FW_MEDIUM:FW_BOLD;auto key=std::make_pair(height,quality*1000+weight);auto found=fontCache.find(key);if(found!=fontCache.end())return found->second;HFONT font=CreateFontW(-height,0,0,0,weight,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,quality,DEFAULT_PITCH,L"Arial");if(font)fontCache[key]=font;return font;}

static int text(HDC dc,int x,int y,int size,const std::string&s,COLORREF color=RGB(255,246,215),int maxWidth=-1,int maxHeight=-1,bool shadowed=true,bool bold=false,bool centered=false){
 int width=maxWidth>=0?maxWidth:frame.width-x-24;if(width<=0)return 0;std::wstring value=wide(s);int fontSize=crtMode?std::max(18,size):std::max(10,(int)std::round(size*textPercent/100.));int minimum=std::min(fontSize,crtMode?18:14);HFONT font=0;HGDIOBJ previous=0;SIZE extent={};TEXTMETRICW tm={};
 for(;;){font=drawingFont(fontSize,bold);previous=SelectObject(dc,font);GetTextExtentPoint32W(dc,value.c_str(),(int)value.size(),&extent);GetTextMetricsW(dc,&tm);bool fits=extent.cx<=pixelWidth(width)&&(maxHeight<0||tm.tmHeight<=maxHeight*drawY);if(fits||fontSize<=minimum)break;SelectObject(dc,previous);--fontSize;}
 int height=maxHeight>=0?pixelY(y+maxHeight)-pixelY(y):std::max(1,(int)tm.tmHeight+2);RECT rect={pixelX(x),pixelY(y),pixelX(x+width),pixelY(y)+height};if(dc==foregroundScratch.dc&&(rect.bottom<=0||rect.top>=foregroundScratch.height)){SelectObject(dc,previous);return std::min(width,(int)std::ceil(extent.cx/drawX));}int savedText=SaveDC(dc);IntersectClipRect(dc,rect.left,rect.top,rect.right,rect.bottom);UINT flags=DT_SINGLELINE|DT_TOP|DT_NOPREFIX|DT_END_ELLIPSIS|(centered?DT_CENTER:0);if(shadowed){RECT shadow=rect;int offset=std::max(1,(int)std::round(drawY*.8));OffsetRect(&shadow,offset,offset);SetTextColor(dc,RGB(9,15,24));DrawTextW(dc,value.c_str(),(int)value.size(),&shadow,flags);}SetTextColor(dc,color);DrawTextW(dc,value.c_str(),(int)value.size(),&rect,flags);RestoreDC(dc,savedText);SelectObject(dc,previous);return std::min(width,(int)std::ceil(extent.cx/drawX));
}
static int measureText(HDC dc,int size,const std::string&s){HFONT font=drawingFont(crtMode?std::max(18,size):std::max(10,(int)std::round(size*textPercent/100.)));HGDIOBJ old=SelectObject(dc,font);std::wstring value=wide(s);SIZE extent={};GetTextExtentPoint32W(dc,value.c_str(),(int)value.size(),&extent);SelectObject(dc,old);return (int)std::ceil(extent.cx/drawX);}
static void box(HDC dc,int x,int y,int w,int h,COLORREF c){RECT r={pixelX(x),pixelY(y),pixelX(x+w),pixelY(y+h)};if(dc==foregroundScratch.dc&&(r.bottom<=0||r.top>=foregroundScratch.height))return;HBRUSH b=CreateSolidBrush(c);FillRect(dc,&r,b);DeleteObject(b);}
static std::string windSpeed(const json&j){if(!j.contains("wind_speed_10m")||!j["wind_speed_10m"].is_number())return "--";return options::rounded(options::wind(j["wind_speed_10m"].get<double>(),windUnit))+" "+options::windLabel(windUnit);}
static void iconEllipse(HDC dc,int x,int y,int w,int h,COLORREF fill){if(dc==foregroundScratch.dc&&(pixelY(y+h)<=0||pixelY(y)>=foregroundScratch.height))return;HBRUSH brush=CreateSolidBrush(fill);HGDIOBJ oldBrush=SelectObject(dc,brush),oldPen=SelectObject(dc,GetStockObject(NULL_PEN));Ellipse(dc,pixelX(x),pixelY(y),pixelX(x+w),pixelY(y+h));SelectObject(dc,oldPen);SelectObject(dc,oldBrush);DeleteObject(brush);}
static void iconLine(HDC dc,int x1,int y1,int x2,int y2,COLORREF color,int thickness=1){if(dc==foregroundScratch.dc&&(std::max(pixelY(y1),pixelY(y2))+thickness*drawY<0||std::min(pixelY(y1),pixelY(y2))-thickness*drawY>=foregroundScratch.height))return;HPEN pen=CreatePen(PS_SOLID,std::max(1,(int)std::round(thickness*drawY)),color);HGDIOBJ old=SelectObject(dc,pen);MoveToEx(dc,pixelX(x1),pixelY(y1),0);LineTo(dc,pixelX(x2),pixelY(y2));SelectObject(dc,old);DeleteObject(pen);}
static bool isScenic(){return background==4;}

static int64_t renderFrequency(){static int64_t rate=0;if(!rate){LARGE_INTEGER f;rate=QueryPerformanceFrequency(&f)&&f.QuadPart>0?f.QuadPart:1000;}return rate;}
static int64_t renderNow(){LARGE_INTEGER n;if(renderFrequency()!=1000&&QueryPerformanceCounter(&n))return n.QuadPart;return GetTickCount();}
static void syncSceneTimer(HWND h){frameMeter.reset();KillTimer(h,4);animationPending=false;sceneTick=GetTickCount();lastAnimationDelay=rendering::budgetDelay(0,legacyRendering);if(sceneLevel&&isScenic()&&!saverPreview&&!IsIconic(h))SetTimer(h,4,lastAnimationDelay,0);}
static void exitWeatherPreview(){if(!previewMode)return;previewSession.leave(data,background);syncWeatherAudio();syncSceneTimer(mainWindow);repaint(mainWindow);}
static void useWeatherPreview(int index){if(!previewSession.enter(index,data,background,nowEpoch()))return;cycle.page=0;syncWeatherAudio();syncSceneTimer(mainWindow);repaint(mainWindow);}
static void finishAnimationFrame(HWND h,int64_t started,bool recomposed=true){GdiFlush();if(recomposed){lastPaintMs=(int)std::ceil(std::max(0.,(renderNow()-started)*1000./renderFrequency()));if(animationPending&&!sceneCold&&isScenic()&&!saverPreview&&!crtMode&&graphicsQuality.sample(lastSceneMs)){sceneCacheBackground=-1;}}if(benchmarkActive&&recomposed)benchmarkFrame(std::max(0.,(renderNow()-started)*1000./renderFrequency()));if(animationPending){frameMeter.frame(GetTickCount());animationPending=false;lastAnimationDelay=rendering::budgetDelay(lastPaintMs,legacyRendering);if(sceneLevel&&isScenic()&&!saverPreview&&!IsIconic(h))SetTimer(h,4,lastAnimationDelay,0);}}
static COLORREF colorMix(COLORREF a,COLORREF b,double t){t=std::max(0.,std::min(1.,t));return RGB(GetRValue(a)+(int)((GetRValue(b)-GetRValue(a))*t),GetGValue(a)+(int)((GetGValue(b)-GetGValue(a))*t),GetBValue(a)+(int)((GetBValue(b)-GetBValue(a))*t));}
static void blendRect(HDC dc,int x,int y,int w,int h,COLORREF color,int opacity){if(opacity>=255){box(dc,x,y,w,h,color);return;}if(opacity<=0)return;BackBuffer*target=dc==pictureBuffer.dc?&pictureBuffer:dc==sceneBuffer.dc?&sceneBuffer:0;if(!target||!target->pixels){box(dc,x,y,w,h,color);return;}GdiFlush();uint32_t rgb=(GetRValue(color)<<16)|(GetGValue(color)<<8)|GetBValue(color);rendering::Rect panel={pixelX(x),pixelY(y),pixelX(x+w),pixelY(y+h)};if(dirtyClipActive&&target==&pictureBuffer){for(auto r:dirtyRegions){r=rendering::intersect(r,panel);if(!rendering::empty(r))rendering::blendFlat(target->pixels,target->width,target->height,r.left,r.top,r.right,r.bottom,rgb,(unsigned)(opacity*256/255));}}else rendering::blendFlat(target->pixels,target->width,target->height,panel.left,panel.top,panel.right,panel.bottom,rgb,(unsigned)(opacity*256/255));}

static int64_t moonEpoch(const json&r);
static double backgroundMoonFraction(){int64_t utc=nowEpoch();if(moonTable.is_array())for(size_t i=0;i+1<moonTable.size();++i){int64_t at=moonEpoch(moonTable[i]),next=moonEpoch(moonTable[i+1]);if(at<=utc&&utc<next){std::string phase=moonTable[i].value("phase",std::string());double q=phase=="New Moon"?0:phase=="First Quarter"?.25:phase=="Full Moon"?.5:.75;return q+.25*double(utc-at)/double(next-at);}}return .5;}
static void sceneBackground(HDC dc){
 sceneCold=false;
 weather_theme::Condition kind=sceneCondition;bool day=sceneDay;double t=saverPreview?0:scene.time;
 auto X=[](double x){return (int)std::round(x*frame.width/640.);};auto Y=[](double y){return (int)std::round(y*frame.height/480.);};
 auto ellipse=[&](double x,double y,double w,double h,COLORREF c){iconEllipse(dc,X(x+w/2)-Y(w)/2,Y(y),Y(w),Y(h),c);};
 auto line=[&](double x,double y,double a,double b,COLORREF c){iconLine(dc,X(x),Y(y),X(a),Y(b),c);};
 bool wet=kind==weather_theme::Rain||kind==weather_theme::Storm,gray=wet||kind==weather_theme::Cloudy||kind==weather_theme::Snow||kind==weather_theme::Fog;
 COLORREF top=day?(gray?RGB(48,69,88):RGB(17,86,153)):(gray?RGB(15,25,42):RGB(7,16,40));
 COLORREF bottom=day?(gray?RGB(115,139,153):RGB(128,196,221)):(gray?RGB(47,62,77):RGB(35,58,88));
 static int skyKind=-1,skyDay=-1,skyW=0,skyH=0;bool skyNew=!skyBuffer.dc||skyBuffer.width!=pixelX(frame.width)||skyBuffer.height!=pixelY(frame.height);bool skyOK=skyBuffer.ensure(dc,pixelX(frame.width),pixelY(frame.height));
 if(skyOK){if(skyNew||skyKind!=(int)kind||skyDay!=(int)day||skyW!=skyBuffer.width||skyH!=skyBuffer.height){sceneCold=true;++sceneCacheBuilds;HDC output=dc;dc=skyBuffer.dc;
 TRIVERTEX vertices[2]={{0,0,(COLOR16)(GetRValue(top)<<8),(COLOR16)(GetGValue(top)<<8),(COLOR16)(GetBValue(top)<<8),0},{pixelX(frame.width),pixelY(frame.height),(COLOR16)(GetRValue(bottom)<<8),(COLOR16)(GetGValue(bottom)<<8),(COLOR16)(GetBValue(bottom)<<8),0}};GRADIENT_RECT gradient={0,1};GradientFill(dc,vertices,2,&gradient,1,GRADIENT_FILL_RECT_V);

 dc=output;skyKind=kind;skyDay=day;skyW=skyBuffer.width;skyH=skyBuffer.height;}BitBlt(dc,0,0,skyBuffer.width,skyBuffer.height,skyBuffer.dc,0,0,SRCCOPY);}else{box(dc,0,0,frame.width,frame.height,top);}
 if(!day&&!gray){for(int i=0;i<45;++i){double a=.04+.86*std::pow((1+std::sin(t*(.5+i%4*.13)+i*1.8))*.5,2);ellipse(9+(i*139)%620+3*std::sin(t/190.+i),12+(i*83)%350+2*std::sin(t/260.+i*.7),i%9?1.5:2.5,i%9?1.5:2.5,colorMix(top,RGB(222,235,250),a));}}
 if(!gray&&kind!=weather_theme::Unknown){BackBuffer*target=dc==sceneBuffer.dc?&sceneBuffer:dc==pictureBuffer.dc?&pictureBuffer:0;if(target&&target->pixels){auto position=sky_motion::celestial(t,frame.width);int radius=std::max(8,pixelY(Y(day?80:35)));double phase=backgroundMoonFraction();int key=day?10000:(int)(phase*1000);auto&sprite=sunRadiance;
 if(sprite.width!=radius*2||key!=celestialKey){sceneCold=true;++sceneCacheBuilds;celestialKey=key;sprite.build(radius*2,radius*2,[&](int x,int y){double nx=(x-radius+.5)/radius,ny=(y-radius+.5)/radius;uint32_t value=day?sky_motion::sun(nx,ny):sky_motion::moon(nx,ny,phase);return rendering::premultiply(value&0xffffff,value>>24);});}
 GdiFlush();sprite.composite(target->pixels,target->width,target->height,pixelX(position.x)-radius,pixelY(Y(position.y))-radius,{0,0,target->width,target->height},255,!legacyRendering);}}

 // Only transform sprites when geometry/lighting changes; steady frames alpha-blit cached runs.
 if(kind!=weather_theme::Clear&&kind!=weather_theme::Unknown){auto&clouds=cloudMasks;auto*sprites=cloudSprites;if(clouds.empty())for(int i=0;i<4;++i)clouds.emplace_back(320,160,i);
 BackBuffer* target=dc==sceneBuffer.dc?&sceneBuffer:dc==pictureBuffer.dc?&pictureBuffer:0;
 if(target&&target->pixels){GdiFlush();bool covered=gray&&kind!=weather_theme::Fog;int count=kind==weather_theme::Partly?4:kind==weather_theme::Fog?6:8;
 COLORREF shade=day?(wet?RGB(42,58,77):RGB(85,110,133)):RGB(20,33,50),lit=day?(wet?RGB(153,174,186):RGB(225,231,227)):RGB(99,122,145);
 uint32_t dark=(GetRValue(shade)<<16)|(GetGValue(shade)<<8)|GetBValue(shade),light=(GetRValue(lit)<<16)|(GetGValue(lit)<<8)|GetBValue(lit);
 for(int i=0;i<count;++i){double depth=.55+(i%3)*.18,w=kind==weather_theme::Fog?650:covered?760+(i%2)*60:220+(i%3)*48,h=kind==weather_theme::Fog?150:covered?220+(i%2)*30:100+(i%3)*22;
 double cx,cy;if(covered){int row=i/2;double phase=std::fmod(t*(1.6+row*.4)+row*83,1280.);cx=phase-1280+(i%2)*640;cy=-110+row*135;}else{cx=std::fmod(i*197+t*(1.4+i%3)*depth+1400.,1400.)-700;cy=kind==weather_theme::Fog?-40+i*95:-30+(i*67)%285;}
 // Overcast cloud rows overlap continuously; draw wrap copies to avoid gaps at the edges.
 int copies=covered?3:1;for(int wrap=0;wrap<copies;++wrap){double x=cx+(covered?(wrap-1)*1280.:0);int left=pixelX(X(x)),up=pixelY(Y(cy));int width=std::max(1,pixelX(X(w))),height=std::max(1,pixelY(Y(h)));
 unsigned strength=kind==weather_theme::Fog?38:covered?(i<2?48:72):(i<2?65:90);int style=(int)kind*10+day;
 int slot=i<2?i+4:i%4;if(sprites[slot].prepare(clouds[i%4],width,height,style,i%4,dark,light,strength)){sceneCold=true;++sceneCacheBuilds;}sprites[slot].raster.composite(target->pixels,target->width,target->height,left,up,{0,0,target->width,target->height},255,!legacyRendering);
 }}}}
 GdiFlush();int64_t precipitationStarted=renderNow();
 if(wet){int weatherCode=-1;try{weatherCode=data.at("current").value("weather_code",-1);}catch(...){}bool drizzle=weatherCode>=51&&weatherCode<=57;int count=sceneTier()>=2?180:sceneTier()==0?55:100;if(drizzle)count=count*3/5;BackBuffer*rainTarget=dc==sceneBuffer.dc?&sceneBuffer:dc==pictureBuffer.dc?&pictureBuffer:0;for(int i=0;i<count;++i){auto drop=benchmark_weather::rain(i,t,drizzle);double length=drizzle?3+drop.depth*4:10+drop.depth*12;COLORREF color=colorMix(bottom,day?RGB(183,208,225):RGB(133,166,195),(drizzle?.2:.3)+drop.depth*.4);double endX=drop.x-(drizzle?1.5:4)*drop.depth,endY=drop.y+length;if(rainTarget&&rainTarget->pixels){uint32_t rgb=(GetRValue(color)<<16)|(GetGValue(color)<<8)|GetBValue(color);precipitation::streak(rainTarget->pixels,rainTarget->width,rainTarget->height,pixelX(X(drop.x)),pixelY(Y(drop.y)),pixelX(X(endX)),pixelY(Y(endY)),rgb,std::max(1.,drawY));}else line(drop.x,drop.y,endX,endY,color);}}
 if(kind==weather_theme::Snow){for(int i=0;i<(sceneTier()>=2?100:sceneTier()==0?32:60);++i){double z=.45+(i%7)/10.,y=std::fmod(i*83+t*19*z,500.)-10,x=std::fmod(i*149+std::sin(t*.38+i)*13+t*3*z+660,660.)-10;double radius=sceneTier()>=2?1.1+(i%5)*.7:2+i%3;if(sceneTier()>=2&&i%5==4)ellipse(x-.7,y-.7,radius+1.4,radius+1.4,colorMix(bottom,RGB(216,232,242),.25));ellipse(x,y,radius,radius,colorMix(bottom,RGB(228,239,246),.45+z*.4));}}
 GdiFlush();lastPrecipitationMs=(renderNow()-precipitationStarted)*1000./renderFrequency();
 if(kind==weather_theme::Storm&&!saverPreview){double age=t-scene.flashStart;if(age>=0&&age<scene.flashLength){double alpha=std::pow(std::sin(scenic::pi*age/scene.flashLength),.65);blendRect(dc,0,0,frame.width,frame.height,RGB(206,224,248),(int)(36*alpha));double x=70+std::fmod(scene.flashStart*127,480.),y=8;for(int k=0;k<8;++k){double nx=x+std::sin(scene.flashStart*7+k*4.8)*24,ny=y+32;COLORREF c=colorMix(top,RGB(228,237,255),std::max(.4,alpha));if(sceneTier()>=2){line(x-1,y,nx-1,ny,colorMix(top,RGB(135,172,232),.55));line(x+2,y,nx+2,ny,colorMix(top,RGB(135,172,232),.55));}line(x,y,nx,ny,c);line(x+1,y,nx+1,ny,c);if(k==2||k==4){line(nx,ny,nx+27,ny+19,c);line(nx+27,ny+19,nx+15,ny+40,c);}x=nx;y=ny;}}}
 if(dc==sceneBuffer.dc&&!transitionPixels.empty()){DWORD elapsed=GetTickCount()-transitionStarted;if(elapsed>=1800||transitionWidth!=sceneBuffer.width||transitionHeight!=sceneBuffer.height)releaseTransition();else{GdiFlush();unsigned amount=(unsigned)(elapsed*256/1800);for(size_t i=0;i<transitionPixels.size();++i){uint32_t a=transitionPixels[i],b=sceneBuffer.pixels[i];sceneBuffer.pixels[i]=((((a&0xff00ff)*(256-amount)+(b&0xff00ff)*amount)>>8)&0xff00ff)|((((a&0xff00)*(256-amount)+(b&0xff00)*amount)>>8)&0xff00);}}}

}
static void renderScene(HDC dc){int64_t started=renderNow();lastPrecipitationMs=0;sceneBackground(dc);GdiFlush();lastSceneMs=std::max(0.,(renderNow()-started)*1000./renderFrequency());lastCloudSkyMs=std::max(0.,lastSceneMs-lastPrecipitationMs);}
static void scaleScene(HDC mem,int renderWidth,int renderHeight,int sourceWidth,int sourceHeight){
 scaleComparisonActive=false;scaleBackend="Native copy";int method=-1;bool comparison=false;
 if(!legacyRendering&&(sourceWidth!=renderWidth||sourceHeight!=renderHeight)){if(scaleChoice.sw!=sourceWidth||scaleChoice.sh!=sourceHeight||scaleChoice.dw!=renderWidth||scaleChoice.dh!=renderHeight)renderProfile.reset();scaleComparisonActive=true;smoothScaler.setVectorAllowed(!legacyRendering);scaleChoice.configure(sourceWidth,sourceHeight,renderWidth,renderHeight);smoothScaler.prepare(sourceWidth,sourceHeight,renderWidth,renderHeight);method=scaleChoice.method();comparison=!sceneCold&&!frameRebuiltForeground;}
 GdiFlush();int64_t started=renderNow();
 if(legacyRendering){scaleBackend="Legacy nearest";rendering::restoreNearest(pictureBuffer.pixels,renderWidth,renderHeight,sceneBuffer.pixels,sourceWidth,sourceHeight,{{0,0,renderWidth,renderHeight}});}
 else if(sourceWidth==renderWidth&&sourceHeight==renderHeight)BitBlt(mem,0,0,renderWidth,renderHeight,sceneBuffer.dc,0,0,SRCCOPY);
 else if(method==1&&smoothScaler.draw(sceneBuffer.pixels,pictureBuffer.pixels))scaleBackend=smoothScaler.simdActive()?"Cached bilinear (SSE2)":"Cached bilinear (scalar)";
 else{method=0;scaleBackend="GDI HALFTONE";SetStretchBltMode(mem,HALFTONE);SetBrushOrgEx(mem,0,0,0);StretchBlt(mem,0,0,renderWidth,renderHeight,sceneBuffer.dc,0,0,sourceWidth,sourceHeight,SRCCOPY);}
 GdiFlush();lastScaleMs=std::max(0.,(renderNow()-started)*1000./renderFrequency());if(comparison)scaleChoice.record(method,lastScaleMs);
}
static void drawBackground(HDC dc){if(isScenic()){renderScene(dc);return;}
 COLORREF top[]={RGB(10,30,90),RGB(58,15,33),RGB(9,42,32),RGB(39,19,65),RGB(30,47,76),RGB(9,18,47),RGB(34,56,78),RGB(28,43,61)};
 COLORREF bottom[]={RGB(10,78,186),RGB(116,35,47),RGB(17,91,63),RGB(82,42,120),RGB(159,82,64),RGB(22,39,73),RGB(71,99,122),RGB(56,78,96)};
 COLORREF a=top[background],b=bottom[background];for(int y=0;y<frame.height;y+=4){double t=y/(double)frame.height;box(dc,0,y,frame.width,std::min(4,frame.height-y),RGB(GetRValue(a)+(int)((GetRValue(b)-GetRValue(a))*t),GetGValue(a)+(int)((GetGValue(b)-GetGValue(a))*t),GetBValue(a)+(int)((GetBValue(b)-GetBValue(a))*t)));}

}
static void weatherIcon(HDC dc,int x,int y,int size,int code,bool night=false){if(dc==foregroundScratch.dc&&(pixelY(y+size)+drawY*3<0||pixelY(y)-drawY*3>=foregroundScratch.height))return;options::Icon kind=options::icon(code);auto X=[&](int n){return x+n*size/48;};auto Y=[&](int n){return y+n*size/48;};auto S=[&](int n){return std::max(1,n*size/48);};
 if(kind==options::Unknown){text(dc,x+size/4,y,size*3/4,"?",RGB(190,204,220),size*3/4,size);return;}
 if(kind==options::Sun||kind==options::PartCloud){int cx=kind==options::Sun?24:18,cy=kind==options::Sun?24:15,r=kind==options::Sun?12:9;if(night){int savedDC=SaveDC(dc);HRGN outer=CreateEllipticRgn(pixelX(X(cx-r)),pixelY(Y(cy-r)),pixelX(X(cx+r)),pixelY(Y(cy+r))),cut=CreateEllipticRgn(pixelX(X(cx-r+7)),pixelY(Y(cy-r-3)),pixelX(X(cx+r+7)),pixelY(Y(cy+r-3)));CombineRgn(outer,outer,cut,RGN_DIFF);ExtSelectClipRgn(dc,outer,RGN_AND);iconEllipse(dc,X(cx-r),Y(cy-r),S(r*2),S(r*2),RGB(235,239,223));RestoreDC(dc,savedDC);DeleteObject(outer);DeleteObject(cut);}else{for(int i=0;i<8;++i){double angle=i*3.141592653589793/4;iconLine(dc,X(cx+(int)((r+3)*std::cos(angle))),Y(cy+(int)((r+3)*std::sin(angle))),X(cx+(int)((r+7)*std::cos(angle))),Y(cy+(int)((r+7)*std::sin(angle))),RGB(245,174,42),S(2));}iconEllipse(dc,X(cx-r),Y(cy-r),S(r*2),S(r*2),RGB(255,210,85));}if(kind==options::Sun)return;}
 COLORREF cloud=kind==options::Storm?RGB(153,181,208):RGB(233,245,255);iconEllipse(dc,X(8),Y(22),S(17),S(15),cloud);iconEllipse(dc,X(17),Y(14),S(22),S(23),cloud);iconEllipse(dc,X(30),Y(23),S(15),S(14),cloud);box(dc,X(16),Y(28),S(22),S(9),cloud);
 if(kind==options::Rain){bool icy=code==56||code==57||code==66||code==67;int count=code==65||code==82?4:3;for(int i=0;i<count;++i){int xx=12+i*9;iconLine(dc,X(xx),Y(39),X(xx-3),Y(code>=51&&code<=55?43:47),icy?RGB(193,226,255):RGB(105,205,255),S(code==65||code==82?3:2));if(icy)iconLine(dc,X(xx-4),Y(46),X(xx+2),Y(46),RGB(240,250,255));}}
 if(kind==options::Snow){for(int i=0;i<3;++i){int cx=14+i*12;iconLine(dc,X(cx-3),Y(43),X(cx+4),Y(43),RGB(245,250,255));iconLine(dc,X(cx),Y(40),X(cx),Y(47),RGB(245,250,255));}}
 if(kind==options::Fog){for(int i=0;i<3;++i)iconLine(dc,X(5+i*2),Y(35+i*5),X(44-i*2),Y(35+i*5),RGB(169,204,223),S(2));}
 if(kind==options::Storm&&(code==96||code==99)){for(int i=0;i<3;++i)iconEllipse(dc,X(10+i*12),Y(42),S(3),S(3),RGB(224,246,255));}
 if(kind==options::Storm){POINT p[]={{pixelX(X(25)),pixelY(Y(30))},{pixelX(X(18)),pixelY(Y(41))},{pixelX(X(25)),pixelY(Y(40))},{pixelX(X(22)),pixelY(Y(48))},{pixelX(X(34)),pixelY(Y(35))},{pixelX(X(27)),pixelY(Y(36))}};HBRUSH b=CreateSolidBrush(RGB(255,219,69));HGDIOBJ oldB=SelectObject(dc,b),oldP=SelectObject(dc,GetStockObject(NULL_PEN));Polygon(dc,p,6);SelectObject(dc,oldB);SelectObject(dc,oldP);DeleteObject(b);}
}
static int64_t moonEpoch(const json&r){std::string t=r.value("time",std::string("00:00"));int h=0,m=0;std::sscanf(t.c_str(),"%d:%d",&h,&m);return calendar::epoch(r.at("year").get<int>(),r.at("month").get<int>(),r.at("day").get<int>(),h,m);}
static COLORREF holidayAccent(page_decoration::Symbol type){return type==page_decoration::Tree?RGB(158,234,175):type==page_decoration::Heart?RGB(255,174,192):type==page_decoration::Leaf?RGB(255,196,128):type==page_decoration::Firework?RGB(255,218,135):RGB(161,221,255);}
static void dateBadge(HDC dc,int x,int y,calendar::Date date,COLORREF accent){
 box(dc,x,y,50,54,RGB(239,245,245));box(dc,x,y,50,18,accent);
 text(dc,x+3,y+4,10,page_decoration::month(date.month),RGB(18,40,65),44,12,false,true,true);
 text(dc,x+4,y+21,25,std::to_string(date.day),RGB(18,40,65),42,30,false,true,true);
 iconLine(dc,x+11,y-3,x+11,y+4,RGB(239,245,245),2);iconLine(dc,x+39,y-3,x+39,y+4,RGB(239,245,245),2);
}
static void holidaySymbol(HDC dc,int x,int y,int size,page_decoration::Symbol type,COLORREF color){
 int cx=x+size/2,cy=y+size/2,r=size/2-2;
 if(type==page_decoration::Tree){for(int k=0;k<3;++k){int yy=y+2+k*size/5,rr=4+k*size/7;iconLine(dc,cx,yy,cx-rr,yy+size/3,color,2);iconLine(dc,cx-rr,yy+size/3,cx+rr,yy+size/3,color,2);iconLine(dc,cx+rr,yy+size/3,cx,yy,color,2);}iconLine(dc,cx,y+size*3/4,cx,y+size,color,2);}
 else if(type==page_decoration::Heart){static const auto outline=page_decoration::heart();POINT points[64];for(size_t i=0;i<outline.size();++i){points[i].x=pixelX(x+(int)std::round(outline[i].x*size));points[i].y=pixelY(y+(int)std::round(outline[i].y*size));}HBRUSH brush=CreateSolidBrush(color);HGDIOBJ oldBrush=SelectObject(dc,brush),oldPen=SelectObject(dc,GetStockObject(NULL_PEN));Polygon(dc,points,64);SelectObject(dc,oldPen);SelectObject(dc,oldBrush);DeleteObject(brush);}
 else if(type==page_decoration::Leaf){iconLine(dc,cx,y+size,cx,y+4,color,2);for(int k=0;k<3;++k){int yy=y+6+k*size/5;iconLine(dc,cx,yy+6,x+3+k*2,yy,color,2);iconLine(dc,cx,yy+6,x+size-3-k*2,yy,color,2);}}
 else if(type==page_decoration::Firework){for(int k=0;k<8;++k){double a=k*scenic::pi/4;iconLine(dc,cx+(int)(r*.4*std::cos(a)),cy+(int)(r*.4*std::sin(a)),cx+(int)(r*std::cos(a)),cy+(int)(r*std::sin(a)),color,2);}iconEllipse(dc,cx-2,cy-2,4,4,RGB(255,246,215));}
 else{for(int k=0;k<10;++k){double a=-scenic::pi/2+k*scenic::pi/5,b=a+scenic::pi/5;double ra=k%2?r*.42:r,rb=k%2?r:r*.42;iconLine(dc,cx+(int)(ra*std::cos(a)),cy+(int)(ra*std::sin(a)),cx+(int)(rb*std::cos(b)),cy+(int)(rb*std::sin(b)),color,2);}}
}
static void drawExtras(HDC dc,int page){
 int64_t utc=foregroundEpoch?foregroundEpoch:nowEpoch();calendar::Date local=localDate(utc);
 if(page==3){
  double fraction=0;bool covered=false;std::string nextNew="Unavailable",nextFull="Unavailable";
  if(moonTable.is_array()){
   for(size_t i=0;i<moonTable.size();++i){const auto&r=moonTable[i];int64_t at=moonEpoch(r);std::string phase=r.value("phase",std::string());
    if(at>utc&&phase=="New Moon"&&nextNew=="Unavailable")nextNew=calendar::dateString(localDate(at));
    if(at>utc&&phase=="Full Moon"&&nextFull=="Unavailable")nextFull=calendar::dateString(localDate(at));
    if(i+1<moonTable.size()&&at<=utc&&moonEpoch(moonTable[i+1])>utc){
     double q=phase=="New Moon"?0:phase=="First Quarter"?.25:phase=="Full Moon"?.5:.75;
     fraction=q+.25*(double)(utc-at)/(double)(moonEpoch(moonTable[i+1])-at);covered=true;
    }
   }
  }
  if(!covered){text(dc,32,140,23,"Moon data available for 2020-2050.");text(dc,32,180,19,"Check the PC date or update the data pack.");return;}
  const char*names[]={"New Moon","Waxing Crescent","First Quarter","Waxing Gibbous","Full Moon","Waning Gibbous","Last Quarter","Waning Crescent"};int phase=(int)std::floor(fraction*8+.5)%8;
  double illum=(1-std::cos(6.283185307179586*fraction))*.5;
  if(illuminationTable.is_array()&&illuminationTable.size()>1){
   int64_t start=illuminationTable[0][0].get<int64_t>();int64_t index=(utc-start)/86400;
   if(utc>=start&&index>=0&&index+1<(int64_t)illuminationTable.size()){
    double a=illuminationTable[(size_t)index][1].get<double>(),b=illuminationTable[(size_t)index+1][1].get<double>();
    illum=a+(b-a)*(double)(utc-(start+index*86400))/86400.;
   }
  }
  double phaseAngle=std::acos(1-2*std::max(0.,std::min(1.,illum)));
  // Paint a small moon using the phase geometry, without an animation loop.
  for(int y=-39;y<=39;++y){double half=std::sqrt(std::max(0.0,1600.0-y*y));int left=(int)-half,right=(int)half;box(dc,84+left,184+y,right-left+1,1,RGB(55,65,91));double terminator=std::cos(phaseAngle)*half;
   if(fraction<.5){left=(int)terminator;right=(int)half;}else{left=(int)-half;right=(int)-terminator;}if(right>left)box(dc,84+left,184+y,right-left,1,RGB(241,236,202));}
  text(dc,150,151,24,names[phase],RGB(255,246,215),frame.width-182,32);text(dc,150,193,19,"Illuminated: ~"+std::to_string((int)std::round(illum*100))+"%");
  const auto&daily=weather_pages::section(data,"daily");int index=weather_pages::dayIndex(daily,calendar::dateString(local));
  auto sun=[&](const char*k){const auto&v=weather_pages::at(daily,k,index);return v.is_string()?features::solarTime(v.get<std::string>(),clock24):std::string("--");};
  text(dc,34,252,20,"Sunrise: "+sun("sunrise"),RGB(255,208,129),frame.width/2-40,29);text(dc,frame.width/2+12,252,20,"Sunset: "+sun("sunset"),RGB(255,208,129),frame.width/2-44,29);
  auto&duration=weather_pages::at(daily,"daylight_duration",index);std::string daylight="--";if(duration.is_number()){int m=(int)std::round(duration.get<double>()/60);daylight=std::to_string(m/60)+" h "+std::to_string(m%60)+" min";}text(dc,34,292,19,"Daylight: "+daylight,RGB(146,223,255),frame.width-68,27);
  text(dc,34,336,17,"Next New Moon: "+nextNew,RGB(255,246,215),frame.width-68,25);text(dc,34,365,17,"Next Full Moon: "+nextFull,RGB(255,246,215),frame.width-68,25);
  if(crtMode)text(dc,34,400,18,"Moon illumination estimated",RGB(255,246,215),frame.width-68,24);else text(dc,34,403,11,"Phase dates: Meeus | illumination estimated | "+zoneName(),RGB(255,246,215),frame.width-68,17);
 }else if(page==4){
  static int64_t cachedDay=-1;static std::vector<calendar::Holiday> holidays;int64_t day=calendar::dayNumber(local.year,local.month,local.day);
  if(day!=cachedDay){holidays=calendar::upcoming(local);cachedDay=day;}
  for(size_t i=0;i<std::min((size_t)5,holidays.size());++i){const auto&h=holidays[i];int y=101+(int)i*62;auto date=calendar::fromEpoch(h.day*86400);auto kind=page_decoration::symbol(h.name);COLORREF accent=holidayAccent(kind);dateBadge(dc,34,y+2,date,accent);int countX=frame.width-132;holidaySymbol(dc,countX-43,y+16,26,kind,accent);
   text(dc,103,y+3,18,h.name,i==0?accent:RGB(255,246,215),countX-154,25);text(dc,103,y+32,13,calendar::weekdayName(date)+" | "+std::to_string(date.year)+(i==0?" | UP NEXT":""),RGB(206,226,239),countX-115,21);
   text(dc,countX,y+12,17,page_decoration::countdown(h.day-day),accent,103,26);text(dc,countX,y+36,11,h.day==day?"CELEBRATE TODAY":"UNTIL THE DATE",RGB(206,226,239),103,17);
  }
 }else if(page==5){
  auto centered=[&](int y,int size,const std::string&value,COLORREF color){text(dc,std::max(32,(frame.width-measureText(dc,size,value))/2),y,size,value,color,frame.width-64,size+8);};
  COLORREF accent=sceneDay?RGB(255,216,143):RGB(161,221,255);centered(88,26,calendar::weekdayName(local),accent);weatherIcon(dc,frame.width/2-22,124,44,0,!sceneDay);
  std::string clock=presentation::clockText(local.hour,local.minute,clock24),suffix=local.hour>=12?"PM":"AM";int total=measureText(dc,82,clock)+(clock24?0:measureText(dc,27,suffix)+10),clockX=std::max(32,(frame.width-total)/2);int cw=text(dc,clockX,175,82,clock,RGB(255,246,215),frame.width-clockX-32,98);if(!clock24)text(dc,clockX+cw+10,218,27,suffix,accent,70,36);
  box(dc,frame.width/2-80,276,160,2,accent);
  int dateX=frame.width/2-141;dateBadge(dc,dateX,294,local,accent);text(dc,dateX+70,295,23,page_decoration::monthYear(local),accent,frame.width-dateX-102,32);text(dc,dateX+70,329,14,"Day "+std::to_string(page_decoration::dayOfYear(local))+" of "+std::to_string(calendar::leap(local.year)?366:365),RGB(206,226,239),frame.width-dateX-102,23);
  centered(369,17,zoneName().substr(0,62),RGB(206,226,239));centered(400,12,"PC clock | Location time zone adjusts for daylight saving",RGB(206,226,239));

 }else{
  text(dc,40,154,32,"Your local forecast returns soon");text(dc,40,219,24,"Next cycle in "+std::to_string(cycle.hold)+" seconds");text(dc,40,279,18,"Space or right-click: advance now");
 }
}
static memory_audit::History memoryHistory;static memory_audit::Events uiMemoryEvents;static DWORD memoryStarted=0;
static uint64_t trackedRenderBytes(){uint64_t total=backBufferStats().liveBytes+foregroundCache.bytes()+cloudCacheBytes()+sunRadiance.bytes()+smoothScaler.bytes()+audio.bufferedBytes()+transitionPixels.capacity()*4+previousScene.capacity()*4;for(const auto&mask:cloudMasks)total+=mask.opacity.capacity()+mask.lighting.capacity();return total;}
static memory_audit::Point memoryPoint(bool countModules=false){memory_audit::Point p;if(!memoryStarted)memoryStarted=GetTickCount();p.elapsed=(DWORD)(GetTickCount()-memoryStarted)/1000;p.page=cycle.page;p.builds=foregroundBuilds;p.trackedKB=trackedRenderBytes()/1024;
 char path[MAX_PATH];GetSystemDirectoryA(path,MAX_PATH);HMODULE dll=LoadLibraryA((std::string(path)+"\\psapi.dll").c_str());if(dll){typedef BOOL(WINAPI*GetMemory)(HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD);GetMemory fn=(GetMemory)GetProcAddress(dll,"GetProcessMemoryInfo");PROCESS_MEMORY_COUNTERS m={};m.cb=sizeof(m);if(fn&&fn(GetCurrentProcess(),&m,sizeof(m))){p.available=true;p.workingKB=m.WorkingSetSize/1024;p.commitKB=m.PagefileUsage/1024;}if(countModules){typedef BOOL(WINAPI*EnumModules)(HANDLE,HMODULE*,DWORD,LPDWORD);EnumModules enumerate=(EnumModules)GetProcAddress(dll,"EnumProcessModules");HMODULE modules[512];DWORD needed=0;if(enumerate&&enumerate(GetCurrentProcess(),modules,sizeof(modules),&needed))p.modules=needed/sizeof(HMODULE);}FreeLibrary(dll);}
 typedef DWORD(WINAPI*GetGUI)(HANDLE,DWORD);GetGUI gui=(GetGUI)GetProcAddress(GetModuleHandleA("user32.dll"),"GetGuiResources");if(gui){p.gdi=gui(GetCurrentProcess(),0);p.user=gui(GetCurrentProcess(),1);}return p;
}
static void recordUIEvent(memory_audit::Event event){uiMemoryEvents.push(event,memoryPoint(true));}
static const COLORREF cream=RGB(255,246,215),blueAccent=RGB(146,223,255),warmAccent=RGB(255,208,129),greenAccent=RGB(163,236,181);
static COLORREF temperatureColor(const json&v){return !v.is_number()?cream:v.get<double>()<=32?blueAccent:v.get<double>()>=80?warmAccent:cream;}
static std::string tempValue(const json&v){return v.is_number()?options::rounded(options::temperature(v.get<double>(),celsius))+options::temperatureLabel(celsius):"--";}
static std::string precipValue(const json&v){return weather_pages::value(v,celsius?" mm":" in",celsius?1:2,celsius?1:1/25.4);}
static std::string localHourKey(int64_t utc){auto l=localDate(utc);return calendar::dateString(l)+"T"+presentation::clockText(l.hour,0,true);}
static void metric(HDC dc,int x,int y,int width,const std::string&label,const std::string&value,COLORREF color=cream){text(dc,x,y,14,label,blueAccent,width,19);text(dc,x,y+20,22,value,color,width,29);}
static void chanceBar(HDC dc,int x,int y,int width,const json&v){if(!v.is_number())return;int fill=(int)(width*std::max(0.,std::min(100.,v.get<double>()))/100);box(dc,x,y,width,3,RGB(85,119,145));if(fill)box(dc,x,y,fill,3,blueAccent);}
static void drawWeatherPage(HDC dc,int page){
 const auto&c=weather_pages::section(data,"current");const auto&d=weather_pages::section(data,"daily");const auto&h=weather_pages::section(data,"hourly");auto local=localDate(foregroundEpoch);int today=weather_pages::dayIndex(d,calendar::dateString(local));int w=frame.width;
 auto current=[&](const char*k)->const json&{static const json missing;return c.contains(k)?c[k]:missing;};
 auto hours=weather_pages::hours(h,localHourKey(foregroundEpoch));
 auto hv=[&](const char*k)->const json&{return weather_pages::at(h,k,hours.empty()?-1:hours.front());};
 if(page==0){
  int right=w/2+12;weatherIcon(dc,w/2-100,137,76,weather_pages::code(current("weather_code")),!sceneDay);
  text(dc,32,139,68,tempValue(current("temperature_2m")),temperatureColor(current("temperature_2m")),w/2-110,85);
  text(dc,34,229,27,conditions(weather_pages::code(current("weather_code"))),cream,w-68,34);
  metric(dc,right,145,w-right-32,"FEELS LIKE",tempValue(current("apparent_temperature")));
  metric(dc,right,211,w-right-32,"HUMIDITY",weather_pages::value(current("relative_humidity_2m"),"%"));
  metric(dc,34,289,w/2-50,"WIND",windSpeed(c),blueAccent);
  metric(dc,right,289,w-right-32,"GUSTS",current("wind_gusts_10m").is_number()?options::rounded(options::wind(current("wind_gusts_10m").get<double>(),windUnit))+" "+options::windLabel(windUnit):"--",blueAccent);
  std::string direction=current("wind_direction_10m").is_number()?refinements::compass(current("wind_direction_10m").get<double>()):"--";text(dc,34,355,16,"From "+direction,cream,w-68,23);
  if(current("wind_direction_10m").is_number()){double a=current("wind_direction_10m").get<double>()*scenic::pi/180;int cx=w-66,cy=367,dx=(int)(17*std::sin(a)),dy=(int)(-17*std::cos(a));iconLine(dc,cx-dx,cy-dy,cx+dx,cy+dy,blueAccent,2);iconLine(dc,cx+dx,cy+dy,cx+dx-(int)(8*std::sin(a+.55)),cy+dy+(int)(8*std::cos(a+.55)),blueAccent,2);iconLine(dc,cx+dx,cy+dy,cx+dx-(int)(8*std::sin(a-.55)),cy+dy+(int)(8*std::cos(a-.55)),blueAccent,2);}
 }else if(page==1){
  int dateX=32,highX=w*23/100,lowX=w*37/100,chanceX=w*51/100,weatherX=w*65/100;
  text(dc,dateX,135,13,"DAY / DATE");text(dc,highX,135,13,"HIGH",warmAccent);text(dc,lowX,135,13,"LOW",blueAccent);text(dc,chanceX,135,13,"PRECIP",blueAccent);text(dc,weatherX,135,13,"CONDITIONS");
  for(int i=0;i<7;++i){int y=161+i*32;const auto&date=weather_pages::at(d,"time",i);std::string label="--";if(date.is_string()){auto st=date.get<std::string>();int yy,mm,dd;if(std::sscanf(st.c_str(),"%d-%d-%d",&yy,&mm,&dd)==3){auto dt=calendar::fromEpoch(calendar::epoch(yy,mm,dd));label=calendar::weekdayName(dt).substr(0,3)+" "+st.substr(5);}}text(dc,dateX,y,15,label,cream,highX-dateX-8,25);text(dc,highX,y,17,tempValue(weather_pages::at(d,"temperature_2m_max",i)),warmAccent,lowX-highX-6,25);text(dc,lowX,y,17,tempValue(weather_pages::at(d,"temperature_2m_min",i)),blueAccent,chanceX-lowX-6,25);auto&p=weather_pages::at(d,"precipitation_probability_max",i);text(dc,chanceX,y,16,weather_pages::value(p,"%"),blueAccent,weatherX-chanceX-8,25);chanceBar(dc,chanceX,y+24,weatherX-chanceX-20,p);int code=weather_pages::code(weather_pages::at(d,"weather_code",i));weatherIcon(dc,weatherX,y-1,25,code,previewMode&&previewIndex==1);text(dc,weatherX+31,y+1,14,conditions(code),cream,w-weatherX-60,24);}
 }else if(page==2){
  int col=(w-80)/3;int xs[]={32,32+col+8,32+2*(col+8)};metric(dc,xs[0],143,col,"DEW POINT",tempValue(hv("dew_point_2m")));metric(dc,xs[1],143,col,"VISIBILITY",weather_pages::value(hv("visibility"),celsius?" km":" mi",1,celsius?.001:1/1609.344));metric(dc,xs[2],143,col,"PRESSURE (SEA LEVEL)",weather_pages::value(current("pressure_msl")," hPa"));
  metric(dc,xs[0],226,col,"CLOUD COVER",weather_pages::value(current("cloud_cover"),"%"));auto&uv=weather_pages::at(d,"uv_index_max",today);COLORREF uvc=uv.is_number()?(uv.get<double>()<3?greenAccent:uv.get<double>()<6?warmAccent:RGB(255,167,163)):cream;metric(dc,xs[1],226,col,"UV MAX TODAY",weather_pages::value(uv,"",1),uvc);text(dc,xs[1],278,13,uv.is_number()?weather_pages::uvLabel(uv.get<double>()):"Not reported",uvc,col,20);
  metric(dc,xs[2],226,col,"PRECIP TOTAL TODAY",precipValue(weather_pages::at(d,"precipitation_sum",today)),blueAccent);
  metric(dc,xs[0],316,col,"SNOW TOTAL TODAY",weather_pages::value(weather_pages::at(d,"snowfall_sum",today),celsius?" cm":" in",1,celsius?1:1/2.54),blueAccent);
  text(dc,xs[1],321,13,"Hourly details: "+(hours.empty()?std::string("not reported"):features::solarTime(weather_pages::at(h,"time",hours.front()).get<std::string>(),clock24)),cream,w-xs[1]-32,24);
  text(dc,xs[1],351,13,"Model forecast; not a station observation.",cream,w-xs[1]-32,25);
 }else if(page==6){
  int half=w/2;int x[]={32,half+14};int pw=half-52;int code=weather_pages::code(weather_pages::at(d,"weather_code",today));text(dc,x[0],141,23,"TODAY",warmAccent);weatherIcon(dc,x[0]+pw-49,136,47,code);text(dc,x[0],188,19,conditions(code),cream,pw,27);metric(dc,x[0],226,pw,"HIGH / LOW",tempValue(weather_pages::at(d,"temperature_2m_max",today))+" / "+tempValue(weather_pages::at(d,"temperature_2m_min",today)));auto&p=weather_pages::at(d,"precipitation_probability_max",today);text(dc,x[0],290,17,"Precip chance: "+weather_pages::value(p,"%"),blueAccent,pw,26);chanceBar(dc,x[0],321,pw,p);text(dc,x[0],345,16,"Total: "+precipValue(weather_pages::at(d,"precipitation_sum",today)),blueAccent,pw,25);
  std::string date=calendar::dateString(local),next=calendar::dateString(calendar::fromEpoch(calendar::epoch(local.year,local.month,local.day)+86400));std::string start=local.hour<6?date+"T00:00":local.hour>=18?localHourKey(foregroundEpoch):date+"T18:00";std::string end=local.hour<6?date+"T06:00":next+"T06:00";auto night=weather_pages::night(h,start,end);
  text(dc,x[1],141,23,local.hour<6?"OVERNIGHT":"TONIGHT",blueAccent,pw,32);weatherIcon(dc,x[1]+pw-49,177,47,night.weather,true);text(dc,x[1],188,18,conditions(night.weather),cream,pw-48,36);metric(dc,x[1],226,pw,"LOW",night.haveLow?tempValue(json(night.low)):"--");text(dc,x[1],290,17,"Precip chance: "+(night.haveChance?weather_pages::value(json(night.chance),"%" ):"--"),blueAccent,pw,26);if(night.haveChance)chanceBar(dc,x[1],321,pw,json(night.chance));text(dc,x[1],345,16,"Total: "+(night.haveAmount?precipValue(json(night.amount)):"--"),blueAccent,pw,25);text(dc,32,390,12,"Tonight uses the remaining hourly forecast through 6 AM.",cream,w-64,18);
 }else if(page==7){
  int col=(w-64)/6;for(int i=0;i<12;++i){int idx=i<(int)hours.size()?hours[i]:-1,x=32+(i%6)*col,y=137+(i/6)*134;auto&t=weather_pages::at(h,"time",idx);text(dc,x,y,14,t.is_string()?features::solarTime(t.get<std::string>(),clock24):"--",cream,col-7,20);auto&day=weather_pages::at(h,"is_day",idx);weatherIcon(dc,x+(col-38)/2,y+22,38,weather_pages::code(weather_pages::at(h,"weather_code",idx)),day.is_number()&&day.get<int>()==0);auto&temp=weather_pages::at(h,"temperature_2m",idx);text(dc,x,y+66,21,tempValue(temp),temperatureColor(temp),col-7,27);auto&p=weather_pages::at(h,"precipitation_probability",idx);text(dc,x,y+97,14,weather_pages::value(p,"%"),blueAccent,col-7,20);chanceBar(dc,x,y+119,col-14,p);}
  text(dc,32,407,11,"Hourly model forecast | blue bars: precipitation chance",blueAccent,w-64,17);
 }
}
static void drawAboutPage(HDC dc){
 // Use the existing application logo; DrawIconEx participates in the cached foreground.
 static HICON logo=(HICON)LoadImageA(GetModuleHandle(0),MAKEINTRESOURCEA(1),IMAGE_ICON,128,128,LR_DEFAULTCOLOR|LR_SHARED);
 int x=frame.width/2-50,y=94;if(logo)DrawIconEx(dc,pixelX(x),pixelY(y),logo,std::max(1,(int)(100*drawX)),std::max(1,(int)(100*drawY)),0,0,DI_NORMAL);
 std::string name="Retro Weather";text(dc,(frame.width-measureText(dc,35,name))/2,205,35,name,warmAccent,frame.width-64,44);
 text(dc,32,264,18,"Version 1.2.1 | Classic weather, simply rendered.",cream,frame.width-64,27);
 text(dc,32,305,16,"Weather: Open-Meteo (CC BY 4.0)",blueAccent,frame.width-64,24);text(dc,32,333,14,"open-meteo.com | creativecommons.org/licenses/by/4.0",cream,frame.width-64,22);
 text(dc,32,362,16,"Locations: Zippopotam.us / GeoNames (ODbL)",blueAccent,frame.width-64,24);
 text(dc,32,397,12,"Forecasts are model-based. Not an alert service.",cream,frame.width-64,18);
}
static void crtBadge(HDC dc,int x,int y,calendar::Date date,COLORREF accent){box(dc,x,y,64,62,RGB(239,245,245));box(dc,x,y,64,24,accent);text(dc,x+3,y+3,18,page_decoration::month(date.month),RGB(18,40,65),58,22,false,true,true);text(dc,x+3,y+27,30,std::to_string(date.day),RGB(18,40,65),58,33,false,true,true);}
static void crtMetric(HDC dc,int x,int y,int width,const std::string&label,const std::string&value,COLORREF color=cream){text(dc,x,y,20,label,blueAccent,width,27);text(dc,x,y+34,30,value,color,width,39);}
static void drawCrtPage(HDC dc,int page){
 const auto&c=weather_pages::section(data,"current");const auto&d=weather_pages::section(data,"daily");const auto&h=weather_pages::section(data,"hourly");auto local=localDate(foregroundEpoch);int today=weather_pages::dayIndex(d,calendar::dateString(local));auto hours=weather_pages::hours(h,localHourKey(foregroundEpoch));auto cur=[&](const char*k)->const json&{static const json missing;return c.contains(k)?c[k]:missing;};auto hv=[&](const char*k)->const json&{return weather_pages::at(h,k,hours.empty()?-1:hours.front());};
 if(page==0){text(dc,30,119,72,tempValue(cur("temperature_2m")),temperatureColor(cur("temperature_2m")),300,88);weatherIcon(dc,420,112,96,weather_pages::code(cur("weather_code")),!sceneDay);text(dc,32,216,30,conditions(weather_pages::code(cur("weather_code"))),cream,576,40);text(dc,32,274,24,"Feels like: "+tempValue(cur("apparent_temperature")),cream,576,32);text(dc,32,325,23,"Humidity: "+weather_pages::value(cur("relative_humidity_2m"),"%"),cream,278,32);text(dc,330,325,23,"Wind: "+windSpeed(c),blueAccent,278,32);std::string gust=cur("wind_gusts_10m").is_number()?options::rounded(options::wind(cur("wind_gusts_10m").get<double>(),windUnit))+" "+options::windLabel(windUnit):"--";text(dc,32,376,22,"Gusts: "+gust,blueAccent,278,31);text(dc,330,376,20,cur("wind_direction_10m").is_number()?refinements::compass(cur("wind_direction_10m").get<double>()):"Direction: --",cream,278,31);
 }else if(page==1){int xs[]={28,135,235,335,430};const char*labels[]={"Day","High","Low","Chance","Weather"};for(int i=0;i<5;++i)text(dc,xs[i],109,18,labels[i],blueAccent,i==4?180:90,26);for(int i=0;i<7;++i){int y=143+i*36;auto&date=weather_pages::at(d,"time",i);std::string label="--";if(date.is_string()){auto st=date.get<std::string>();int yy,mm,dd;if(std::sscanf(st.c_str(),"%d-%d-%d",&yy,&mm,&dd)==3)label=calendar::weekdayName(calendar::fromEpoch(calendar::epoch(yy,mm,dd))).substr(0,3)+" "+std::to_string(dd);}text(dc,28,y,20,label,cream,102,30);text(dc,135,y,20,tempValue(weather_pages::at(d,"temperature_2m_max",i)),warmAccent,88,30);text(dc,235,y,20,tempValue(weather_pages::at(d,"temperature_2m_min",i)),blueAccent,88,30);text(dc,335,y,20,weather_pages::value(weather_pages::at(d,"precipitation_probability_max",i),"%"),blueAccent,86,30);int code=weather_pages::code(weather_pages::at(d,"weather_code",i));weatherIcon(dc,430,y,27,code,previewMode&&previewIndex==1);text(dc,464,y+1,18,crt_profile::shortCondition(code),cream,148,29);}text(dc,28,400,18,"Chance = rain or snow probability",blueAccent,584,24);
 }else if(page==2){if(!crtPart){crtMetric(dc,32,124,274,"Dew point",tempValue(hv("dew_point_2m")));crtMetric(dc,334,124,274,"Visibility",weather_pages::value(hv("visibility"),celsius?" km":" mi",1,celsius?.001:1/1609.344));crtMetric(dc,32,261,274,"Sea-level pressure",weather_pages::value(cur("pressure_msl")," hPa"));crtMetric(dc,334,261,274,"Cloud cover",weather_pages::value(cur("cloud_cover"),"%"));text(dc,32,384,18,"Dew point and visibility: hourly forecast",cream,576,25);}else{auto&uv=weather_pages::at(d,"uv_index_max",today);crtMetric(dc,32,124,274,"UV max today",weather_pages::value(uv,"",1),uv.is_number()&&uv.get<double>()>=6?warmAccent:greenAccent);text(dc,32,204,20,uv.is_number()?weather_pages::uvLabel(uv.get<double>()):"Not reported",cream,274,28);crtMetric(dc,334,124,274,"Precip total today",precipValue(weather_pages::at(d,"precipitation_sum",today)),blueAccent);crtMetric(dc,32,274,274,"Snow total today",weather_pages::value(weather_pages::at(d,"snowfall_sum",today),celsius?" cm":" in",1,celsius?1:1/2.54),blueAccent);crtMetric(dc,334,274,274,"Wind gusts",cur("wind_gusts_10m").is_number()?options::rounded(options::wind(cur("wind_gusts_10m").get<double>(),windUnit))+" "+options::windLabel(windUnit):"--",blueAccent);}
 }else if(page==6){int code=weather_pages::code(weather_pages::at(d,"weather_code",today));text(dc,32,110,25,"TODAY",warmAccent,400,34);weatherIcon(dc,518,110,64,code);text(dc,32,154,26,tempValue(weather_pages::at(d,"temperature_2m_max",today))+" high / "+tempValue(weather_pages::at(d,"temperature_2m_min",today))+" low",cream,470,36);text(dc,32,194,21,conditions(code),cream,576,29);text(dc,32,228,20,"Precip: "+weather_pages::value(weather_pages::at(d,"precipitation_probability_max",today),"%")+" | "+precipValue(weather_pages::at(d,"precipitation_sum",today)),blueAccent,576,28);std::string date=calendar::dateString(local),next=calendar::dateString(calendar::fromEpoch(calendar::epoch(local.year,local.month,local.day)+86400));std::string start=local.hour<6?date+"T00:00":local.hour>=18?localHourKey(foregroundEpoch):date+"T18:00",end=local.hour<6?date+"T06:00":next+"T06:00";auto n=weather_pages::night(h,start,end);text(dc,32,272,25,local.hour<6?"OVERNIGHT":"TONIGHT",blueAccent,400,34);weatherIcon(dc,518,272,64,n.weather,true);text(dc,32,316,26,(n.haveLow?tempValue(json(n.low)):"--")+" low",cream,470,36);text(dc,32,356,21,conditions(n.weather),cream,576,29);text(dc,32,390,20,"Precip: "+(n.haveChance?weather_pages::value(json(n.chance),"%" ):"--")+" | "+(n.haveAmount?precipValue(json(n.amount)):"--"),blueAccent,576,28);
 }else if(page==7){for(int i=0;i<6;++i){int at=crt_profile::firstHour(crtPart)+i,idx=at<(int)hours.size()?hours[at]:-1,x=32+(i%3)*198,y=117+(i/3)*148;auto&t=weather_pages::at(h,"time",idx);text(dc,x,y,21,t.is_string()?features::solarTime(t.get<std::string>(),clock24):"--",cream,180,29);auto&day=weather_pages::at(h,"is_day",idx);weatherIcon(dc,x,y+36,48,weather_pages::code(weather_pages::at(h,"weather_code",idx)),day.is_number()&&day.get<int>()==0);auto&temp=weather_pages::at(h,"temperature_2m",idx);text(dc,x+58,y+42,28,tempValue(temp),temperatureColor(temp),122,38);auto&p=weather_pages::at(h,"precipitation_probability",idx);text(dc,x,y+103,20,"Precip "+weather_pages::value(p,"%"),blueAccent,180,28);chanceBar(dc,x,y+137,174,p);}
 }else if(page==3){drawExtras(dc,3);
 }else if(page==4){auto list=calendar::upcoming(local);int64_t day=calendar::dayNumber(local.year,local.month,local.day);for(size_t i=0;i<std::min(size_t(5),list.size());++i){auto&entry=list[i];auto date=calendar::fromEpoch(entry.day*86400);int y=90+(int)i*66;auto symbol=page_decoration::symbol(entry.name);auto accent=holidayAccent(symbol);crtBadge(dc,28,y,date,accent);text(dc,110,y+2,20,entry.name,i?cream:accent,322,30);text(dc,110,y+34,18,calendar::weekdayName(date).substr(0,3)+" "+std::to_string(date.year)+(i?"":" | NEXT"),cream,322,26);holidaySymbol(dc,448,y+20,26,symbol,accent);text(dc,490,y+20,20,page_decoration::countdown(entry.day-day),accent,122,31);}
 }else if(page==5){auto center=[&](int y,int size,const std::string&v,COLORREF color){text(dc,28,y,size,v,color,584,size+10,true,false,true);};COLORREF accent=sceneDay?warmAccent:blueAccent;center(87,26,calendar::weekdayName(local),accent);weatherIcon(dc,298,124,44,0,!sceneDay);std::string clock=presentation::clockText(local.hour,local.minute,clock24),suffix=local.hour>=12?"PM":"AM";int total=measureText(dc,76,clock)+(clock24?0:measureText(dc,26,suffix)+8),x=(640-total)/2,cw=text(dc,x,174,76,clock,cream,640-x-28,94);if(!clock24)text(dc,x+cw+8,214,26,suffix,accent,66,36);crtBadge(dc,142,284,local,accent);text(dc,228,285,24,page_decoration::monthYear(local),accent,384,34);text(dc,228,325,20,"Day "+std::to_string(page_decoration::dayOfYear(local))+" of "+std::to_string(calendar::leap(local.year)?366:365),cream,384,28);center(370,20,zoneName(),cream);center(400,18,"Location time from your PC clock",cream);
 }else if(page==8){static HICON logo=LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(1));DrawIconEx(dc,pixelX(34),pixelY(103),logo,pixelWidth(80),std::max(1,(int)(80*drawY)),0,0,DI_NORMAL);text(dc,136,113,34,"Retro Weather",warmAccent,476,46);text(dc,136,161,22,"Version 1.2.1",cream,476,32);text(dc,32,235,21,"Weather: Open-Meteo (CC BY 4.0)",blueAccent,576,30);text(dc,32,273,18,"creativecommons.org/licenses/by/4.0",cream,576,27);text(dc,32,310,21,"Locations: Zippopotam.us / GeoNames",blueAccent,576,30);text(dc,32,348,18,"open-meteo.com | zippopotam.us",cream,576,27);text(dc,32,395,18,"Model forecasts; not an alert service.",cream,576,26);
 }else{drawExtras(dc,page);}
}
static void drawCrtForeground(HDC dc,int page){drawOriginY=0;std::string title=page>=0&&page<features::pageCount?features::pageNames[page]:"Intermission";if(page==2||page==7)title+=" ("+std::to_string(crtPart+1)+"/2)";text(dc,28,15,30,title,cream,584,42);if(page==0||page==1||page==2||page==6||page==7){std::string location=previewMode?"SIMULATED WEATHER":data.empty()?"Add a location from the Weather menu":data.value("city",std::string())+", "+data.value("state",std::string())+" | "+data.value("zip",std::string());text(dc,28,67,20,location,cream,584,29);}else if(previewMode)text(dc,28,61,18,"SIMULATED WEATHER",blueAccent,584,25);
 if(data.empty()&&(page==0||page==1||page==2||page==6||page==7)){text(dc,32,166,26,"Add your location to load the forecast.",cream,576,40);text(dc,32,230,22,"Weather > Add or Change Location",cream,576,34);}else drawCrtPage(dc,page);
 std::string notice=status;if(!data.empty()){auto updated=localDate(data.value("fetched",0LL));int64_t age=std::max<int64_t>(0,(foregroundEpoch-data.value("fetched",0LL))/60);notice=(lastUpdateFailed?"Update failed | ":age>60?"Stale | ":!sessionUpdated?"Saved | ":"")+std::string("Updated ")+presentation::clockText(updated.hour,updated.minute,true)+" | "+std::to_string(age)+" min ago";}if(previewMode)notice="Benchmark: simulated weather";if(page!=8)text(dc,24,427,18,notice,cream,592,24);text(dc,24,452,18,(cycle.paused?"Paused | ":"")+std::string(!previewMode&&!data.empty()?"open-meteo.com (CC BY 4.0) | Audio: ":"Audio: ")+(audio.enabled?audio.title():std::string("Disabled")),cream,592,24);
}
static void drawForeground(HDC mem,int page){
 if(crtMode){crt_profile::SafeContent safe(drawX,drawY,drawOffsetX,drawOffsetY);drawCrtForeground(mem,page);return;}
 drawOriginY=0;text(mem,30,17,30,page>=0&&page<features::pageCount?features::pageNames[page]:"Intermission",cream,frame.width-60,36);
 if(previewMode||cycle.paused)text(mem,32,57,13,previewMode?(recordingSceneName.empty()?std::string("BENCHMARK - SIMULATED"):std::string("REPLAY - ")+recordingSceneName+" - SIMULATED"):"PAUSED",blueAccent,frame.width-64,19);
 drawOriginY=frame.bodyOffset();
 if(page==8)drawAboutPage(mem);
 else if(page==3||page==4||page==5||page==features::pausePage)drawExtras(mem,page);
 else if(!data.empty()){
  text(mem,32,86,21,data.value("city",std::string())+", "+data.value("state",std::string()),cream,frame.width-64,27);
  text(mem,32,113,12,data.value("country",std::string("United States"))+" | "+data.value("zip",std::string()),cream,frame.width-64,18);
  drawWeatherPage(mem,page);
 }else{text(mem,35,150,28,"Your local forecast, simply rendered.",cream,frame.width-70,38);text(mem,35,213,20,"Weather > Add or Change Location... (Ctrl+L)",cream,frame.width-70,30);text(mem,35,255,18,"F11: fullscreen    Space: next page",cream,frame.width-70,28);}
 drawOriginY=0;std::string notice=status;
 if(!data.empty()){int64_t fetched=data.value("fetched",0LL),age=foregroundEpoch-fetched;calendar::Date updated=localDate(fetched);notice=(lastUpdateFailed?"UPDATE FAILED | ":!sessionUpdated?"SAVED WEATHER | ":"")+std::string("Updated ")+calendar::dateString(updated)+" "+presentation::clockText(updated.hour,updated.minute,true)+" | Age: "+std::to_string(std::max<int64_t>(0,age/60))+" min";if(age>3600||age<0)notice="STALE | "+notice;if(busy)notice+=" | Updating...";}
 if(previewMode)notice="BENCHMARK - SIMULATED WEATHER";
 if(page!=8)text(mem,25,frame.height-43,12,notice,cream,frame.width-50,17);
 text(mem,25,frame.height-23,12,std::string(!previewMode&&!data.empty()?"open-meteo.com (CC BY 4.0) | Audio: ":"Audio: ")+(audio.enabled?audio.title():std::string("Off")),cream,frame.width-50,18);
}
static void paint(HWND h){
 int64_t paintStarted=renderNow();int page=cycle.page;drawingPixelOffsetY=0;PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rc;GetClientRect(h,&rc);presentation::Viewport v=viewport(h);frame=crtMode?presentation::Frame():presentation::frameFor(v);drawOriginY=0;if(IsIconic(h)||v.width<=0||v.height<=0){EndPaint(h,&ps);return;}
 auto size=rendering::foreground(v.width,v.height,frame.width,frame.height,crtMode);int renderWidth=size.width,renderHeight=size.height;if(!frameDirty&&pictureBuffer.dc&&pictureBuffer.width==renderWidth&&pictureBuffer.height==renderHeight){int clip=SaveDC(dc);ExcludeClipRect(dc,v.x,v.y,v.x+v.width,v.y+v.height);FillRect(dc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));RestoreDC(dc,clip);if(renderWidth==v.width&&renderHeight==v.height)BitBlt(dc,v.x,v.y,v.width,v.height,pictureBuffer.dc,0,0,SRCCOPY);else{SetStretchBltMode(dc,COLORONCOLOR);StretchBlt(dc,v.x,v.y,v.width,v.height,pictureBuffer.dc,0,0,renderWidth,renderHeight,SRCCOPY);}EndPaint(h,&ps);finishAnimationFrame(h,paintStarted,false);return;}
 if(!pictureBuffer.ensure(dc,renderWidth,renderHeight)){size=rendering::fit(frame.width,frame.height,640);renderWidth=size.width;renderHeight=size.height;if(!pictureBuffer.ensure(dc,renderWidth,renderHeight)){EndPaint(h,&ps);finishAnimationFrame(h,paintStarted);return;}}
 dirtyClipActive=false;dirtyRegions.clear();bool partial=false;int pictureState=0;HDC mem=pictureBuffer.dc;drawX=renderWidth/(double)frame.width;drawY=renderHeight/(double)frame.height;SetBkMode(mem,TRANSPARENT);
 lastCacheMs=lastScaleMs=lastOverlayMs=lastPresentMs=0;if(!isScenic()){lastSceneMs=0;scaleBackend="Not used";scaleComparisonActive=false;renderProfile.reset();}int64_t cacheStarted=renderNow();
 bool rebuildForeground=contentDirty||!foregroundReady||foregroundCache.width!=renderWidth||foregroundCache.height!=renderHeight;frameRebuiltForeground=rebuildForeground;
 if(rebuildForeground){contentDirty=true;foregroundEpoch=nowEpoch();int band=memory_audit::stripHeight(renderHeight);if(foregroundScratch.ensure(dc,renderWidth,band)){
 PatBlt(mem,0,0,renderWidth,renderHeight,BLACKNESS);drawForeground(mem,page);GdiFlush();foregroundCache.begin(renderWidth,renderHeight);
 for(int first=0;first<renderHeight;first+=band){drawingPixelOffsetY=first;PatBlt(foregroundScratch.dc,0,0,renderWidth,band,WHITENESS);drawForeground(foregroundScratch.dc,page);drawingPixelOffsetY=0;GdiFlush();foregroundCache.appendBackgroundStrip(pictureBuffer.pixels,foregroundScratch.pixels,first,std::min(band,renderHeight-first));}
 foregroundCache.finish();foregroundLeft=renderWidth;foregroundTop=renderHeight;foregroundRight=foregroundBottom=0;for(const auto&run:foregroundCache.runs){foregroundLeft=std::min(foregroundLeft,run.x);foregroundTop=std::min(foregroundTop,run.y);foregroundRight=std::max(foregroundRight,run.x+run.count);foregroundBottom=std::max(foregroundBottom,run.y+1);}foregroundReady=true;++foregroundBuilds;
 }else foregroundReady=false;drawingPixelOffsetY=0;drawOriginY=0;}

 if(rebuildForeground)lastCacheMs=std::max(0.,(renderNow()-cacheStarted)*1000./renderFrequency());
 if(isScenic()){
  auto backdrop=rendering::fit(renderWidth,renderHeight,saverPreview?640:sceneLimit());if(legacyRendering&&sceneLevel)backdrop=rendering::fit(renderWidth,renderHeight,sceneLevel==1?480:640);
  if(sceneBuffer.ensure(dc,backdrop.width,backdrop.height)){
   if(sceneLevel||sceneCacheBackground!=background||sceneCacheWidth!=backdrop.width||sceneCacheHeight!=backdrop.height||sceneCacheFrameWidth!=frame.width||sceneCacheFrameHeight!=frame.height){double foregroundX=drawX,foregroundY=drawY;drawX=backdrop.width/(double)frame.width;drawY=backdrop.height/(double)frame.height;renderScene(sceneBuffer.dc);drawX=foregroundX;drawY=foregroundY;sceneCacheBackground=background;sceneCacheWidth=backdrop.width;sceneCacheHeight=backdrop.height;sceneCacheFrameWidth=frame.width;sceneCacheFrameHeight=frame.height;}
   GdiFlush();bool reusable=legacyRendering&&sceneLevel&&!contentDirty&&previousSceneWidth==backdrop.width&&previousSceneHeight==backdrop.height&&previousScene.size()==(size_t)backdrop.width*backdrop.height;
   if(reusable){dirtyRegions=rendering::changedTiles(previousScene.data(),sceneBuffer.pixels,backdrop.width,backdrop.height,renderWidth,renderHeight);partial=rendering::area(dirtyRegions)<(int64_t)renderWidth*renderHeight*3/5;}
   if(partial){pictureState=SaveDC(mem);HRGN region=CreateRectRgn(0,0,0,0);bool clipOK=pictureState&&region;for(auto r:dirtyRegions){if(!clipOK)break;HRGN tile=CreateRectRgn(r.left,r.top,r.right,r.bottom);if(!tile||CombineRgn(region,region,tile,RGN_OR)==ERROR)clipOK=false;if(tile)DeleteObject(tile);}if(clipOK)clipOK=ExtSelectClipRgn(mem,region,RGN_AND)!=ERROR;if(region)DeleteObject(region);if(clipOK){rendering::restoreNearest(pictureBuffer.pixels,renderWidth,renderHeight,sceneBuffer.pixels,backdrop.width,backdrop.height,dirtyRegions);dirtyClipActive=true;lastDirtyPercent=(int)(rendering::area(dirtyRegions)*100/((int64_t)renderWidth*renderHeight));}else{if(pictureState)RestoreDC(mem,pictureState);pictureState=0;partial=false;lastDirtyPercent=100;rendering::restoreNearest(pictureBuffer.pixels,renderWidth,renderHeight,sceneBuffer.pixels,backdrop.width,backdrop.height,{{0,0,renderWidth,renderHeight}});}}

   else{lastDirtyPercent=100;scaleScene(mem,renderWidth,renderHeight,backdrop.width,backdrop.height);}

   if(legacyRendering&&sceneLevel){previousScene.assign(sceneBuffer.pixels,sceneBuffer.pixels+(size_t)backdrop.width*backdrop.height);previousSceneWidth=backdrop.width;previousSceneHeight=backdrop.height;}else{previousScene.clear();previousSceneWidth=previousSceneHeight=0;}

  }else{previousScene.clear();previousSceneWidth=previousSceneHeight=0;renderScene(mem);}
 }else{sceneCacheBackground=-1;drawBackground(mem);}

 int64_t overlayStarted=renderNow();
 if(!partial||!dirtyRegions.empty()){if(foregroundReady){GdiFlush();if(partial){for(auto r:dirtyRegions)foregroundCache.composite(pictureBuffer.pixels,renderWidth,renderHeight,displayShift().x,displayShift().y,r);}else foregroundCache.composite(pictureBuffer.pixels,renderWidth,renderHeight,displayShift().x,displayShift().y);}else drawForeground(mem,page);}
 GdiFlush();lastOverlayMs=std::max(0.,(renderNow()-overlayStarted)*1000./renderFrequency());
 if(pictureState)RestoreDC(mem,pictureState);dirtyClipActive=false;lastPartialFrame=partial;
 // Clear only letterbox/safe-margin pixels. Never clear the visible picture
 // before presenting the finished back buffer: that caused the frame flashes.
 int64_t presentStarted=renderNow();int clip=SaveDC(dc);ExcludeClipRect(dc,v.x,v.y,v.x+v.width,v.y+v.height);FillRect(dc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));RestoreDC(dc,clip);
 if(v.width>0&&v.height>0){if(renderWidth==v.width&&renderHeight==v.height)BitBlt(dc,v.x,v.y,v.width,v.height,mem,0,0,SRCCOPY);else{SetStretchBltMode(dc,crtMode?COLORONCOLOR:HALFTONE);SetBrushOrgEx(dc,0,0,0);StretchBlt(dc,v.x,v.y,v.width,v.height,mem,0,0,renderWidth,renderHeight,SRCCOPY);}}
 GdiFlush();lastPresentMs=std::max(0.,(renderNow()-presentStarted)*1000./renderFrequency());
 if(animationPending&&isScenic())renderProfile.observe(lastSceneMs,lastScaleMs,lastOverlayMs,lastPresentMs,sceneCold||frameRebuiltForeground);
 if(frameRebuiltForeground&&foregroundReady)memoryHistory.push(memoryPoint());
 contentDirty=false;frameDirty=false;EndPaint(h,&ps);finishAnimationFrame(h,paintStarted);if(fontCache.size()>512)clearFonts();
}
static void fitFullscreen(){
 MONITORINFO mi={};mi.cbSize=sizeof(mi);
 if(GetMonitorInfo(MonitorFromWindow(mainWindow,MONITOR_DEFAULTTONEAREST),&mi))
  SetWindowPos(mainWindow,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,
   mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,
   SWP_FRAMECHANGED|SWP_SHOWWINDOW);
}
static void toggleFull(){
 if(!full){trackPlacement(mainWindow);wasMaximized=windowMaximized;}changingWindowMode=true;
 full=!full;
 wakeCursor();
 if(full){
  SetTimer(mainWindow,2,250,0);
  saved=normalWindow;SetMenu(mainWindow,0);
  SetWindowLong(mainWindow,GWL_STYLE,WS_POPUP|WS_VISIBLE|WS_CLIPCHILDREN);
  fitFullscreen();
 }else{
  KillTimer(mainWindow,2);
  // Fit the restored window into an available work area if the display changed.
  MONITORINFO mi={};mi.cbSize=sizeof(mi);
  if(GetMonitorInfo(MonitorFromRect(&saved,MONITOR_DEFAULTTONEAREST),&mi)){
   int width=std::min((int)(saved.right-saved.left),(int)(mi.rcWork.right-mi.rcWork.left));
   int height=std::min((int)(saved.bottom-saved.top),(int)(mi.rcWork.bottom-mi.rcWork.top));
   saved.left=std::max((int)mi.rcWork.left,std::min((int)saved.left,(int)mi.rcWork.right-width));
   saved.top=std::max((int)mi.rcWork.top,std::min((int)saved.top,(int)mi.rcWork.bottom-height));
   saved.right=saved.left+width;saved.bottom=saved.top+height;
  }
  SetWindowLong(mainWindow,GWL_STYLE,WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN);
  SetMenu(mainWindow,menus);
  SetWindowPos(mainWindow,0,saved.left,saved.top,saved.right-saved.left,
   saved.bottom-saved.top,SWP_FRAMECHANGED|SWP_NOZORDER|SWP_SHOWWINDOW);
 }
 if(!full&&wasMaximized)ShowWindow(mainWindow,SW_MAXIMIZE);changingWindowMode=false;if(!full)trackPlacement(mainWindow);
 SetFocus(mainWindow);updateMenus();repaint(mainWindow,TRUE);
}
static void appendAudioMenu(HMENU menu);static HMENU buildMenus(bool popup=false);
static void contextMenu(HWND h,LPARAM l){POINT p={(short)LOWORD(l),(short)HIWORD(l)};if(p.x==-1&&p.y==-1)GetCursorPos(&p);HMENU popup=buildMenus(true);SetForegroundWindow(h);UINT command=TrackPopupMenu(popup,TPM_RETURNCMD|TPM_RIGHTBUTTON,p.x,p.y,0,h,0);DestroyMenu(popup);if(command)SendMessage(h,WM_COMMAND,command,0);}
static void appendAudioMenu(HMENU menu){AppendMenuA(menu,MF_STRING,130,audio.enabled?"Disable Weather Audio\tM":"Enable Weather Audio\tM");HMENU volume=CreatePopupMenu();int levels[]={0,10,25,50,75,100};for(int i=0;i<6;++i){std::string label=std::to_string(levels[i])+"%";AppendMenuA(volume,MF_STRING|(audio.volume==levels[i]?MF_CHECKED:0),150+i,label.c_str());}AppendMenuA(menu,MF_POPUP,(UINT_PTR)volume,"Volume");}
static HMENU buildMenus(bool popup){
 HMENU root=(popup?CreatePopupMenu():CreateMenu()),weather=CreatePopupMenu(),display=CreatePopupMenu(),pages=CreatePopupMenu(),sound=CreatePopupMenu(),tools=CreatePopupMenu(),help=CreatePopupMenu();appendLocations(weather);
 HMENU temperatures=CreatePopupMenu();AppendMenuA(temperatures,MF_STRING|(!celsius?MF_CHECKED:0),320,"Fahrenheit");AppendMenuA(temperatures,MF_STRING|(celsius?MF_CHECKED:0),321,"Celsius");AppendMenuA(weather,MF_SEPARATOR,0,0);AppendMenuA(weather,MF_POPUP,(UINT_PTR)temperatures,"Temperature Units");HMENU wind=CreatePopupMenu();for(int i=0;i<4;++i)AppendMenuA(wind,MF_STRING|(windUnit==i?MF_CHECKED:0),330+i,options::windLabel(i));AppendMenuA(weather,MF_POPUP,(UINT_PTR)wind,"Wind Units");
 AppendMenuA(display,MF_STRING,101,full?"Exit Fullscreen\tF11":"Fullscreen\tF11");AppendMenuA(display,MF_STRING|(crtMode?MF_CHECKED:0),220,"CRT Mode");AppendMenuA(display,MF_POPUP,(UINT_PTR)makeBackgrounds(),"Background");AppendMenuA(display,MF_STRING|(startFullscreen?MF_CHECKED:0),123,"Start Fullscreen");HMENU advanced=CreatePopupMenu(),margins=CreatePopupMenu(),sizes=CreatePopupMenu(),aspects=CreatePopupMenu();int values[]={0,2,5,8};for(int i=0;i<4;++i){auto label=std::to_string(values[i])+"%";AppendMenuA(margins,MF_STRING|(safeMargin==values[i]?MF_CHECKED:0),210+i,label.c_str());}for(int i=0;i<3;++i){auto label=std::to_string(90+i*10)+"%";AppendMenuA(sizes,MF_STRING|(textPercent==90+i*10?MF_CHECKED:0),230+i,label.c_str());}for(int i=0;i<6;++i)AppendMenuA(aspects,MF_STRING|(aspectMode==i?MF_CHECKED:0),300+i,presentation::aspectName(i));AppendMenuA(advanced,MF_POPUP|(crtMode?MF_GRAYED:0),(UINT_PTR)margins,"Safe Margins");AppendMenuA(advanced,MF_POPUP|(crtMode?MF_GRAYED:0),(UINT_PTR)sizes,"Text Size");AppendMenuA(advanced,MF_POPUP|(crtMode?MF_GRAYED:0),(UINT_PTR)aspects,"Aspect Ratio");if(crtMode)AppendMenuA(advanced,MF_STRING|MF_GRAYED,0,"CRT profile: full output, large safe text");AppendMenuA(display,MF_POPUP,(UINT_PTR)advanced,"Advanced Display");
 AppendMenuA(pages,MF_STRING,102,"Next Page\tSpace");AppendMenuA(pages,MF_STRING,106,cycle.paused?"Resume Pages and Audio\tP":"Pause Pages and Audio\tP");AppendMenuA(pages,MF_SEPARATOR,0,0);AppendMenuA(pages,MF_STRING,122,"Choose / Order Pages...");AppendMenuA(pages,MF_STRING,120,"Page Timing...");AppendMenuA(pages,MF_STRING|(clock24?MF_CHECKED:0),121,"24-hour Clock");appendAudioMenu(sound);
 AppendMenuA(tools,MF_STRING,166,"Diagnostics / Benchmark...");HMENU backups=CreatePopupMenu();AppendMenuA(backups,MF_STRING,164,"Back Up Settings...");AppendMenuA(backups,MF_STRING,165,"Restore Settings...");AppendMenuA(tools,MF_POPUP,(UINT_PTR)backups,"Settings Backup");HMENU saver=CreatePopupMenu();AppendMenuA(saver,MF_STRING,162,"Screensaver Controls...");AppendMenuA(saver,MF_STRING,163,"Windows Screensaver Settings...");AppendMenuA(tools,MF_POPUP,(UINT_PTR)saver,"Screensaver");AppendMenuA(tools,MF_STRING|(isAutostart()?MF_CHECKED:0),124,"Start with Windows");AppendMenuA(tools,MF_STRING,126,"Restore Display / Page Defaults...");AppendMenuA(tools,MF_SEPARATOR,0,0);AppendMenuA(tools,MF_STRING,104,"Exit Retro Weather\tAlt+F4");AppendMenuA(help,MF_STRING,161,"Controls / Shortcuts...");AppendMenuA(help,MF_STRING,160,"About Retro Weather\tF1");AppendMenuA(help,MF_STRING,167,"Weather Data / License Terms...");AppendMenuA(help,MF_STRING,168,"Open-Meteo Attribution Website...");
 const char*names[]={"&Weather","&Display","&Pages","&Audio","&Tools","&Help"};HMENU groups[]={weather,display,pages,sound,tools,help};for(int i=0;i<6;++i)AppendMenuA(root,MF_POPUP,(UINT_PTR)groups[i],names[i]);return root;
}
static void updateMenus(){syncWeatherAudio();HMENU previous=menus;menus=buildMenus();if(mainWindow&&!full&&!saverRun&&!saverPreview){SetMenu(mainWindow,menus);DrawMenuBar(mainWindow);}if(previous)DestroyMenu(previous);}
static std::array<int,9> editingDurations;static int editingPage=0;
static void setDurationFields(HWND h,int edit,int combo,int seconds){bool minutes=seconds>0&&seconds%60==0;SetDlgItemInt(h,edit,minutes?seconds/60:seconds,FALSE);SendDlgItemMessage(h,combo,CB_SETCURSEL,minutes?1:0,0);}
static bool readDuration(HWND h,int edit,int combo,bool allowZero,int&seconds){BOOL ok;UINT n=GetDlgItemInt(h,edit,&ok,FALSE);int mul=SendDlgItemMessage(h,combo,CB_GETCURSEL,0,0)==1?60:1;if(!ok||(!allowZero&&n==0)||n>3600u/mul){MessageBoxA(h,"Choose 1 second to 60 minutes per page, or 0 to 60 minutes for the cycle pause.","Timing",MB_OK|MB_ICONEXCLAMATION);SetFocus(GetDlgItem(h,edit));return false;}seconds=(int)n*mul;return true;}
static void showPageDuration(HWND h){bool useDefault=editingDurations[editingPage]==0;CheckDlgButton(h,508,useDefault?BST_CHECKED:BST_UNCHECKED);setDurationFields(h,506,507,useDefault?duration:editingDurations[editingPage]);EnableWindow(GetDlgItem(h,506),!useDefault);EnableWindow(GetDlgItem(h,507),!useDefault);}
static bool commitPageDuration(HWND h){if(IsDlgButtonChecked(h,508)==BST_CHECKED){editingDurations[editingPage]=0;return true;}int n;if(!readDuration(h,506,507,false,n))return false;editingDurations[editingPage]=n;return true;}
static INT_PTR CALLBACK timingDialog(HWND h,UINT msg,WPARAM w,LPARAM){
 if(msg==WM_INITDIALOG){for(int id:{502,504,507}){SendDlgItemMessageA(h,id,CB_ADDSTRING,0,(LPARAM)"Seconds");SendDlgItemMessageA(h,id,CB_ADDSTRING,0,(LPARAM)"Minutes");}for(auto name:features::pageNames)SendDlgItemMessageA(h,505,CB_ADDSTRING,0,(LPARAM)name);editingDurations=pageDurations;editingPage=cycle.page<features::pageCount?cycle.page:0;SendDlgItemMessage(h,505,CB_SETCURSEL,editingPage,0);setDurationFields(h,501,502,duration);setDurationFields(h,503,504,pauseDuration);showPageDuration(h);return TRUE;}
 if(msg==WM_COMMAND){int cmd=LOWORD(w);if(cmd==IDCANCEL){EndDialog(h,IDCANCEL);return TRUE;}if(cmd==505&&HIWORD(w)==CBN_SELCHANGE){int next=(int)SendDlgItemMessage(h,505,CB_GETCURSEL,0,0);if(!commitPageDuration(h)){SendDlgItemMessage(h,505,CB_SETCURSEL,editingPage,0);return TRUE;}editingPage=next;showPageDuration(h);return TRUE;}if(cmd==508){bool useDefault=IsDlgButtonChecked(h,508)==BST_CHECKED;if(!useDefault&&editingDurations[editingPage]==0){int fallback;if(readDuration(h,501,502,false,fallback))setDurationFields(h,506,507,fallback);}EnableWindow(GetDlgItem(h,506),!useDefault);EnableWindow(GetDlgItem(h,507),!useDefault);return TRUE;}if(cmd==IDOK){int a,b;if(!commitPageDuration(h)||!readDuration(h,501,502,false,a)||!readDuration(h,503,504,true,b))return TRUE;duration=a;pauseDuration=b;pageDurations=editingDurations;cycle.restart();savePrefs();repaint(mainWindow);EndDialog(h,IDOK);return TRUE;}}
 return FALSE;
}
static features::PageConfig editingPages;
static void fillPageList(HWND h,int selected){SendDlgItemMessage(h,601,LB_RESETCONTENT,0,0);for(int n:editingPages.order){std::string label=(editingPages.mask&(1u<<n)?"[x] ":"[ ] ")+std::string(features::pageNames[n]);SendDlgItemMessageA(h,601,LB_ADDSTRING,0,(LPARAM)label.c_str());}SendDlgItemMessage(h,601,LB_SETCURSEL,selected,0);}
static INT_PTR CALLBACK pagesDialog(HWND h,UINT msg,WPARAM w,LPARAM){if(msg==WM_INITDIALOG){editingPages=pageConfig;fillPageList(h,0);return TRUE;}if(msg==WM_COMMAND){int cmd=LOWORD(w),selected=(int)SendDlgItemMessage(h,601,LB_GETCURSEL,0,0);if(cmd==IDCANCEL){EndDialog(h,IDCANCEL);return TRUE;}if(cmd==IDOK){if(!editingPages.mask){MessageBoxA(h,"Keep at least one page enabled.","Pages",MB_OK);return TRUE;}pageConfig=editingPages;cycle.configure(pageConfig);saveExtraPrefs();repaint(mainWindow);EndDialog(h,IDOK);return TRUE;}if(selected>=0&&selected<features::pageCount){if(cmd==602)editingPages.mask^=1u<<editingPages.order[selected];else if(cmd==603&&selected>0){std::swap(editingPages.order[selected],editingPages.order[selected-1]);--selected;}else if(cmd==604&&selected<features::pageCount-1){std::swap(editingPages.order[selected],editingPages.order[selected+1]);++selected;}else return FALSE;fillPageList(h,selected);return TRUE;}}return FALSE;}
static void refreshLocationDialog(const std::string&message,bool useCached){
 if(!locationWindow||!IsWindow(locationWindow))return;std::string code=locations::extractZip(controlText(zipEdit));
 if(!locations::valid(pendingLocation)||pendingLocation["zip"].get<std::string>()!=code)pendingLocation=json();
 if(useCached&&pendingLocation.empty()&&locationMeta.contains(code)&&locations::valid(locationMeta[code]))pendingLocation=locationMeta[code];
 bool ready=locations::valid(pendingLocation)&&!busy;EnableWindow(GetDlgItem(locationWindow,IDOK),ready);EnableWindow(applyButton,!busy);EnableWindow(GetDlgItem(locationWindow,704),!busy&&std::find(savedZips.begin(),savedZips.end(),code)!=savedZips.end());
 std::string display=message;if(display.empty())display=busy?"Request in progress. You can cancel and continue using the player.":locations::valid(pendingLocation)?locations::label(pendingLocation):"Enter a five-digit US ZIP, choose Look Up ZIP, then confirm the resolved location.";
 setText(GetDlgItem(locationWindow,703),display);SendMessage(locationWindow,DM_SETDEFID,ready?IDOK:702,0);
}
static INT_PTR CALLBACK locationDialog(HWND h,UINT msg,WPARAM w,LPARAM){
 if(msg==WM_INITDIALOG){locationWindow=h;zipEdit=GetDlgItem(h,701);applyButton=GetDlgItem(h,702);++locationSerial;pendingLocation=json();selectedLocation=json();
  for(const auto&code:savedZips)SendMessageW(zipEdit,CB_ADDSTRING,0,(LPARAM)wide(locationLabel(code)).c_str());
  if(validZip(zip))setText(zipEdit,locationLabel(zip));else setText(zipEdit,"");refreshLocationDialog();SetFocus(zipEdit);return FALSE;
 }
 if(msg==WM_COMMAND){int command=LOWORD(w);
  if(command==IDCANCEL){EndDialog(h,IDCANCEL);return TRUE;}
  if(command==701&&(HIWORD(w)==CBN_EDITCHANGE||HIWORD(w)==CBN_SELCHANGE)){
   if(HIWORD(w)==CBN_SELCHANGE){int selection=(int)SendMessage(zipEdit,CB_GETCURSEL,0,0);if(selection>=0){int n=(int)SendMessageW(zipEdit,CB_GETLBTEXTLEN,selection,0);std::vector<wchar_t>v(n+1);SendMessageW(zipEdit,CB_GETLBTEXT,selection,(LPARAM)v.data());SetWindowTextW(zipEdit,v.data());}}
   refreshLocationDialog();return TRUE;
  }
  if(command==704){std::string code=locations::extractZip(controlText(zipEdit));if(!busy&&std::find(savedZips.begin(),savedZips.end(),code)!=savedZips.end()&&MessageBoxA(h,"Remove this saved location? Displayed weather is kept.","Locations",MB_YESNO|MB_ICONQUESTION)==IDYES){savedZips.erase(std::remove(savedZips.begin(),savedZips.end(),code),savedZips.end());locationMeta.erase(code);saveExtraPrefs();saveLocations();SendMessage(zipEdit,CB_RESETCONTENT,0,0);for(const auto&z:savedZips)SendMessageW(zipEdit,CB_ADDSTRING,0,(LPARAM)wide(locationLabel(z)).c_str());setText(zipEdit,"");pendingLocation=json();refreshLocationDialog();updateMenus();}return TRUE;}
  if(command==702){if(busy)return TRUE;std::string code=locations::extractZip(controlText(zipEdit));if(!validZip(code)){pendingLocation=json();refreshLocationDialog("Enter exactly five digits, including any leading zero.",false);SetFocus(zipEdit);return TRUE;}
   pendingLocation=json();if(startRequest(code,1))refreshLocationDialog("Looking up the ZIP. Nothing is saved until you choose Use Location.",false);else refreshLocationDialog(lastError,false);return TRUE;
  }
  if(command==IDOK){if(busy||!locations::valid(pendingLocation))return TRUE;if(locations::extractZip(controlText(zipEdit))!=pendingLocation["zip"].get<std::string>()){refreshLocationDialog("The ZIP changed. Look up the new code first.",false);return TRUE;}
   selectedLocation=pendingLocation;EndDialog(h,IDOK);return TRUE;
  }
 }
 if(msg==WM_CLOSE){EndDialog(h,IDCANCEL);return TRUE;}return FALSE;
}
static void showLocationDialog(){
 if(locationWindow){SetForegroundWindow(locationWindow);return;}
 INT_PTR result=DialogBoxW(GetModuleHandle(0),MAKEINTRESOURCEW(700),mainWindow,locationDialog);locationWindow=zipEdit=applyButton=0;++locationSerial;
 if(result==IDOK&&locations::valid(selectedLocation)){exitWeatherPreview();std::string code=selectedLocation["zip"].get<std::string>();locationMeta[code]=selectedLocation;rememberZip(code);saveLocations();startRequest(code,0,selectedLocation,true);updateMenus();}
}
static std::string diagnostics(){
 std::ostringstream out;out<<"Retro Weather v1.2.1 (32-bit XP SP3 / Pentium III target)\r\nBuild date: "<<buildDate<<"\r\n";
 typedef LONG(WINAPI*RtlVersion)(OSVERSIONINFOW*);RtlVersion version=(RtlVersion)GetProcAddress(GetModuleHandleA("ntdll.dll"),"RtlGetVersion");OSVERSIONINFOW os={};os.dwOSVersionInfoSize=sizeof(os);if(version&&version(&os)==0)out<<"Windows NT: "<<os.dwMajorVersion<<'.'<<os.dwMinorVersion<<" build "<<os.dwBuildNumber<<"\r\n";
 HKEY key;if(RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){char name[256]={};DWORD bytes=sizeof(name)-1,type=0;if(RegQueryValueExA(key,"ProcessorNameString",0,&type,(BYTE*)name,&bytes)==ERROR_SUCCESS&&type==REG_SZ)out<<"CPU: "<<name<<"\r\n";RegCloseKey(key);}
 SYSTEM_INFO si;GetSystemInfo(&si);out<<"Logical CPUs: "<<si.dwNumberOfProcessors<<" | SSE2 available: "<<(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)?"yes":"no")<<"\r\n";
 MEMORYSTATUSEX memory={};memory.dwLength=sizeof(memory);if(GlobalMemoryStatusEx(&memory))out<<"RAM total/available: "<<memory.ullTotalPhys/(1024*1024)<<" / "<<memory.ullAvailPhys/(1024*1024)<<" MB\r\n";
 auto mem=memoryPoint(true);if(mem.available)out<<"Process working set: "<<mem.workingKB<<" KB | Process commit (PagefileUsage): "<<mem.commitKB<<" KB\r\n";out<<"GUI resources: GDI "<<mem.gdi<<" | USER "<<mem.user<<" | Loaded modules: "<<mem.modules<<"\r\n";auto buffers=backBufferStats();out<<"Tracked render buffers: "<<buffers.live<<" DIBs | Live "<<buffers.liveBytes/1024<<" KB | Peak simultaneous "<<buffers.peakBytes/1024<<" KB | Successful allocations "<<buffers.allocations<<"\r\n";out<<"Foreground scratch: "<<foregroundScratch.width<<" x "<<foregroundScratch.height<<" | Transition storage: "<<transitionPixels.capacity()*4/1024<<" KB | Tracked render/cache/audio storage: "<<trackedRenderBytes()/1024<<" KB\r\n";

 FILETIME created,exited,kernel,user;if(GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel,&user)){ULARGE_INTEGER k,u;k.LowPart=kernel.dwLowDateTime;k.HighPart=kernel.dwHighDateTime;u.LowPart=user.dwLowDateTime;u.HighPart=user.dwHighDateTime;out<<"Process CPU time: "<<(k.QuadPart+u.QuadPart)/10000000.<<" seconds (cumulative)\r\n";}
 RECT rc;GetClientRect(mainWindow,&rc);auto v=viewport(mainWindow);auto f=crtMode?presentation::Frame():presentation::frameFor(v);out<<"Client: "<<rc.right<<" x "<<rc.bottom<<" | DPI: "<<windowDpi(mainWindow)<<"\r\n";
 out<<"Aspect: "<<presentation::aspectName(aspectMode)<<" | Picture: "<<v.width<<" x "<<v.height<<"\r\nLogical layout: "<<f.width<<" x "<<f.height<<" | "<<(crtMode?"low-memory":"native-resolution foreground")<<"\r\n";
 out<<"Last frame: "<<(lastPartialFrame?"changed regions":"full composition")<<" | Changed area: "<<lastDirtyPercent<<"% | Cached fonts: "<<fontCache.size()<<"\r\n";out<<"Render policy: "<<(legacyRendering?"legacy/no-SSE2":"standard")<<" | Last composition: "<<lastPaintMs<<" ms | Animation rest: "<<lastAnimationDelay<<" ms\r\n";out<<"Render buffers: foreground "<<pictureBuffer.width<<" x "<<pictureBuffer.height<<" | scenery "<<sceneBuffer.width<<" x "<<sceneBuffer.height<<"\r\n";out<<"Background: "<<backgroundNames[background]<<" | Animation: "<<"automatic adaptive"<<" | Graphics: "<<sceneQualityName()<<" (automatic)\r\n";out<<"Animation FPS: ";if(IsIconic(mainWindow)||!isScenic()||saverPreview)out<<"inactive";else if(frameMeter.ready)out<<frameMeter.fps<<" (measured, recent ~2-second window)";else out<<"collecting";out<<" | Scene drawing: "<<lastSceneMs<<" ms"<<(sceneCold?" (cache rebuild)":" (warm)")<<" | Scene cache builds: "<<sceneCacheBuilds<<" | Foreground cache builds: "<<foregroundBuilds<<" | Foreground cache: "<<foregroundCache.bytes()/1024<<" KB | Cloud sprites: "<<cloudCacheBytes()/1024<<" KB\r\n";out<<"Rain drawing: direct clipped DIB streaks (GDI fallback if unavailable)\r\n";out<<"Scene detail (last frame ms): precipitation "<<lastPrecipitationMs<<" | remaining sky/clouds/effects "<<lastCloudSkyMs<<"\r\n";out<<"Cloud blend: "<<(!legacyRendering&&RW_BLEND_SSE2?"SSE2":"scalar")<<"\r\n";out<<"Frame stages (ms): scenery "<<lastSceneMs<<" | scale "<<lastScaleMs<<" | overlay "<<lastOverlayMs<<" | present "<<lastPresentMs<<" | foreground rebuild "<<lastCacheMs<<"\r\n";out<<"Warm-frame stage averages ("<<renderProfile.samples<<" samples, smoothed): "<<renderProfile.average[0]<<" / "<<renderProfile.average[1]<<" / "<<renderProfile.average[2]<<" / "<<renderProfile.average[3]<<" ms (scene/scale/overlay/present)\r\n";out<<"Scaling: "<<scaleBackend<<" | Comparison: "<<(!scaleComparisonActive?"not needed":scaleChoice.complete()?"complete":"collecting warm frames")<<" | Median GDI / bilinear: "<<(scaleChoice.median(0)<0?"pending":std::to_string(scaleChoice.median(0)))<<" / "<<(scaleChoice.median(1)<0?"pending":std::to_string(scaleChoice.median(1)))<<" ms | Scaling tables: "<<smoothScaler.bytes()/1024<<" KB\r\n";out<<"CRT profile: "<<(crtMode?"ON - full output raster for 4:3 CRT, 2% content-only inset, 18px minimum logical text, 640x480 foreground, lightweight scenery":"off")<<"\r\n";out<<"Normal saved safe margin: "<<safeMargin<<"% | Text: "<<textPercent<<"% | Fullscreen: "<<(full?"yes":"no")<<"\r\n";
 out<<"Page: "<<features::pageNames[cycle.page<features::pageCount?cycle.page:0]<<" | Cycling paused: "<<(cycle.paused?"yes":"no")<<"\r\n";
 out<<"Effective/default page duration: "<<options::pageSeconds(cycle.page,duration,pageDurations)<<" / "<<duration<<" sec | Cycle pause: "<<pauseDuration<<" sec\r\n";out<<"Units: "<<options::temperatureLabel(celsius)<<" / "<<options::windLabel(windUnit)<<"\r\n";
 out<<"Recent foreground rebuild memory (KB; t+seconds / page / rebuild / working / commit / tracked / GDI / USER):\r\n";for(size_t i=0;i<memoryHistory.count;++i){const auto&m=memoryHistory.at(i);out<<m.elapsed<<" / "<<(m.page<features::pageCount?features::pageNames[m.page]:"Cycle pause")<<" / "<<m.builds<<" / "<<m.workingKB<<" / "<<m.commitKB<<" / "<<m.trackedKB<<" / "<<m.gdi<<" / "<<m.user<<"\r\n";}out<<"Tracked storage excludes OS/driver/font internals, JSON and allocator overhead. Working set alone does not establish a leak.\r\n";
 out<<"UI memory events (sequence / t+seconds / event / working KB / commit KB / tracked KB / GDI / USER / loaded modules):\r\n";for(size_t i=0;i<uiMemoryEvents.count;++i){const auto&e=uiMemoryEvents.at(i);const auto&m=e.memory;out<<e.sequence<<" / "<<m.elapsed<<" / "<<memory_audit::eventName(e.event)<<" / "<<m.workingKB<<" / "<<m.commitKB<<" / "<<m.trackedKB<<" / "<<m.gdi<<" / "<<m.user<<" / "<<m.modules<<"\r\n";}out<<"Export includes the current destination-ready event. File-write/notification events and diagnostics-close events appear on the next refresh/export. Module count is unavailable if zero.\r\n";
 out<<"Location configured: "<<(validZip(zip)?"yes":"no")<<" | Saved locations: "<<savedZips.size()<<"\r\n";
 out<<"Request running: "<<(busy?"yes":"no")<<" | Update failed: "<<(lastUpdateFailed?"yes":"no")<<"\r\n";
 if(!data.empty()){int64_t fetched=data.value("fetched",0LL);auto d=calendar::fromEpoch(fetched);out<<"Last successful fetch (UTC): "<<calendar::dateString(d)<<' '<<presentation::clockText(d.hour,d.minute,true)<<"\r\nWeather age: "<<(nowEpoch()-fetched)/60<<" minutes\r\n";}
 out<<"Last helper exit: "<<lastHelperExit<<" | Last HTTP status: "<<lastHttp<<"\r\n";
 out<<"Last application/weather error: "<<(lastError.empty()?"none":lastError)<<"\r\n";
 out<<"Last request error: "<<(lastRequestError.empty()?"none":lastRequestError)<<"\r\n";
 out<<"Display shift (pixels): "<<displayShift().x<<", "<<displayShift().y<<" | interval: 45 sec | burn-in reduction, not prevention\r\n";out<<"Benchmark simulation: "<<(previewMode?"SIMULATED":"off")<<" | Internal scene index: "<<previewIndex<<"\r\n";out<<"Weather scene: "<<weather_theme::name(sceneCondition)<<" | Location daylight: "<<(sceneDay?"day":"night")<<"\r\n";out<<"Audio: "<<(audio.isPaused()?"paused":audio.isActive()?"playing":"not playing")<<" | Audio enabled: "<<(audio.enabled?"yes":"no")<<" | Volume: "<<audio.volume<<"%\r\n";
 out<<"Audio transition: "<<(audio.transitioning()?"crossfade":"steady")<<" | Last volume API result: "<<audio.volumeResult<<"\r\n";out<<"Mode: "<<(saverRun?"screensaver":saverPreview?"screensaver preview":"player")<<"\r\n";out<<"Screensaver/sleep prevention requested in normal player mode only.\r\n";
 out<<"\r\nReport omits ZIP/city, usernames, file paths and sound filenames.\r\nCompatibility is a test target, not certification of all XP-11 systems.\r\nWeather: Open-Meteo (CC BY 4.0); locations: Zippopotam.us.\r\n";return lastBenchmarkReport.empty()?out.str():lastBenchmarkReport+"\r\nPOST-RUN SYSTEM SNAPSHOT\r\n"+out.str();
}
static bool diagnosticFolder(const std::wstring& path){
 if(CreateDirectoryW(path.c_str(),0))return true;
 if(GetLastError()!=ERROR_ALREADY_EXISTS)return false;
 DWORD attributes=GetFileAttributesW(path.c_str());return attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_DIRECTORY);
}
static void quickExportReport(HWND h,bool fullBenchmark=false){
 recordUIEvent(memory_audit::Event::QuickBefore);
 wchar_t documents[MAX_PATH]={};std::wstring path;HANDLE file=INVALID_HANDLE_VALUE;DWORD error=0;
 if(FAILED(SHGetFolderPathW(h,CSIDL_PERSONAL|CSIDL_FLAG_CREATE,0,SHGFP_TYPE_CURRENT,documents)))error=ERROR_PATH_NOT_FOUND;
 else{
  std::wstring root=std::wstring(documents)+L"\\Retro Weather",directory=root+L"\\Diagnostics";
  if(!diagnosticFolder(root)||!diagnosticFolder(directory))error=GetLastError();
  else{
   SYSTEMTIME now;GetLocalTime(&now);
   for(unsigned attempt=0;attempt<1000;++attempt){
    path=directory+L"\\"+diagnostic_export::filename(now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,now.wMilliseconds,attempt,fullBenchmark);
    if(fullBenchmark&&!benchmarkExportStem.empty()&&attempt==0)path=benchmarkExportStem+L".txt";

    file=CreateFileW(path.c_str(),GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);
    if(file!=INVALID_HANDLE_VALUE)break;error=GetLastError();if(error!=ERROR_FILE_EXISTS&&error!=ERROR_ALREADY_EXISTS)break;
   }
  }
 }
 bool ok=file!=INVALID_HANDLE_VALUE,cleanupOK=true;
 if(ok){
  recordUIEvent(memory_audit::Event::QuickReady);
  std::string latest=diagnostics();std::wstring report=wide(latest);
  WORD bom=0xfeff;DWORD written=0,bytes=(DWORD)(report.size()*sizeof(wchar_t));
  ok=WriteFile(file,&bom,2,&written,0)&&written==2;
  if(ok)ok=WriteFile(file,report.data(),bytes,&written,0)&&written==bytes;
  if(ok)ok=FlushFileBuffers(file)!=FALSE;
  if(!ok)error=GetLastError();
  if(!CloseHandle(file)){if(ok)error=GetLastError();ok=false;}
  if(!ok)cleanupOK=DeleteFileW(path.c_str())!=FALSE;
 }
 recordUIEvent(ok?memory_audit::Event::QuickWritten:memory_audit::Event::QuickFailed);
 std::wstring notice=ok?L"Diagnostics saved to:\n"+path:L"Could not save diagnostics. Windows error: "+std::to_wstring(error)+L".\nOpen Tools > Diagnostics / Benchmark and use Export Latest Report to retry.";
 if(!cleanupOK)notice+=L"\nIncomplete file could not be removed:\n"+path;
 MessageBoxW(h,notice.c_str(),L"Quick Export Diagnostics",MB_OK|(ok?MB_ICONINFORMATION:MB_ICONERROR));
 recordUIEvent(memory_audit::Event::NoticeClosed);
}
static uint64_t processCpuTime(){FILETIME a,b,k,u;if(!GetProcessTimes(GetCurrentProcess(),&a,&b,&k,&u))return 0;ULARGE_INTEGER x,y;x.LowPart=k.dwLowDateTime;x.HighPart=k.dwHighDateTime;y.LowPart=u.dwLowDateTime;y.HighPart=u.dwHighDateTime;return x.QuadPart+y.QuadPart;}

struct RecordingChoice{bool enabled=false,audio=true;int resolution=2;};
static RecordingChoice recordingChoice;
static INT_PTR CALLBACK recordingOptionsDialog(HWND h,UINT msg,WPARAM w,LPARAM){if(msg==WM_INITDIALOG){CheckDlgButton(h,1201,BST_UNCHECKED);CheckDlgButton(h,1204,BST_CHECKED);for(int i=0;i<recording_plan::resolutionCount;++i)SendDlgItemMessageA(h,1202,CB_ADDSTRING,0,(LPARAM)recording_plan::label(i));SendDlgItemMessage(h,1202,CB_SETCURSEL,2,0);EnableWindow(GetDlgItem(h,1206),!lastBenchmarkReport.empty());bool available=VideoRecorder::available();EnableWindow(GetDlgItem(h,1201),available);EnableWindow(GetDlgItem(h,1202),available);EnableWindow(GetDlgItem(h,1204),available);if(!available)SetDlgItemTextA(h,1205,"MP4 unavailable on this Windows installation. XP uses text diagnostics only.");return TRUE;}if(msg==WM_COMMAND){if(LOWORD(w)==1206){quickExportReport(h,true);return TRUE;}if(LOWORD(w)==IDCANCEL){EndDialog(h,IDCANCEL);return TRUE;}if(LOWORD(w)==IDOK){recordingChoice.enabled=IsDlgButtonChecked(h,1201)==BST_CHECKED&&VideoRecorder::available();recordingChoice.resolution=recording_plan::resolutionIndex((int)SendDlgItemMessage(h,1202,CB_GETCURSEL,0,0));recordingChoice.audio=IsDlgButtonChecked(h,1204)==BST_CHECKED;EndDialog(h,IDOK);return TRUE;}}return FALSE;}
struct BenchmarkRun {
 benchmark::Timeline timeline;benchmark::Frames frames;DWORD wallStart=0,measureAt=0;uint64_t cpuStart=0;bool measuring=false;unsigned measuredFrames=0,pageMask=0;bool shiftOK=true,audioOK=true;memory_audit::Point before,peak;
 std::vector<std::string> results;features::Cycle savedCycle;int savedCrtPart=0;unsigned crtPanels[2]={};rendering::Quality savedQuality;bool savedEnabled=true;int savedVolume=25;int controls[5]={-1,-1,-1,-1,-1};std::string header,fault;
 bool replay=false,recordReady=false,recordFinishing=false,recordCancelled=false;DWORD recorderStarted=0,replayStarted=0,recordMemoryTick=0;uint64_t recordCpuStart=0,recordPeakKB=0;int replayScene=-1,replayPage=-1,captureWidth=0,captureHeight=0;uint64_t lastCapture=~uint64_t(0);unsigned captureAttempts=0;std::string recordFault;std::wstring videoPath;VideoRecorder::Options videoOptions;std::unique_ptr<VideoRecorder>recorder;
};
static BenchmarkRun benchmarkRun;
static void benchmarkFrame(double ms){if(!benchmarkActive||benchmarkRun.replay)return;auto&b=benchmarkRun;double values[]={lastSceneMs,lastScaleMs,lastOverlayMs,lastPresentMs,lastPrecipitationMs};b.frames.add(ms,values,!b.measuring||sceneCold||frameRebuiltForeground,b.timeline.stage==0?0:sceneBuffer.width,b.timeline.stage==0?0:sceneBuffer.height,b.timeline.stage==0?-1:sceneTier());if(b.measuring)++b.measuredFrames;if(cycle.page>=0&&cycle.page<features::pageCount)b.pageMask|=1u<<cycle.page;if(crtMode&&(cycle.page==2||cycle.page==7))b.crtPanels[cycle.page==7?1:0]|=1u<<crtPart;if(foregroundReady){auto d=displayShift();b.shiftOK=b.shiftOK&&foregroundLeft+d.x>=0&&foregroundTop+d.y>=0&&foregroundRight+d.x<=pictureBuffer.width&&foregroundBottom+d.y<=pictureBuffer.height;}}
static void benchmarkSampleMemory(){auto&b=benchmarkRun;auto p=memoryPoint();b.peak.workingKB=std::max(b.peak.workingKB,p.workingKB);b.peak.commitKB=std::max(b.peak.commitKB,p.commitKB);b.peak.trackedKB=std::max(b.peak.trackedKB,p.trackedKB);b.peak.gdi=std::max(b.peak.gdi,p.gdi);b.peak.user=std::max(b.peak.user,p.user);}
static void benchmarkEnterStage(){auto&b=benchmarkRun;b.frames=benchmark::Frames();b.measuring=false;b.measuredFrames=0;b.audioOK=true;int stage=b.timeline.stage;
 if(stage==0){previewSession.enter(0,data,background,nowEpoch());background=0;cycle.page=0;cycle.paused=false;syncWeatherAudio();syncSceneTimer(mainWindow);repaint(mainWindow);}
 else if(stage<10){useWeatherPreview(stage-1);cycle.paused=false;audio.setPaused(false);}
 else {cycle.paused=true;audio.setPaused(true);b.controls[0]=audio.isPaused()?1:0;}
 b.before=memoryPoint(true);b.peak=b.before;
}
static void benchmarkFinishStage(bool partial){auto&b=benchmarkRun;unsigned now=GetTickCount(),elapsed=b.measuring?now-b.measureAt:0;uint64_t cpu=processCpuTime();SYSTEM_INFO si;GetSystemInfo(&si);auto after=memoryPoint(true);benchmarkSampleMemory();std::ostringstream out;out<<"\r\n["<<benchmark::name(b.timeline.stage)<<"] "<<(partial?"PARTIAL / CANCELLED":"COMPLETE")<<"\r\n";
 out<<"Measured wall time: "<<elapsed/1000.<<" s | rendered compositions: "<<b.measuredFrames;if(elapsed)out<<" | rendered FPS: "<<b.measuredFrames*1000./elapsed;out<<"\r\n";
 if(b.measuring&&elapsed&&cpu&&b.cpuStart)out<<"Process CPU: "<<options::cpuPercent(cpu>=b.cpuStart?cpu-b.cpuStart:0,elapsed,si.dwNumberOfProcessors)<<"% all cores | "<<options::cpuPercent(cpu>=b.cpuStart?cpu-b.cpuStart:0,elapsed,1)<<"% single-core equivalent\r\n";else out<<"CPU sample: NOT AVAILABLE (no measured interval or CPU timing unavailable)\r\n";
 out<<"Steady warm frames: "<<b.frames.frames<<" | average / p95 / maximum composition ms: "<<(b.frames.frames?b.frames.elapsedSum/b.frames.frames:0)<<" / "<<b.frames.percentile(.95)<<" / "<<b.frames.maximum<<"\r\n";
 out<<"Settling/cache/page-rebuild frames: "<<b.frames.transient<<" | maximum composition ms: "<<b.frames.transientMax<<"\r\n";
 out<<"Steady stage averages ms (scene / scale / overlay / present / precipitation): ";for(int i=0;i<5;++i)out<<(i?" / ":"")<<(b.frames.frames?b.frames.sums[i]/b.frames.frames:0);out<<"\r\n";
 out<<"Scenery at last warm frame: "<<b.frames.width<<" x "<<b.frames.height<<" | quality tier: "<<b.frames.tier<<" | measured geometry/quality changes: "<<b.frames.geometryChanges<<" | foreground: "<<pictureBuffer.width<<" x "<<pictureBuffer.height<<"\r\n";
 if(b.frames.geometryChanges)out<<"Comparison note: adaptive quality changed; this is a mixed-quality sample.\r\n";
 if(!b.before.available||!after.available)out<<"Memory API unavailable for one or more endpoints; zero values may mean unavailable.\r\n";out<<"Memory KB before / after / peak: working "<<b.before.workingKB<<" / "<<after.workingKB<<" / "<<b.peak.workingKB<<"; commit "<<b.before.commitKB<<" / "<<after.commitKB<<" / "<<b.peak.commitKB<<"; tracked "<<b.before.trackedKB<<" / "<<after.trackedKB<<" / "<<b.peak.trackedKB<<"\r\n";
 out<<"GDI before/after/peak: "<<b.before.gdi<<" / "<<after.gdi<<" / "<<b.peak.gdi<<" | USER: "<<b.before.user<<" / "<<after.user<<" / "<<b.peak.user<<" | modules before/after: "<<b.before.modules<<" / "<<after.modules<<"\r\n";
 out<<"Rendering: "<<(b.timeline.stage==0?"BASELINE (no continuous animation expected)":b.measuredFrames?"PASS - frames observed":"NOT MEASURED")<<"\r\n";
 bool expected=b.timeline.stage!=0;bool audioState=b.timeline.stage==10?true:(expected?audio.isActive()&&!audio.isPaused():!audio.isActive());b.audioOK=b.audioOK&&audio.volumeResult==0;
 out<<"Audio/API state: "<<(audioState&&b.audioOK?"PASS":"FAIL")<<" | "<<audio.title()<<" | volume API result: "<<audio.volumeResult<<" | audibility/loop continuity: MANUAL LISTENING REQUIRED\r\n";
 out<<"Sample window includes >17 s sound loop: "<<(elapsed>=17000&&expected?"yes (configured hardware loop; listen to confirm)":"no / not applicable")<<"\r\n";
 out<<"Last live weather request error present: "<<(!lastRequestError.empty()?"yes":"no")<<" (network updates suspended during benchmark)\r\n";b.results.push_back(out.str());
}
static bool reserveRecordingPath(){auto&b=benchmarkRun;wchar_t documents[MAX_PATH]={};if(FAILED(SHGetFolderPathW(mainWindow,CSIDL_PERSONAL|CSIDL_FLAG_CREATE,0,SHGFP_TYPE_CURRENT,documents)))return false;std::wstring root=std::wstring(documents)+L"\\Retro Weather",directory=root+L"\\Diagnostics";if(!diagnosticFolder(root)||!diagnosticFolder(directory))return false;SYSTEMTIME now;GetLocalTime(&now);for(unsigned n=0;n<1000;++n){std::wstring name=diagnostic_export::filename(now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,now.wMilliseconds,n,true);std::wstring stem=directory+L"\\"+name.substr(0,name.size()-4);if(GetFileAttributesW((stem+L".txt").c_str())!=INVALID_FILE_ATTRIBUTES)continue;HANDLE file=CreateFileW((stem+L".mp4").c_str(),GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);if(file!=INVALID_HANDLE_VALUE){CloseHandle(file);benchmarkExportStem=stem;b.videoPath=stem+L".mp4";return true;}if(GetLastError()!=ERROR_FILE_EXISTS&&GetLastError()!=ERROR_ALREADY_EXISTS)return false;}return false;}
static void beginRecording(HWND h){auto&b=benchmarkRun;b.replay=true;b.measuring=false;b.recorderStarted=GetTickCount();b.recordCpuStart=processCpuTime();if(!reserveRecordingPath()){b.recordFault="Could not create a new recording file in Documents. Text results are retained.";EndDialog(h,IDOK);return;}auto size=recording_plan::fit(pictureBuffer.width,pictureBuffer.height,recordingChoice.resolution);b.captureWidth=pictureBuffer.width;b.captureHeight=pictureBuffer.height;b.videoOptions.width=size.width;b.videoOptions.height=size.height;b.videoOptions.audio=recordingChoice.audio;b.videoOptions.volume=b.savedVolume?b.savedVolume:25;b.videoOptions.bitrate=recording_plan::bitrate(size.width,size.height);b.recorder.reset(new VideoRecorder);if(!b.recorder->start(b.videoPath,base+"\\Sounds",b.videoOptions)){b.recordFault="Could not start MP4 recorder. Try a lower resolution; the text benchmark is retained.";DeleteFileW(b.videoPath.c_str());EndDialog(h,IDOK);return;}KillTimer(h,110);SetTimer(h,110,100,0);SetDlgItemTextA(h,1001,"Timed benchmark complete.\r\nStarting optional MP4 encoder...\r\nRecording is excluded from the timed results.");}
static void recordingTick(HWND h,DWORD now){auto&b=benchmarkRun;if(!b.recorder){EndDialog(h,IDOK);return;}auto status=b.recorder->status();if((DWORD)(now-b.recordMemoryTick)>=500){b.recordMemoryTick=now;b.recordPeakKB=std::max(b.recordPeakKB,memoryPoint().workingKB);}if(status.done){EndDialog(h,IDOK);return;}if(!status.ready){if((DWORD)(now-b.recorderStarted)>20000&&!b.recordFinishing){b.recordFault="Encoder initialization exceeded 20 seconds; recording stopped.";b.recordFinishing=true;b.recorder->finish();}return;}if(b.recordFinishing){SetDlgItemTextA(h,1001,"Finalizing MP4...\r\nTimed results are already complete.\r\nOriginal settings will be restored before export.");return;}
 if(!b.recordReady){b.recordReady=true;b.replayStarted=now;}
 unsigned elapsed=now-b.replayStarted;if(elapsed>=recording_plan::totalMs){b.recordFinishing=true;b.recorder->finish();SetDlgItemTextA(h,1001,"Replay complete. Finalizing MP4...");return;}
 unsigned sceneIndex=recording_plan::stage(elapsed),page=recording_plan::page(elapsed);int part=crt_profile::part((elapsed%recording_plan::sceneMs)%1600,1600);if(crtMode&&part!=crtPart){crtPart=part;if(page==2||page==7)repaint(mainWindow);}if((int)sceneIndex!=b.replayScene){b.replayScene=sceneIndex;b.replayPage=-1;recordingSceneName=benchmark::name(sceneIndex+1);useWeatherPreview(sceneIndex);}if((int)page!=b.replayPage){b.replayPage=page;cycle.page=page;repaint(mainWindow);}UpdateWindow(mainWindow);
 if(IsIconic(mainWindow)||pictureBuffer.width!=b.captureWidth||pictureBuffer.height!=b.captureHeight){b.recordFault="Recording stopped because the player was minimized or render dimensions changed.";b.recordFinishing=true;b.recorder->finish();return;}
 uint64_t frame=elapsed/100;if(frame!=b.lastCapture&&!contentDirty&&!frameDirty){b.lastCapture=frame;++b.captureAttempts;GdiFlush();b.recorder->enqueue(pictureBuffer.pixels,pictureBuffer.width,pictureBuffer.height,frame,sceneIndex,sceneIndex*(recording_plan::sceneMs/100));}
 std::ostringstream text;text<<"Timed benchmark complete - MP4 replay\r\nScene "<<sceneIndex+1<<" / 9: "<<benchmark::name(sceneIndex+1)<<" | page "<<page+1<<" / 9\r\nReplay: "<<elapsed/1000<<" / 144 sec | "<<b.videoOptions.width<<" x "<<b.videoOptions.height<<" / 10 FPS\r\nVideo frames written: "<<status.written<<" | queue drops: "<<status.dropped<<"\r\nNo file-size cap. Only weather picture captured.\r\nCancel stops recording and keeps the timed report.";setText(GetDlgItem(h,1001),text.str());}
static std::string finishRecordingReport(){auto&b=benchmarkRun;std::ostringstream out;out<<"\r\nOPTIONAL VIDEO RECORDING\r\nRequested: "<<(recordingChoice.enabled?"yes":"no")<<"\r\n";if(!recordingChoice.enabled)return out.str();if(!b.replay){out<<"NOT RUN - benchmark ended before the recording replay.\r\n";return out.str();}if(!b.recordFault.empty())out<<b.recordFault<<"\r\n";if(b.recorder){auto status=b.recorder->status();out<<"Status: "<<(b.recordCancelled?"PARTIAL - CANCELLED":status.error?"FAILED":!b.recordFault.empty()?"PARTIAL - STOPPED":"FINALIZED")<<" | encoder HRESULT: 0x"<<std::hex<<(unsigned long)status.error<<std::dec<<"\r\n";SYSTEM_INFO si;GetSystemInfo(&si);DWORD wall=GetTickCount()-b.recorderStarted;uint64_t cpu=processCpuTime();if(wall&&cpu>=b.recordCpuStart)out<<"Recording-only process CPU: "<<options::cpuPercent(cpu-b.recordCpuStart,wall,si.dwNumberOfProcessors)<<"% all cores | sampled peak working KB: "<<b.recordPeakKB<<" (excluded from timed benchmark)\r\n";out<<"Resolution: "<<b.videoOptions.width<<" x "<<b.videoOptions.height<<" | target FPS: 10 | video bitrate: "<<b.videoOptions.bitrate<<" | file-size cap: none\r\nRequested preset: "<<recording_plan::label(recordingChoice.resolution)<<" (maximum; no upscaling)\r\n";out<<"Frames submitted / written / queue drops: "<<status.queued<<" / "<<status.written<<" / "<<status.dropped<<" | missing capture time slots: "<<(status.written&&status.lastFrame+1>=status.written?status.lastFrame+1-status.written:0)<<"\r\n";out<<"Encoded frames per video second: "<<(status.written?status.written*10./(status.lastFrame+1):0)<<"\r\n";out<<"Video duration seconds: "<<(status.written?(status.lastFrame+1)/10.:0)<<" | file bytes: "<<status.bytes<<" | captured scene mask: "<<status.sceneMask<<" / 511\r\n";out<<"Captured page masks per scene (511 = all nine): ";for(int i=0;i<9;++i)out<<(i?", ":"")<<status.pageMask[i];out<<"\r\n";if(status.error)out<<"The Windows encoder could not complete this preset. Try a lower recording resolution. Text benchmark results remain available.\r\n";if(status.cleanupFailed)out<<"WARNING: Windows refused cleanup of the incomplete video. Remove the named file manually.\r\n";if((!status.error||status.cleanupFailed)&&!b.videoPath.empty())out<<"Video file: "<<utf8(b.videoPath.substr(b.videoPath.find_last_of(L"\\/")+1))<<"\r\n";out<<"Replay render quality: "<<sceneQualityName()<<" | source image: "<<b.captureWidth<<" x "<<b.captureHeight<<" | encoder selection: system-managed (hardware allowed)\r\n";out<<"Audio: "<<(b.videoOptions.audio?"AAC 44.1 kHz mono, generated from bundled sounds; not speaker/microphone capture":"not included")<<"\r\n";b.recorder.reset();}
 out<<"Encoding started after timed measurements. Post-run memory may include Windows encoder caches. Video timing/drop limits do not describe benchmark rendering performance.\r\n";return out.str();}
static INT_PTR CALLBACK benchmarkDialog(HWND h,UINT msg,WPARAM w,LPARAM){
 try{
  if(msg==WM_INITDIALOG){benchmarkWindow=h;benchmarkRun.wallStart=GetTickCount();benchmarkRun.timeline.begin(GetTickCount());benchmarkEnterStage();SetTimer(h,110,500,0);return TRUE;}
  if(msg==WM_TIMER&&w==110){auto&b=benchmarkRun;DWORD now=GetTickCount();if(b.replay){recordingTick(h,now);return TRUE;}unsigned elapsed=b.timeline.elapsed(now);int stage=b.timeline.stage;benchmarkSampleMemory();
   if(elapsed>=benchmark::settling&&!b.measuring){b.measuring=true;b.measureAt=now;b.cpuStart=processCpuTime();}
   int page=benchmark::page(stage,elapsed);if(page>=0){int part=crt_profile::part(elapsed%3000,3000);if(crtMode&&crtPart!=part){crtPart=part;if(page==2||page==7)repaint(mainWindow);}if(page!=cycle.page){cycle.page=page;repaint(mainWindow);}}
   else if(stage==10){if(elapsed>=4000&&b.controls[1]<0){cycle.paused=false;audio.setPaused(false);b.controls[1]=audio.isActive()&&!audio.isPaused();}if(elapsed>=8000&&b.controls[2]<0){audio.setEnabled(false);b.controls[2]=!audio.isActive();}if(elapsed>=12000&&b.controls[3]<0){audio.setEnabled(true,false);b.controls[3]=audio.isActive();}if(elapsed>=16000&&b.controls[4]<0)b.controls[4]=audio.isActive()&&!audio.isPaused()&&audio.volumeResult==0;}
   if(stage>0&&stage<10&&elapsed>2500)b.audioOK=b.audioOK&&audio.isActive()&&!audio.isPaused()&&audio.volumeResult==0;
   std::ostringstream progress;progress<<"Stage "<<stage+1<<" / "<<benchmark::stages<<": "<<benchmark::name(stage)<<"\r\n"<<(elapsed<benchmark::settling?"Settling":"Measuring")<<" - "<<elapsed/1000<<" / "<<benchmark::duration(stage)/1000<<" sec\r\nElapsed: "<<(now-b.wallStart)/1000<<" sec / about "<<benchmark::total()/1000<<" sec\r\nExpected audio: "<<(stage==0?"silent":stage==10?"pause, resume, off, then on":audio.title())<<"\r\nListen for sound continuity. Normal CPU budget stays active.\r\nOne report is exported after completion or cancellation.";setText(GetDlgItem(h,1001),progress.str());
   if(b.timeline.ready(now)){benchmarkFinishStage(false);b.timeline.next(now);if(b.timeline.done){if(recordingChoice.enabled){beginRecording(h);return TRUE;}EndDialog(h,IDOK);return TRUE;}benchmarkEnterStage();}return TRUE;
  }
  if((msg==WM_COMMAND&&LOWORD(w)==IDCANCEL)||msg==WM_CLOSE){if(benchmarkRun.replay){EnableWindow(GetDlgItem(h,IDCANCEL),FALSE);SetDlgItemTextA(h,1001,"Stopping recording and finalizing MP4...\r\nTimed results are retained.");benchmarkRun.recordCancelled=true;benchmarkRun.recordFinishing=true;if(benchmarkRun.recorder)benchmarkRun.recorder->finish();return TRUE;}benchmarkFinishStage(true);EndDialog(h,IDCANCEL);return TRUE;}
  if(msg==WM_DESTROY){KillTimer(h,110);benchmarkWindow=0;}
 }catch(const std::exception&){benchmarkRun.fault="Benchmark stopped after an internal error; partial results retained.";if(benchmarkRun.replay&&benchmarkRun.recorder){benchmarkRun.recordFault="Recording stopped after an internal error.";benchmarkRun.recordFinishing=true;benchmarkRun.recorder->finish();return TRUE;}EndDialog(h,IDCANCEL);return TRUE;}
 return FALSE;
}
static void runBenchmark(HWND owner){
 if(benchmarkActive||saverRun||saverPreview)return;if(busy||locationWindow||settingsOperation){MessageBoxA(owner,"Wait for the current weather/location operation to finish, then start the benchmark.","Full Benchmark",MB_OK);return;}
 if(DialogBoxW(GetModuleHandle(0),MAKEINTRESOURCEW(1200),owner,recordingOptionsDialog)!=IDOK)return;
 auto&b=benchmarkRun;b=BenchmarkRun();lastBenchmarkReport.clear();benchmarkExportStem.clear();b.header=diagnostics();b.savedCycle=cycle;b.savedCrtPart=crtPart;b.savedQuality=graphicsQuality;b.savedEnabled=audio.enabled;b.savedVolume=audio.volume;std::string originalData=data.dump();int originalBackground=background;benchmarkActive=true;settingsOperation=true;
 audio.volume=b.savedVolume?b.savedVolume:25;audio.enabled=true;cycle.paused=false;INT_PTR result=DialogBoxW(GetModuleHandle(0),MAKEINTRESOURCEW(1000),owner,benchmarkDialog);
 if(result==-1)b.fault="Benchmark dialog could not open.";
 std::string recordingReport=finishRecordingReport();recordingSceneName.clear();
 if(previewMode)exitWeatherPreview();cycle=b.savedCycle;crtPart=b.savedCrtPart;graphicsQuality=b.savedQuality;audio.volume=b.savedVolume;audio.setEnabled(b.savedEnabled,cycle.paused);syncWeatherAudio();audio.applyVolume();syncSceneTimer(mainWindow);releaseTransition();settingsOperation=false;benchmarkActive=false;benchmarkWindow=0;updateMenus();repaint(mainWindow);UpdateWindow(mainWindow);
 bool restored=data.dump()==originalData&&background==originalBackground&&audio.enabled==b.savedEnabled&&audio.volume==b.savedVolume&&cycle.page==b.savedCycle.page&&cycle.paused==b.savedCycle.paused;
 std::ostringstream report;report<<"RETRO WEATHER FULL DIAGNOSTICS / BENCHMARK\r\nRun status: "<<(result==IDOK?"COMPLETED":"CANCELLED / INTERRUPTED")<<" | completed/partial stage entries: "<<b.results.size()<<" / "<<benchmark::stages<<" | elapsed: "<<(b.wallStart?(DWORD)(GetTickCount()-b.wallStart)/1000.:0)<<" sec\r\nNormal adaptive quality, frame cap and CPU budget retained. No forced maximum-load mode.\r\nAudio was temporarily enabled at "<<(b.savedVolume?b.savedVolume:25)<<"%; original state restored afterward.\r\nMemory sampled at 500ms intervals; peaks are sampled, not absolute maxima.\r\nWarm FPS includes cache/page rebuilds; timing statistics separate cold/rebuild frames. Basic baseline is idle.\r\nAudio audibility, smoothness, lightning appearance and absence of trails: MANUAL CONFIRMATION REQUIRED.\r\nScreensaver entry/exit, minimize/restore and other display resolutions: MANUAL / NOT AUTOMATED in this run.\r\nTime-zone/live data/settings restoration: "<<(restored?"PASS":"FAIL")<<"\r\nPage rendering coverage: "<<(b.pageMask==features::allPages?"PASS - all nine pages observed":"PARTIAL")<<" | mask: "<<b.pageMask<<"\r\nCRT subpanels rendered (3 = both): details "<<b.crtPanels[0]<<" | hourly "<<b.crtPanels[1]<<" | CRT enabled: "<<(crtMode?"yes":"no")<<"\r\nShifted foreground bounds: "<<(b.shiftOK?"PASS - no out-of-bounds shift observed":"FAIL")<<"\r\n";
 const char*checks[]={"Pause API","Resume API","Audio Off releases stream","Audio On opens stream","Post-transition audio state"};for(int i=0;i<5;++i)report<<checks[i]<<": "<<(b.controls[i]<0?"NOT RUN":b.controls[i]?"PASS":"FAIL")<<"\r\n";
 report<<recordingReport;if(!b.fault.empty())report<<b.fault<<"\r\n";for(const auto&stage:b.results)report<<stage;report<<"\r\nINITIAL SYSTEM SNAPSHOT\r\n"<<b.header;lastBenchmarkReport=report.str();b=BenchmarkRun();quickExportReport(owner,true);
}
static void openScreenSaverSettings(HWND owner){OSVERSIONINFOA version={};version.dwOSVersionInfoSize=sizeof(version);GetVersionExA(&version);ShellExecuteW(owner,L"open",L"control.exe",version.dwMajorVersion<6?L"desk.cpl,,2":L"desk.cpl,,1",0,SW_SHOWNORMAL);}
static void shortcutHelp(HWND h){MessageBoxA(h,"F11 or double-click: enter/leave fullscreen\nEsc: leave fullscreen\nSpace: next weather page\nP: pause/resume audio and pages\nF5: refresh weather\nCtrl+L: locations\nM: weather audio\nF1: About Retro Weather\n\nRight-click in fullscreen for menus.\nScreensaver: M enables/disables audio; other keys or mouse exit.","Retro Weather - Keyboard Guide",MB_OK|MB_ICONINFORMATION);}
static INT_PTR CALLBACK saverConfig(HWND h,UINT msg,WPARAM w,LPARAM){if(msg==WM_INITDIALOG)return TRUE;if(msg==WM_COMMAND){int id=LOWORD(w);if(id==IDOK||id==IDCANCEL){EndDialog(h,id);return TRUE;}if(id==952){ShellExecuteW(h,L"open",wide(base+"\\RetroWeather.exe").c_str(),0,wide(base).c_str(),SW_SHOWNORMAL);return TRUE;}}return FALSE;}
static bool atomicBytes(const std::wstring&path,const std::string&bytes){std::wstring temp=path+L".writing-"+std::to_wstring(GetCurrentProcessId());HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)return false;DWORD written=0;bool ok=WriteFile(file,bytes.data(),(DWORD)bytes.size(),&written,0)&&written==bytes.size()&&FlushFileBuffers(file);CloseHandle(file);if(ok)ok=MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;if(!ok)DeleteFileW(temp.c_str());return ok;}
static std::string backupFileBytes(const std::wstring&path){HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("Could not read the selected file.");DWORD size=GetFileSize(file,0);if(size==INVALID_FILE_SIZE||size>2*1024*1024){CloseHandle(file);throw std::runtime_error("Backup is too large (maximum 2 MB).");}std::string bytes(size,'\0');DWORD read=0;bool ok=!size||ReadFile(file,&bytes[0],size,&read,0)&&read==size;CloseHandle(file);if(!ok)throw std::runtime_error("Could not read the complete file.");return bytes;}
static json captureSettings(){json settings=json::object();std::vector<std::string> keys=backup::strings();for(auto row:backup::ranges())keys.push_back(row.first);for(const auto&key:keys){wchar_t buffer[4096];GetPrivateProfileStringW(L"Settings",wide(key).c_str(),L"__RW_MISSING__",buffer,4096,wide(ini()).c_str());if(std::wstring(buffer)!=L"__RW_MISSING__")settings[key]=utf8(buffer);}return backup::validate(json{{"format","RetroWeatherSettings"},{"version",2},{"app_version","1.1-test16"},{"settings",settings},{"locations",locationMeta}});}
static bool chooseBackupFile(HWND h,bool saving,std::wstring&path){wchar_t name[MAX_PATH]=L"RetroWeather_Settings.rwbackup";OPENFILENAMEW dialog={};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=h;dialog.lpstrFilter=L"Retro Weather settings (*.rwbackup)\0*.rwbackup\0All files (*.*)\0*.*\0\0";dialog.lpstrFile=name;dialog.nMaxFile=MAX_PATH;dialog.lpstrDefExt=L"rwbackup";dialog.lpstrTitle=saving?L"Back Up Retro Weather Settings":L"Restore Retro Weather Settings";dialog.Flags=OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(saving?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);BOOL ok=saving?GetSaveFileNameW(&dialog):GetOpenFileNameW(&dialog);if(!ok){if(CommDlgExtendedError())MessageBoxA(h,"Could not open the file dialog.","Settings Backup",MB_OK|MB_ICONERROR);return false;}path=name;return true;}
static void settingsBackup(HWND h,bool restore){
 if(busy||locationWindow){MessageBoxA(h,"Wait for the weather/location request to finish, then try again.","Settings Backup",MB_OK);return;}if(saverRun||saverPreview)return;struct OperationGuard{OperationGuard(){settingsOperation=true;}~OperationGuard(){settingsOperation=false;}} guard;std::wstring path;if(!chooseBackupFile(h,!restore,path))return;
 try{savePlacement(h);savePrefs();saveExtraPrefs();saveLocations();json current=captureSettings();if(!restore){if(!atomicBytes(path,current.dump(2)))throw std::runtime_error("Could not save the backup. Check disk space and destination access.");MessageBoxA(h,"Settings and saved locations backed up. Bundled weather sounds do not need backing up.","Settings Backup",MB_OK|MB_ICONINFORMATION);return;}
 json imported=backup::validate(json::parse(backupFileBytes(path)));
 if(MessageBoxA(h,"Restore the selected settings and locations? Current settings will be backed up first.\n\nRetro Weather will close afterward. Reopen it to use the restored settings. Windows screensaver/startup settings are kept.","Restore Settings",MB_YESNO|MB_ICONQUESTION)!=IDYES)return;
 std::wstring recovery=wide(folder+"\\Before_Restore_"+std::to_string(nowEpoch())+".rwbackup");if(!atomicBytes(recovery,current.dump(2)))throw std::runtime_error("Could not create the recovery backup. Nothing has been restored.");
 std::wstring iniPath=wide(ini()),locPath=wide(folder+"\\locations.json"),staged=wide(folder+"\\restore-staged.ini");WritePrivateProfileStringW(0,0,0,iniPath.c_str());std::string oldIni=backupFileBytes(iniPath),oldLocations=backupFileBytes(locPath);
 // Build a fresh INI from validated keys only: no system paths, Run commands,
 // screensaver active/lock values, or stale crash-recovery flags are imported.
 DeleteFileW(staged.c_str());if(!atomicBytes(staged,std::string("\xff\xfe",2)))throw std::runtime_error("Could not stage settings.");bool stagedOK=true;
 for(auto it=imported["settings"].begin();it!=imported["settings"].end();++it)if(!WritePrivateProfileStringW(L"Settings",wide(it.key()).c_str(),wide(it.value().get<std::string>()).c_str(),staged.c_str()))stagedOK=false;
 WritePrivateProfileStringW(0,0,0,staged.c_str());if(!stagedOK){DeleteFileW(staged.c_str());throw std::runtime_error("Could not stage all settings.");}std::string newIni=backupFileBytes(staged);DeleteFileW(staged.c_str());
 if(!atomicBytes(iniPath,newIni))throw std::runtime_error("Could not restore settings. Original profile remains.");
 if(!atomicBytes(locPath,imported["locations"].dump(2))){bool rolledBack=atomicBytes(iniPath,oldIni)&&atomicBytes(locPath,oldLocations);if(!rolledBack){restoringSettings=true;MessageBoxW(h,(L"Restore failed and automatic rollback could not finish. Recovery backup:\n"+recovery+L"\n\nRetro Weather will close.").c_str(),L"Restore Settings",MB_OK|MB_ICONERROR);DestroyWindow(h);return;}throw std::runtime_error("Location restore failed; original settings were restored.");}
 WritePrivateProfileStringW(0,0,0,iniPath.c_str());restoringSettings=true;MessageBoxW(h,(L"Settings restored. Reopen Retro Weather.\n\nPrevious settings saved to:\n"+recovery).c_str(),L"Restore Settings",MB_OK|MB_ICONINFORMATION);DestroyWindow(h);
 }catch(const std::exception&e){MessageBoxW(h,wide(std::string("Settings operation failed: ")+e.what()).c_str(),L"Settings Backup",MB_OK|MB_ICONERROR);}
}
static bool handleOptions(HWND h,int command){if(previewMode&&((command>=180&&command<200)||(command>=350&&command<=354)||command==126||command==171))exitWeatherPreview();
 if(command==164||command==165){settingsBackup(h,command==165);return true;}
 if(command==166){runBenchmark(h);return true;}
 if(command==161){shortcutHelp(h);return true;}
 if(command==167){ShellExecuteW(h,L"open",wide(base+"\\DATA_LICENSES.txt").c_str(),0,0,SW_SHOWNORMAL);return true;}
 if(command==168){ShellExecuteW(h,L"open",L"https://open-meteo.com/",0,0,SW_SHOWNORMAL);return true;}
 if(command==162){DialogBoxW(GetModuleHandle(0),MAKEINTRESOURCEW(950),h,saverConfig);return true;}
 if(command==163){openScreenSaverSettings(h);return true;}
 if(command==120){DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(500),h,timingDialog);return true;}
 if(command==122){DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(600),h,pagesDialog);return true;}
 if(command==160){cycle.page=8;cycle.elapsed=cycle.hold=0;repaint(h);return true;}
 if(command==126){if(MessageBoxA(h,"Restore display, clock, page order and timing defaults? Locations, audio volume and the Windows startup entry are kept.","Restore Defaults",MB_YESNO|MB_ICONQUESTION)==IDYES){aspectMode=0;safeMargin=0;textPercent=100;crtMode=false;clock24=false;celsius=false;windUnit=0;pageDurations.fill(0);startFullscreen=false;duration=12;pauseDuration=0;pageConfig=features::PageConfig();cycle.configure(pageConfig);cycle.paused=false;audio.setPaused(false);background=0;sceneLevel=legacyRendering?1:2;wakeCursor();}}
 else if(command==123)startFullscreen=!startFullscreen;
 else if(command==124){if(!setAutostart(!isAutostart()))MessageBoxA(h,"Could not update your Windows startup setting.","Retro Weather",MB_OK|MB_ICONEXCLAMATION);}
 else if(command==320||command==321)celsius=command==321;
 else if(command>=330&&command<=333)windUnit=command-330;
 else if(command>=300&&command<=305)aspectMode=command-300;
 else if(command==220){crtMode=!crtMode;crtPart=0;sceneCacheBackground=-1;clearCloudSprites();sunRadiance.clear();skyBuffer.clear();smoothScaler.clear();std::vector<uint32_t>().swap(previousScene);releaseTransition();foregroundCache.clear();foregroundReady=false;clearFonts();}
 else if(command>=210&&command<=213){int margins[]={0,2,5,8};safeMargin=margins[command-210];}
 else if(command>=230&&command<=232)textPercent=90+(command-230)*10;
 else if(command==170){showLocationDialog();return true;}
 else if(command==171){if(!zip.empty()&&MessageBoxA(h,"Remove the current location from the saved list? Its weather stays visible.","Locations",MB_YESNO|MB_ICONQUESTION)==IDYES){savedZips.erase(std::remove(savedZips.begin(),savedZips.end(),zip),savedZips.end());locationMeta.erase(zip);saveLocations();}}
 else if(command>=180&&command<190){int i=command-180;if(!busy&&i<(int)savedZips.size())startRequest(savedZips[i],0,locationMeta.value(savedZips[i],json()),true);return true;}
 else if(command==121)clock24=!clock24;
 else if(command==129){togglePresentation();}
 else if(command==130){audio.setEnabled(!audio.enabled,cycle.paused);}
 else if(command>=350&&command<=354){background=command-350;sceneCacheBackground=-1;if(background<4){foregroundScratch.clear();releaseTransition();smoothScaler.clear();clearCloudSprites();sunRadiance.clear();skyBuffer.clear();}}
 else if(command>=150&&command<=155){int levels[]={0,10,25,50,75,100};audio.volume=levels[command-150];audio.applyVolume();}
 else return false;
 syncWeatherAudio();syncSceneTimer(h);savePrefs();saveExtraPrefs();updateMenus();repaint(h);return true;
}
static void saverKey(HWND h,UINT message,WPARAM key,LPARAM flags){auto action=saver::key(message==WM_SYSKEYDOWN,(unsigned)key,(uint32_t)flags);if(action==saver::ToggleAudio){audio.setEnabled(!audio.enabled,false);syncWeatherAudio();if(!audio.enabled)KillTimer(h,5);repaint(h);}else if(action==saver::Exit)DestroyWindow(h);}
static LRESULT CALLBACK wnd(HWND h,UINT m,WPARAM w,LPARAM l){
 if(saverRun){
  if(m==WM_KEYDOWN||m==WM_SYSKEYDOWN){saverKey(h,m,w,l);return 0;}
  if(m==WM_LBUTTONDOWN||m==WM_RBUTTONDOWN||m==WM_MBUTTONDOWN||m==WM_XBUTTONDOWN||m==WM_MOUSEWHEEL){DestroyWindow(h);return 0;}
  if(m==WM_SETCURSOR){SetCursor(0);return TRUE;}
  if(m==WM_ACTIVATE&&LOWORD(w)==WA_INACTIVE&&GetTickCount()-saverStarted>500){DestroyWindow(h);return 0;}
  if(m==WM_MOUSEMOVE){POINT pos;if(GetTickCount()-saverStarted>500&&GetCursorPos(&pos)&&saver::moved(pos.x,pos.y,saverMouse.x,saverMouse.y))DestroyWindow(h);return 0;}
 }
 if(saverPreview&&(m==WM_CONTEXTMENU||m==WM_LBUTTONDOWN||m==WM_LBUTTONDBLCLK||m==WM_MOUSEMOVE||m==WM_MOUSEWHEEL))return 0;
 switch(m){case WM_CREATE:SetTimer(h,1,1000,0);return 0;
 case WM_COMMAND:if(LOWORD(w)==103)startLoad();else if(LOWORD(w)==101)toggleFull();else if(LOWORD(w)==105){if(full)toggleFull();}else if(LOWORD(w)==102){cycle.next();repaint(h);}else if(LOWORD(w)==106){togglePresentation();savePrefs();updateMenus();repaint(h);}else if(LOWORD(w)==104)DestroyWindow(h);else if(handleOptions(h,LOWORD(w))){}return 0;
 case WM_TIMER:{if(w==5){if(!audio.tick())KillTimer(h,5);return 0;}if(w==4){KillTimer(h,4);DWORD now=GetTickCount();double dt=(DWORD)(now-sceneTick)/1000.;sceneTick=now;if(!IsIconic(h)){scene.advance(dt);animationPending=true;frameDirty=true;InvalidateRect(h,0,FALSE);}return 0;}if(w==3){if(saverPreview){if(!IsWindow(previewParent)){DestroyWindow(h);return 0;}RECT r;GetClientRect(previewParent,&r);SetWindowPos(h,0,0,0,r.right,r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);}else if(saverRun){POINT pos;if(GetTickCount()-saverStarted>500&&GetCursorPos(&pos)&&saver::moved(pos.x,pos.y,saverMouse.x,saverMouse.y))DestroyWindow(h);}return 0;}if(w==2){if(saverRun)return 0;pollCursor();return 0;}++ticks;int previous=cycle.page,oldPart=crtPart;if(!benchmarkActive)cycle.tick(options::pageSeconds(cycle.page,duration,pageDurations),pauseDuration);if(!benchmarkActive)crtPart=crt_profile::part(cycle.elapsed,std::max(1,options::pageSeconds(cycle.page,duration,pageDurations)));std::string oldTrack=audio.title();syncWeatherAudio();audio.poll();
  static int64_t previousMinute=-1;int64_t minute=nowEpoch()/60;bool minuteChanged=minute!=previousMinute;previousMinute=minute;
  if(!IsIconic(h)&&(oldTrack!=audio.title()||previous!=cycle.page||(crtMode&&oldPart!=crtPart&&(cycle.page==2||cycle.page==7))||minuteChanged||(cycle.page==features::pausePage&&!cycle.paused)||ticks%60==0||ticks%45==0))repaint(h);
  if(!previewMode&&!settingsOperation&&!saverPreview&&(resumeRefresh||(lastUpdateFailed?ticks>=retryAt:ticks%900==0))&&validZip(zip)&&!busy&&!locationWindow){resumeRefresh=false;startLoad(false);}
  if(!previewMode&&!settingsOperation&&!saverPreview&&ticks%5==0&&!busy&&!locationWindow&&sessionUpdated){for(const auto&code:savedZips)if((!locationMeta.contains(code)||!locations::valid(locationMeta[code]))&&!metadataAttempted.count(code)){metadataAttempted.insert(code);startRequest(code,2);break;}}return 0;}
 case WM_POWERBROADCAST:if(w==PBT_APMSUSPEND){if(benchmarkActive&&benchmarkWindow)SendMessage(benchmarkWindow,WM_COMMAND,IDCANCEL,0);powerPaused=audio.isActive()&&!audio.isPaused();if(powerPaused)audio.setPaused(true);return TRUE;}if(w==PBT_APMRESUMESUSPEND||w==PBT_APMRESUMEAUTOMATIC||w==PBT_APMRESUMECRITICAL){if(powerPaused){audio.setPaused(cycle.paused);if(audio.isPaused()!=cycle.paused)audio.setEnabled(audio.enabled,cycle.paused);powerPaused=false;}if(!saverPreview)resumeRefresh=true;if(!saverRun&&!saverPreview)SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED|ES_SYSTEM_REQUIRED);repaint(h);return TRUE;}break;
 case WM_TIMECHANGE:repaint(h);return 0;
 case WM_SYSCOMMAND:if((w&0xFFF0)==SC_SCREENSAVE)return 0;if(!saverRun&&!saverPreview&&(w&0xFFF0)==SC_MONITORPOWER)return 0;break;
 case WM_INITMENUPOPUP:updateMenus();return 0;
 case WM_APP+1:{Result*r=(Result*)l;busy=false;lastHelperExit=r->helperExit;lastHttp=r->http;lastRequestError=r->error;
  if(r->kind==1){
   if(locationWindow&&locations::acceptPreview(r->serial,locationSerial,r->zip,controlText(zipEdit))){
    if(r->error.empty()){pendingLocation=r->location;setText(zipEdit,locations::label(pendingLocation));refreshLocationDialog("",false);}else{pendingLocation=json();refreshLocationDialog(r->error,false);}
   }else refreshLocationDialog();
  }else if(r->kind==2){if(r->error.empty()){locationMeta[r->zip]=r->location;saveLocations();}refreshLocationDialog();}
  else if(r->error.empty()){
   data=r->value;zip=r->zip;lastError.clear();locationMeta[zip]=r->location;saveLocations();sessionUpdated=true;lastUpdateFailed=false;retryFailures=0;status="Weather updated successfully.";
   try{writeFile(folder+"\\weather.new",data.dump());if(!MoveFileExA((folder+"\\weather.new").c_str(),(folder+"\\weather.json").c_str(),MOVEFILE_REPLACE_EXISTING))throw std::runtime_error("cache");}catch(...){lastError="Weather loaded; the offline cache could not be saved.";status=lastError;}
   WritePrivateProfileStringA("Settings","ZIP",zip.c_str(),ini().c_str());refreshLocationDialog();
  }else{lastUpdateFailed=true;retryAt=ticks+refinements::retryDelay(++retryFailures);lastError=r->error;status=r->error;refreshLocationDialog();if(r->notify)MessageBoxW(h,wide(r->error+(!data.empty()?"\n\nThe last successful forecast remains visible.":"")).c_str(),L"Weather Update",MB_OK|MB_ICONEXCLAMATION);}
  delete r;updateMenus();repaint(h);return 0;}
 case WM_CONTEXTMENU:if(saverRun||saverPreview)return 0;contextMenu(h,l);return 0;
 case WM_LBUTTONDOWN:case WM_LBUTTONDBLCLK:{SetFocus(h);wakeCursor();int x=0,y=0;
  presentation::Viewport v=viewport(h);presentation::Frame f=presentation::frameFor(v);
  if(f.map(v,(short)LOWORD(l),(short)HIWORD(l),x,y)){
   if(m==WM_LBUTTONDBLCLK)toggleFull();
  }return 0;}
 case WM_MOUSEMOVE:pollCursor();return 0;
 case WM_SETCURSOR:if(LOWORD(l)==HTCLIENT){POINT p;if(GetCursorPos(&p)){SetCursor(cursorIdle.hidden&&mayHideCursor(p)?0:LoadCursor(0,IDC_ARROW));return TRUE;}}break;
 case WM_ENTERMENULOOP:menuActive=true;wakeCursor();return 0;
 case WM_EXITMENULOOP:menuActive=false;wakeCursor();return 0;
 case WM_ACTIVATE:if(saverPreview)return 0;wakeCursor();if(full&&LOWORD(w)!=WA_INACTIVE)SetFocus(h);return 0;
 case WM_DISPLAYCHANGE:foregroundScratch.clear();releaseTransition();scaleChoice=rendering::ScaleChoice();renderProfile.reset();smoothScaler.clear();pictureBuffer.clear();sceneBuffer.clear();skyBuffer.clear();sceneCacheBackground=-1;wakeCursor();if(full)fitFullscreen();repaint(h,TRUE);return 0;
 case 0x02E0:{dpiOverride=LOWORD(w);RECT*r=(RECT*)l;if(!full)SetWindowPos(h,0,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);else fitFullscreen();repaint(h,TRUE);return 0;}
 case WM_MOVE:trackPlacement(h);return 0;
 case WM_SIZE:trackPlacement(h);if(w==SIZE_MINIMIZED){foregroundScratch.clear();releaseTransition();KillTimer(h,4);animationPending=false;frameMeter.reset();}else{syncSceneTimer(h);repaint(h,TRUE);}return 0;case WM_ERASEBKGND:return 1;case WM_PAINT:paint(h);return 0;
 case WM_DESTROY:clearFonts();foregroundScratch.clear();releaseTransition();smoothScaler.clear();foregroundCache.clear();clearCloudSprites();sunRadiance.clear();previousScene.clear();pictureBuffer.clear();sceneBuffer.clear();skyBuffer.clear();KillTimer(h,4);KillTimer(h,5);savePlacement(h);KillTimer(h,3);KillTimer(h,2);wakeCursor();savePrefs();audio.stop();saveExtraPrefs();if(!saverPreview&&!restoringSettings)saveLocations();if(!saverRun&&!saverPreview)releaseScreensaver();SetEvent(stopEvent);if(worker){WaitForSingleObject(worker,4000);CloseHandle(worker);} {MSG msg;while(PeekMessage(&msg,h,WM_APP+1,WM_APP+1,PM_REMOVE))delete (Result*)msg.lParam;}PostQuitMessage(0);return 0;
 }return DefWindowProc(h,m,w,l);
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR arguments,int){
 legacyRendering=IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)==FALSE;SYSTEM_INFO qualityCPU;GetSystemInfo(&qualityCPU);MEMORYSTATUSEX qualityRAM={};qualityRAM.dwLength=sizeof(qualityRAM);GlobalMemoryStatusEx(&qualityRAM);graphicsQuality.configure(legacyRendering,qualityCPU.dwNumberOfProcessors,qualityRAM.ullTotalPhys/(1024*1024));enableDpiAwareness();if(std::string(arguments)=="--screen-saver-settings"){openScreenSaverSettings(0);return 0;}
 char module[MAX_PATH];GetModuleFileNameA(0,module,MAX_PATH);std::string name=module;bool scr=name.size()>4&&_stricmp(name.c_str()+name.size()-4,".scr")==0;saver::Options options=saver::parse(arguments,scr);if(options.mode==saver::Invalid)return 0;saverRun=options.mode==saver::Run;saverPreview=options.mode==saver::Preview;previewParent=(HWND)options.parent;
 if(saverPreview&&!IsWindow(previewParent))return 0;HANDLE saverUIMutex=0;if(saverPreview||options.mode==saver::Configure)saverUIMutex=CreateMutexA(0,FALSE,"RetroWeather-ScreensaverUI");if(saverRun){HANDLE player=OpenMutexA(SYNCHRONIZE,FALSE,"RetroWeatherXP-v01");if(player){CloseHandle(player);return 0;}}
 HANDLE instanceMutex=0;if(!saverPreview&&options.mode!=saver::Configure){instanceMutex=CreateMutexA(0,FALSE,saverRun?"RetroWeather-Screensaver":"RetroWeatherXP-v01");if(GetLastError()==ERROR_ALREADY_EXISTS){if(!saverRun){HWND existing=FindWindowA("RetroWeather",0);if(existing){ShowWindow(existing,SW_RESTORE);SetForegroundWindow(existing);}}CloseHandle(instanceMutex);return 0;}}
 char path[MAX_PATH];GetModuleFileNameA(0,path,MAX_PATH);base=path;base=base.substr(0,base.find_last_of("\\"));
 if(scr){char installed[MAX_PATH]={};DWORD size=sizeof(installed),type=0;HKEY key;bool found=false;for(HKEY root:{HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE}){if(RegOpenKeyExA(root,"Software\\RetroWeather",0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){size=sizeof(installed);if(RegQueryValueExA(key,"InstallDir",0,&type,(BYTE*)installed,&size)==ERROR_SUCCESS&&type==REG_SZ){base=installed;found=true;}RegCloseKey(key);if(found)break;}}if(!found&&GetFileAttributesA((base+"\\data").c_str())==INVALID_FILE_ATTRIBUTES){if(!saverPreview)MessageBoxA(0,"Install Retro Weather before using the screensaver.","Retro Weather",MB_OK);return 0;}}
 if(FAILED(SHGetFolderPathA(0,CSIDL_APPDATA|CSIDL_FLAG_CREATE,0,0,path)))return 1;folder=std::string(path)+"\\RetroWeather";CreateDirectoryA(folder.c_str(),0);
 
 if(options.mode==saver::Configure){HWND owner=(HWND)options.parent;if(!IsWindow(owner))owner=GetForegroundWindow();DialogBoxW(instance,MAKEINTRESOURCEW(950),owner,saverConfig);return 0;}
 char z[16];GetPrivateProfileStringA("Settings","ZIP","",z,sizeof(z),(folder+"\\settings.ini").c_str());zip=z;
 try{data=json::parse(readFile(folder+"\\weather.json"));validateWeather(data);if(!validZip(data.value("zip",""))||data.value("zip","")!=zip)throw std::runtime_error("cache");status="Saved weather. Refresh to get the latest forecast.";}catch(...){data=json();}
 duration=setting("PageSeconds",12,1,3600);pauseDuration=setting("PauseSeconds",0,0,3600);clock24=setting("Clock24",0,0,1)!=0;
 aspectMode=setting("AspectMode",0,0,5);startFullscreen=setting("StartFullscreen",0,0,1)!=0;celsius=setting("Celsius",0,0,1)!=0;windUnit=setting("WindUnit",0,0,3);crtMode=setting("CrtMode",0,0,1)!=0;safeMargin=setting("SafeMargin",0,0,10);textPercent=setting("TextPercent",100,90,110);if(textPercent!=90&&textPercent!=100&&textPercent!=110)textPercent=100;
 pageConfig.parse(stringSetting("PageOrder","0,6,7,1,2,3,5,4,8"),setting("PageMask",511,1,511));cycle.configure(pageConfig);bool missingSavedList=stringSetting("SavedZips","__missing__")=="__missing__";savedZips=features::parseZips(stringSetting("SavedZips"));if(missingSavedList&&savedZips.empty()&&validZip(zip))savedZips.push_back(zip);
 try{locationMeta=locations::sanitize(json::parse(readFile(folder+"\\locations.json")));}catch(...){locationMeta=json::object();}
 if(!zip.empty()&&!locationMeta.contains(zip)){json cached=locations::fromWeather(data);if(locations::valid(cached))locationMeta[zip]=cached;}
 bool legacyPages=stringSetting("PageOrder").size()==13;for(int i=0;i<features::pageCount;++i)pageDurations[i]=setting(("PageOverride"+std::to_string(i)).c_str(),0,0,3600);if(legacyPages){if(!pageDurations[3])pageDurations[3]=pageDurations[6];pageDurations[6]=0;}
 audio.volume=setting("AudioVolume",25,0,100);audio.enabled=setting("AudioEnabled",1,0,1)!=0;background=setting("Background",0,0,4);sceneLevel=legacyRendering?1:2;scene.rng=GetTickCount()^GetCurrentProcessId();
 try{zoneTable=json::parse(readFile(base+"\\data\\timezones.json"));}catch(...){zoneTable=json();}
 try{illuminationTable=json::parse(readFile(base+"\\data\\illumination.json"));}catch(...){illuminationTable=json();}
 try{moonTable=json::parse(readFile(base+"\\data\\moon.json"));}catch(...){moonTable=json();}

 stopEvent=CreateEvent(0,TRUE,FALSE,0);WNDCLASSA wc={};wc.style=CS_DBLCLKS;wc.lpfnWndProc=wnd;wc.hInstance=instance;wc.lpszClassName=saverRun?"RetroWeatherSaver":saverPreview?"RetroWeatherPreview":"RetroWeather";wc.hCursor=LoadCursor(0,IDC_ARROW);wc.hIcon=LoadIcon(instance,MAKEINTRESOURCE(1));RegisterClassA(&wc);
 menus=buildMenus();
 if(saverRun){GetCursorPos(&saverMouse);saverStarted=GetTickCount();}
 RECT r={0,0,640,480};if(saverPreview)GetClientRect(previewParent,&r);if(!saverPreview)AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,TRUE);mainWindow=CreateWindowA(wc.lpszClassName,"Retro Weather v1.2.1",saverPreview?WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN:WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,saverPreview?0:CW_USEDEFAULT,saverPreview?0:CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,saverPreview?previewParent:0,saverPreview?0:menus,instance,0);if(!mainWindow)return 1;
 if(!saverRun&&!saverPreview)inhibitScreensaver();cycle.paused=!saverRun&&!saverPreview&&setting("PagesPaused",0,0,1)!=0;if(saverRun||saverPreview)audio.enabled=false;syncWeatherAudio();syncSceneTimer(mainWindow);updateMenus();
 if(!saverRun&&!saverPreview)restorePlacement(mainWindow);bool maximizeOnStart=windowMaximized;ShowWindow(mainWindow,saverPreview?SW_SHOWNOACTIVATE:SW_SHOW);if(maximizeOnStart)ShowWindow(mainWindow,SW_MAXIMIZE);if(saverRun){MonitorSearch search;search.device=stringSetting("FullscreenMonitor",stringSetting("WindowMonitor").c_str());EnumDisplayMonitors(0,0,findMonitor,(LPARAM)&search);MONITORINFO mi={};mi.cbSize=sizeof(mi);if(search.found&&GetMonitorInfo(search.found,&mi))SetWindowPos(mainWindow,0,mi.rcMonitor.left,mi.rcMonitor.top,640,480,SWP_NOZORDER|SWP_NOACTIVATE);}if(saverRun||(!saverPreview&&startFullscreen))toggleFull();if(saverRun||saverPreview){GetCursorPos(&saverMouse);saverStarted=GetTickCount();SetTimer(mainWindow,3,200,0);if(saverRun){SetForegroundWindow(mainWindow);SetFocus(mainWindow);SetCursor(0);}}UpdateWindow(mainWindow);if(!saverPreview&&validZip(zip))startLoad(false);
 ACCEL keys[]={{FVIRTKEY,VK_F11,101},{FVIRTKEY,VK_ESCAPE,105},{FVIRTKEY,VK_F5,103},{FVIRTKEY,'P',106},{FVIRTKEY|FCONTROL,'L',170},{FVIRTKEY,VK_F1,160},{FVIRTKEY,'M',130}};
 HACCEL shortcuts=CreateAcceleratorTable(keys,7);
 MSG msg;while(GetMessage(&msg,0,0,0)>0){
 if(saverRun&&(msg.message==WM_KEYDOWN||msg.message==WM_SYSKEYDOWN)){saverKey(mainWindow,msg.message,msg.wParam,msg.lParam);continue;}
 if(!saverRun&&!saverPreview&&GetAncestor(msg.hwnd,GA_ROOT)==mainWindow&&shortcuts&&TranslateAccelerator(mainWindow,shortcuts,&msg))continue;
 // Retain key handling if creating the accelerator table fails.
 if(!saverRun&&!saverPreview&&!shortcuts&&(msg.message==WM_KEYDOWN||msg.message==WM_SYSKEYDOWN)){
  if(msg.wParam==VK_F11){toggleFull();continue;}
  if(msg.wParam==VK_ESCAPE&&full){toggleFull();continue;}
  if(msg.wParam==VK_F5){startLoad();continue;}
  if(msg.wParam=='M'&&GetAncestor(msg.hwnd,GA_ROOT)==mainWindow){handleOptions(mainWindow,130);continue;}
 }if(!saverRun&&!saverPreview&&msg.message==WM_KEYDOWN&&msg.wParam==VK_SPACE&&msg.hwnd!=zipEdit){cycle.next();repaint(mainWindow);continue;}TranslateMessage(&msg);DispatchMessage(&msg);}if(shortcuts)DestroyAcceleratorTable(shortcuts);CloseHandle(stopEvent);if(instanceMutex)CloseHandle(instanceMutex);return 0;
}
