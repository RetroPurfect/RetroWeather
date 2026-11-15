RETRO WEATHER v1.2.1 — PUBLIC RELEASE PREPARATION
Release date: 2026-11-15 | File version: 1.2.1.0

Derived from the accepted v1.2 baseline, which remains preserved unchanged.
Adds provider attribution, license links/notices and public distribution files.
See RELEASE_NOTES.txt and VALIDATION.txt for changes and verification limits.

INSTALL / UPGRADE
Run RetroWeather_Setup_v1.2.1.exe. Existing Retro Weather instances are asked to
close gracefully; Retry/Cancel appears if closure does not finish. Locations,
preferences and installed screensaver selection are retained during upgrades.
No forced process termination. Signing remains intentionally skipped.

MENUS
Weather: location/saved locations, refresh, temperature and wind units.
Display: fullscreen, CRT Mode, background, start fullscreen, Advanced Display.
Pages: next, pause/resume pages and audio, choose/order, timing, 24-hour clock.
Audio: enable/disable weather audio, volume.
Tools: Diagnostics / Benchmark and latest report export, settings backup,
screensaver controls/Windows settings, start with Windows, restore defaults, exit.
Help: controls/shortcuts, About, weather data/license terms and provider link.

WEATHER PAGES
Current Conditions; Today & Tonight; Hourly Forecast (12 hours); Seven-Day
Forecast; Weather Details; Sun & Moon; Time & Date; US Holidays; About.
Added details include gusts, dew point, visibility, sea-level pressure, cloud
cover, daily UV maximum, precipitation and snow totals when data is available.
Missing fields show --. Forecasts are model-based, not a station observation
or an alert service. Live-data footers credit Open-Meteo; About and the Help
menu provide full data/license information.

CRT MODE
Enable Display > CRT Mode. For the accepted Dell Inspiron 6400/Core 2 Duo/ATI
X1300 setup, retain 720x480 at 60 Hz and use F11 fullscreen on the 4:3 CRT.
The whole output raster is filled with scenery; only text/icons receive a 2%
safe inset. Logical foreground remains 640x480 with large bold text, lightweight
scenery and crisp presentation. Hourly and Details each use two readable panels
within their page duration. Normal display preferences return when CRT Mode is
disabled. The app does not alter Windows resolution, timing or monitor controls.

SCREENSAVER AUDIO
Every session starts with audio disabled. M enables/disables weather audio
without exiting; holding M toggles once. Disabling stops playback, closes audio
devices and releases PCM buffers. Other keys or normal mouse movement exit.
Preview and basic backgrounds stay silent. Player audio preferences are separate.

DIAGNOSTICS / RECORDING
Tools > Diagnostics / Benchmark runs the 305-second timed test and exports TXT.
Optional recording (off by default) adds a separate 144-second replay, excluded
from timed measurements. 360p/480p/720p/1080p/2160p maximum presets at 10 FPS;
aspect preserved, no upscaling, no file-size cap. Higher resolutions depend on
the installed Windows encoder. XP uses text diagnostics only. MP4 requires
supported Windows 7+ media components. Generated soundtrack is not a recording
of physical speakers or microphone. Export Latest Report is available after a run.
Files are saved in Documents\Retro Weather\Diagnostics.

KEYS
F11 fullscreen | Esc leave fullscreen | Space next page | P pause/resume
M enable/disable weather audio | F5 refresh | Ctrl+L location | F1 About
Right-click for the organized menus in fullscreen.

BUILD / SOURCE
Original player source, assets and synthesis scripts are MIT licensed.
Third-party code/data retain their respective licenses. Read LICENSE.txt,
THIRD_PARTY_NOTICES.txt, DATA_LICENSES.txt and NETWORK_SOURCE.txt.
Build on a modern Windows machine with an XP-capable 32-bit MSVCRT MinGW-w64
compiler, windres and NSIS; run build_windows.cmd. Toolchains are not bundled.
Python 3 plus g++ can run tests/run_tests.py. No paid SDK or app account is needed.
Network source is a separate companion download; it is not needed just to build
the player against the unchanged bundled helper.
The free hosted weather API is restricted to noncommercial use and its limits.
The software's MIT license itself permits commercial reuse; API rights differ.

PUBLIC WEBSITE
The docs/ folder is a static GitHub Pages site. See PUBLISHING.md for a free
public repository, release downloads and Pages setup. No paid domain required.
No account connection or public deployment has been performed by this pack.
