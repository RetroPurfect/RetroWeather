@echo off
setlocal
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 goto compiler_missing
pushd src
windres player.rc -O coff -o player.res
if errorlevel 1 (popd & goto failed)
popd
g++ -std=c++11 -Os -march=pentium3 -mtune=pentium3 -mno-sse -mno-sse2 -mfpmath=387 -s -static -static-libgcc -static-libstdc++ -mwindows -Wl,--major-os-version,5,--minor-os-version,1,--major-subsystem-version,5,--minor-subsystem-version,1 src\main.cpp src\video_recorder.cpp src\player.res -o dist\RetroWeather.exe -lgdi32 -lshell32 -luser32 -lwinmm -ladvapi32 -lcomdlg32 -lmsimg32 -lole32 -lmfuuid
if errorlevel 1 goto failed
g++ -std=c++11 -Os -march=pentium3 -mtune=pentium3 -mno-sse -mno-sse2 -mfpmath=387 -s -static -static-libgcc -static-libstdc++ -mwindows -Wl,--major-os-version,5,--minor-os-version,1,--major-subsystem-version,5,--minor-subsystem-version,1 src\upgrade_close.cpp -o dist\RetroWeatherClose.exe -luser32
if errorlevel 1 goto failed
copy /y dist\RetroWeather.exe dist\RetroWeather.scr >nul
if errorlevel 1 goto failed
if exist "%ProgramFiles(x86)%\NSIS\makensis.exe" (
 "%ProgramFiles(x86)%\NSIS\makensis.exe" installer.nsi
) else (
 makensis installer.nsi
)
if errorlevel 1 goto failed
for %%F in (LICENSE.txt DATA_LICENSES.txt THIRD_PARTY_NOTICES.txt NETWORK_SOURCE.txt) do copy /y "%%F" "dist\%%F" >nul
if errorlevel 1 goto failed
echo Build complete: RetroWeather_Setup_v1.2.1.exe
pause
exit /b 0
:compiler_missing
echo Put an XP-capable 32-bit MinGW-w64 MSVCRT compiler on PATH.
echo Use the i686 compiler, not a 64-bit or UCRT compiler.
:failed
echo Build failed. Read the error above.
pause
exit /b 1
