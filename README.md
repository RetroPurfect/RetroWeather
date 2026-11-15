# Retro Weather

A free, open-source native Windows weather player and screensaver. Animated
scenery, optional synthesized ambience, current conditions, 12-hour and seven-day
forecasts, sun/moon information, time/date and US holidays.

**v1.2.1 is the stable release, confirmed by the project maintainer.**
This release includes attribution and distribution notices.

- XP SP3 through Windows 11 are compatibility targets for the 32-bit build.
- CRT Mode provides larger bold text and lightweight full-raster scenery.
- Screensaver audio starts disabled; M enables/disables it without exiting.
- Built-in diagnostics export a report; optional MP4 replay requires supported
  Windows 7+ media components. XP uses text diagnostics only.
- Unsigned installer. No ads, subscription or Retro Weather account.

## Download

- [Official website](https://retropurfect.github.io/RetroWeather/)
- [Download the v1.2.1 Windows installer](https://github.com/RetroPurfect/RetroWeather/releases/download/v1.2.1/RetroWeather_Setup_v1.2.1.exe)
- [All v1.2.1 downloads, source archives and SHA-256 checksums](https://github.com/RetroPurfect/RetroWeather/releases/tag/v1.2.1)
- [Report a bug or request a feature](https://github.com/RetroPurfect/RetroWeather/issues/new/choose)

The website is hosted free on GitHub Pages from `main/docs`.

## Build

On a modern Windows development machine, use an XP-capable **i686 MSVCRT**
MinGW-w64 compiler plus windres and NSIS, then run `build_windows.cmd`.
64-bit/UCRT toolchains do not produce the intended XP-compatible binary.
No paid SDK is required. Build tools are obtained separately.

Host regression checks: Python 3 + g++, `python tests/run_tests.py`.
See [README.txt](README.txt) for controls and compatibility limits.

## Licenses and service terms

Original code, generated art and synthesized ambience: [MIT](LICENSE.txt).
Third parties retain their respective licenses; see
[THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt) and
[NETWORK_SOURCE.txt](NETWORK_SOURCE.txt). The matching helper source archive is
published beside each binary release, including exact tarballs and build patches.

Weather data by [Open-Meteo](https://open-meteo.com/), CC BY 4.0. Its free hosted
API has **noncommercial-use restrictions and request limits**, separate from the
MIT software license. Locations: Zippopotam.us / GeoNames. Read
[DATA_LICENSES.txt](DATA_LICENSES.txt) for attribution, privacy and database terms.

## Contributing / testing

Open an issue with your app version, OS, CPU/GPU, display resolution and CRT Mode
setting. A benchmark report helps; describe actual visual/audio behavior too.
No automated benchmark upload is performed. Review files for private information
before sharing, especially videos of real location pages.

Free website/repository setup: [PUBLISHING.md](PUBLISHING.md).
Independent project; not affiliated with The Weather Channel or Microsoft.
