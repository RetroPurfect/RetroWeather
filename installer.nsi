Unicode false
!include "MUI2.nsh"
Name "Retro Weather v1.2.1"
OutFile "RetroWeather_Setup_v1.2.1.exe"
InstallDir "$PROGRAMFILES\Retro Weather"
InstallDirRegKey HKCU "Software\RetroWeather" "InstallDir"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
!define MUI_ICON "assets\RetroWeather.ico"
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!define MUI_DIRECTORYPAGE_TEXT_TOP "Choose a folder for Retro Weather. Settings and saved locations are kept during an upgrade."
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
VIProductVersion "1.2.1.0"
VIAddVersionKey /LANG=1033 "ProductName" "Retro Weather"
VIAddVersionKey /LANG=1033 "ProductVersion" "1.2.1"
VIAddVersionKey /LANG=1033 "FileDescription" "Retro Weather Setup"
VIAddVersionKey /LANG=1033 "FileVersion" "1.2.1.0"
VIAddVersionKey /LANG=1033 "LegalCopyright" "(c) 2026 Retro Weather contributors"

Var SaverUpdated
Function CloseRunningRetroWeather
 InitPluginsDir
 File /oname=$PLUGINSDIR\RetroWeatherClose.exe "dist\RetroWeatherClose.exe"
 retry_close:
 ClearErrors
 ExecWait '"$PLUGINSDIR\RetroWeatherClose.exe"' $0
 IfErrors close_failed
 StrCmp $0 0 close_ready
 close_failed:
 MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Retro Weather could not finish closing. Close any remaining Retro Weather windows or screensaver, then choose Retry. Installation has not replaced any program files." IDRETRY retry_close
 Abort
 close_ready:
FunctionEnd
Function .onInit
 Call CloseRunningRetroWeather
 StrCpy $SaverUpdated 0
FunctionEnd
Section "Player (required)"
 SectionIn RO
 Call CloseRunningRetroWeather
 SetOutPath "$INSTDIR"
 File "dist\RetroWeather.exe"
 File "README.txt"
 File "LICENSE.txt"
 File "DATA_LICENSES.txt"
 File "THIRD_PARTY_NOTICES.txt"
 File "NETWORK_SOURCE.txt"
 File "BASELINE.md"
 File "CREDITS.txt"
 File "RELEASE_NOTES.txt"
 File "TESTED_SYSTEMS.txt"
 File "SCREENSAVER_UPGRADE.txt"
 File "COMPATIBILITY_TESTS.md"
 SetOutPath "$INSTDIR\legal"
 File /r "dist\legal\*.*"
 SetOutPath "$INSTDIR\data"
 File /r "dist\data\*.*"
 SetOutPath "$INSTDIR\Sounds"
 File "dist\Sounds\*.wav"
 File "dist\Sounds\manifest.json"
 SetOutPath "$INSTDIR\network"
 File /r "dist\network\*.*"
 SetOutPath "$INSTDIR"
 WriteUninstaller "$INSTDIR\Uninstall.exe"
 WriteRegStr HKCU "Software\RetroWeather" "InstallDir" "$INSTDIR"
 WriteRegStr HKLM "Software\RetroWeather" "InstallDir" "$INSTDIR"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RetroWeather" "DisplayName" "Retro Weather v1.2.1"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RetroWeather" "DisplayVersion" "1.2.1"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RetroWeather" "DisplayIcon" "$INSTDIR\RetroWeather.exe,0"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RetroWeather" "UninstallString" '"$INSTDIR\Uninstall.exe"'
 CreateDirectory "$SMPROGRAMS\Retro Weather"
 CreateShortcut "$SMPROGRAMS\Retro Weather\Retro Weather.lnk" "$INSTDIR\RetroWeather.exe"
 CreateShortcut "$SMPROGRAMS\Retro Weather\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
 CreateShortcut "$DESKTOP\Retro Weather.lnk" "$INSTDIR\RetroWeather.exe"
SectionEnd
Section "Windows Screensaver"
 SetOutPath "$WINDIR"
 File /oname=RetroWeather.scr "dist\RetroWeather.scr"
 StrCpy $SaverUpdated 1
 WriteRegStr HKLM "Software\RetroWeather" "ScreensaverOwner" "$INSTDIR"
 CreateShortcut "$SMPROGRAMS\Retro Weather\Screen Saver Settings.lnk" "$INSTDIR\RetroWeather.exe" "--screen-saver-settings"
