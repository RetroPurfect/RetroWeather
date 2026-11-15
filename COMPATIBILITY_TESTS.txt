v1.2: metadata-only promotion of accepted Test21. See BASELINE.md for scope.
Entries below are the historical validation record, not unresolved release tasks.

Test21 native 720x480/60 Hz CRT full-raster and physical aspect checks pending.
Test20 benchmark completed on Core 2 Duo/XP, both CRT panels covered; user found
app side borders, addressed by this build. Earlier records are historical.

Test20 native Windows menu/profile and Dell X1300/CRT readability checks pending.
Test19 remains user-confirmed. Earlier entries below are historical.

Test19 native Windows installation, calendar badges and saver M-key checks pending.
Earlier records below are historical checkpoints.

Test18: native Windows page appearance and high-resolution encoder tests pending.
Test17 was reported working by the user. Earlier entries below are historical.

Test17 update: redesigned-page runtime checks on Windows are pending.
Earlier results below are historical checkpoints, not Test17 certification.

Retro Weather v1.1 Test16 — Optional small MP4 replay (2026-11-15)

Close Retro Weather and install over Test15. Choose Help > Run Full Diagnostics / Benchmark. A new options dialog offers recording OFF by default. The existing 305-second timed benchmark runs first, unchanged. If recording is enabled, a separate 108-second replay then shows all nine weather scenes and all seven pages in each scene. Recording/encoder initialization cannot affect the earlier timing results. Total with recording is about seven minutes plus encoder setup/finalization.

Options: 720p or lighter 480p, maximum 10 FPS, aspect ratio preserved with even video dimensions. No upscaling of normal-sized sources. File-size limits: 10, 25 (default), or 50 MB (binary MB units). H.264 target bitrate is reduced for the smaller limit. Default 720p + generated audio estimates roughly 13 MB for the full replay; actual codec output varies. 480p is usually smaller. A bounded two-frame queue drops captures instead of accumulating raw frames. No raw recording files are written to disk. Encoders run on a below-normal-priority worker; Windows may use hardware or software encoding. Encoder internal buffers are managed by Windows.

MP4 uses Windows Media Foundation H.264/AAC encoding; Windows 7+ media components/encoders are required. These DLLs are dynamically loaded only for recording, with no new Media Foundation DLL imports. On XP, the recording checkbox is disabled and the normal text benchmark remains available. Missing/unsupported encoders on newer Windows are reported without discarding the text benchmark. Windows N editions may lack media components.

Capture includes only the app's offscreen simulated weather picture: no desktop, progress dialog, microphone or system-audio capture. Optional AAC audio is generated from the bundled sounds, resampled to 44.1 kHz mono, at the temporary benchmark volume. It is an intended soundtrack, not evidence that waveOut/the physical speakers worked; keep listening during the timed run. Replay audio boundaries follow scene transitions; this is not a recording of the actual driver crossfade.

The MP4 is streamed into Documents\Retro Weather\Diagnostics. The final combined text report uses the same timestamped base name when available. The report includes recording resolution/bitrate, written/dropped/missed frame counts, captured scene/page masks, file size, duration, errors, and separate recording-only process CPU/peak working set. Recording statistics are separate from benchmark performance. Post-run memory may retain Windows encoder/allocator caches.

Size monitoring stops recording early near the selected limit with room for MP4 metadata. Finalized output above the limit is removed; failure to remove an owned incomplete/oversized file is explicitly reported. Existing files are never intentionally replaced: a new unique name is reserved first. Cancellation during timing exports partial text without recording. Cancellation during replay stops/finalizes a partial MP4 and retains the completed timed results. Resize/minimize during replay stops capture; original settings/live weather/audio are restored before export. Encoder finalization can take a few seconds. Output failures keep the combined report available for Quick Export retry in the current process.

TEST: run once with recording off to compare normal behavior; then enable recording (720p, 25 MB, audio) and run again. Upload the paired .txt and .mp4. Check video is upright, colors/text legible, scenes/page changes present, audio intended soundtrack present, file size within the selected cap, and live settings restored afterward. First native Windows encoding run remains pending. Use 480p if capture overhead/drop count is high. No overnight testing needed.

Reference implementation guidance: Microsoft Learn, Using the Sink Writer / Sink Writer Video Encoding Tutorial, H.264 Video Encoder and AAC Encoder. No external encoder executable or codec download is bundled.