SectionEnd
; Never set SCRNSAVE.EXE, ScreenSaveTimeOut, ScreenSaveActive or
; ScreenSaverIsSecure during install/upgrade. Windows keeps the user's choices.
Section "-Refresh existing owned screensaver"
 StrCmp $SaverUpdated 1 refresh_saver_done
 ReadRegStr $0 HKLM "Software\RetroWeather" "ScreensaverOwner"
 StrCmp $0 $INSTDIR 0 refresh_saver_done
 IfFileExists "$WINDIR\RetroWeather.scr" 0 refresh_saver_done
 ClearErrors
 CopyFiles /SILENT "$INSTDIR\RetroWeather.exe" "$WINDIR\RetroWeather.scr"
 IfErrors 0 refresh_saver_done
 MessageBox MB_OK "The screensaver could not be updated. Close its Settings preview and run the installer again. Your Windows screensaver selection and timeout have not been changed."
 refresh_saver_done:
SectionEnd
Section "Uninstall"
 System::Call 'kernel32::OpenMutexA(i 0x100000, i 0, m "RetroWeather-ScreensaverUI") p.r0'
 StrCmp $0 0 un_ui_clear
 System::Call 'kernel32::CloseHandle(p r0)'
 Goto un_running
 un_ui_clear:
 FindWindow $0 "RetroWeather" ""
 StrCmp $0 0 un_check_saver
 Goto un_running
 un_check_saver:
 FindWindow $0 "RetroWeatherSaver" ""
 StrCmp $0 0 un_check_preview
 Goto un_running
 un_check_preview:
 FindWindow $0 "RetroWeatherPreview" ""
 StrCmp $0 0 un_ready
 un_running:
 MessageBox MB_OK "Close Retro Weather and any screensaver preview/settings before uninstalling."
 Abort
 un_ready:
 ReadRegStr $0 HKLM "Software\RetroWeather" "ScreensaverOwner"
 StrCmp $0 $INSTDIR 0 un_keep_scr
 Delete "$WINDIR\RetroWeather.scr"
 ReadRegStr $0 HKCU "Control Panel\Desktop" "SCRNSAVE.EXE"
 StrCmp $0 "$WINDIR\RetroWeather.scr" 0 un_keep_scr
 DeleteRegValue HKCU "Control Panel\Desktop" "SCRNSAVE.EXE"
 un_keep_scr:
 Delete "$SMPROGRAMS\Retro Weather\Screen Saver Settings.lnk"
 DeleteRegKey HKLM "Software\RetroWeather"
 Delete "$DESKTOP\Retro Weather.lnk"
 Delete "$SMPROGRAMS\Retro Weather\Retro Weather.lnk"
 Delete "$SMPROGRAMS\Retro Weather\Uninstall.lnk"
 RMDir "$SMPROGRAMS\Retro Weather"
 Delete "$INSTDIR\RetroWeather.exe"
 Delete "$INSTDIR\README.txt"
 Delete "$INSTDIR\BASELINE.md"
 Delete "$INSTDIR\LICENSE.txt"
 Delete "$INSTDIR\DATA_LICENSES.txt"
 Delete "$INSTDIR\THIRD_PARTY_NOTICES.txt"
 Delete "$INSTDIR\NETWORK_SOURCE.txt"
 Delete "$INSTDIR\CREDITS.txt"
 Delete "$INSTDIR\RELEASE_NOTES.txt"
 Delete "$INSTDIR\TESTED_SYSTEMS.txt"
 Delete "$INSTDIR\SCREENSAVER_UPGRADE.txt"
 Delete "$INSTDIR\COMPATIBILITY_TESTS.md"
 RMDir /r "$INSTDIR\network"
 RMDir /r "$INSTDIR\legal"
 RMDir /r "$INSTDIR\data"
 RMDir /r "$INSTDIR\Sounds"
 ; AppData settings are preserved.
 Delete "$INSTDIR\Uninstall.exe"
 RMDir "$INSTDIR"
 DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Retro Weather"
 DeleteRegKey HKCU "Software\RetroWeather"
 DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RetroWeather"
SectionEnd
