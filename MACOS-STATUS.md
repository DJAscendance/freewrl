# FreeWRL on macOS (Apple Silicon): status

Fork: https://github.com/DJAscendance/freewrl, branch `macos-arm64` ([PR #1](https://github.com/DJAscendance/freewrl/pull/1)).
Base: upstream SourceForge `master` @ `e99ab4a00` (2020-02-21, the last upstream commit).
Tested on: MacBook Pro M1, macOS 27.0, Xcode 27.0, Homebrew. Last updated 2026-09-25.

Legend: ✅ verified (with evidence) · 🟡 fixed or present but not verified · ❌ known broken / disabled · ❔ inconclusive

## Build

| | item | notes |
| --- | --- | --- |
| ✅ | Xcode project builds (Release, arm64) | `xcodebuild -project OSX_gui/FreeWRL-Desktop/FreeWRL.xcodeproj -scheme FreeWRL -configuration Release ARCHS=arm64 CODE_SIGN_IDENTITY=- build` |
| ✅ | Homebrew dependencies | `brew install freetype imlib2 openal-soft freealut ode ffmpeg libxml2` |
| ✅ | Latest engine sources compile | `Component_PTM.c` (added upstream in 2020, never in the Mac project) now included |
| 🟡 | Debug configuration | not built since the header-path changes |
| ❌ | Standalone / distributable app | links Homebrew dylibs in `/opt/homebrew`; needs bundling (`OSX_gui/fixdylibs.sh` approach) and signing |
| ❌ | Autotools (`./configure`) build on macOS | not attempted; Xcode is the supported Mac path |

## App behaviour

| | item | notes |
| --- | --- | --- |
| ✅ | Launches without crashing | startup race fixed (timer drew before the library instance existed); dozens of launches since, no crash |
| ✅ | Opens a world from the command line / `open -a … --args <file-or-url>` | |
| ✅ | Retina rendering | viewport sized in backing pixels; scene centred and full-window |
| ✅ | HUD (status bar, menu buttons, text) sized for Retina | density factor from `backingScaleFactor` |
| 🟡 | Mouse picking / navigation on Retina | coordinates scaled by the backing factor; not exercised by automated tests |
| 🟡 | Quit with `q` | used to segfault; app now terminates when the library reports shutdown. Not re-tested since the fix |
| 🟡 | Window moved to a non-Retina display | should rescale (density re-applied on change); untested, no external display |
| ✅ | Library console messages visible | mirrored to stderr (parse errors, load failures) |

## Loading & parsing

| | item | notes |
| --- | --- | --- |
| ✅ | VRML97 files from disk | `freewrl/tests` 1, 2, 3, 6, 7, 16, 49, 50 and JS tests load |
| ✅ | Worlds over HTTP (libcurl) | Cybertown 002 over HTTP, 3/3 runs, 0 download failures |
| ✅ | Parallel texture downloads | thread-safe `curl_global_init`; earlier failures were a test-server artifact (listen backlog 5) |
| ✅ | `MFNode … NULL` (blaxxun-era content) | previously aborted the whole parse; Cybertown 002 now parses |
| ✅ | Inline | test 16 renders |
| 🟡 | Server-rooted EXTERNPROTO (`/externprotos/…`) | Cybertown 002 renders, but FreeWRL never requested `shared_xite.wrl`; may be because `home.wrl` also defines `BlaxxunZone` locally. Needs a look |
| 🟡 | X3D XML (`.x3d`) files | not tested |

## Rendering

| | item | notes |
| --- | --- | --- |
| ✅ | Geometry, materials, lighting | tests 1, 3; Cybertown room matches X_ITE (match 0.986) |
| ✅ | JPEG textures (Imlib2) | Imlib2 was never enabled in the Mac config; every texture failed before |
| ✅ | MultiTexture | test 2 (X_ITE can't render this one in a VRML97 file) |
| ✅ | Animation (TimeSensor + interpolators) | tests 6, 7 animate |
| ✅ | Text | test 49 renders |
| 🟡 | PNG / GIF textures | should work via Imlib2; not specifically tested |
| ❌ | MovieTexture | disabled: `MPEG_Utils_ffmpeg.c` uses ffmpeg-4 APIs removed in ffmpeg 5+; needs porting |
| 🟡 | Sound (OpenAL/ALUT) | test 50 loads; one "resource failed to load" in its log; audio not checked |
| 🟡 | Fonts for Text / HUD | Mac app looks in `/Applications/FreeWRL/fonts` (missing); text still rendered in test 49, source of glyphs not confirmed |

## Scripting (duktape; SpiderMonkey unavailable on Homebrew)

| | item | notes |
| --- | --- | --- |
| ✅ | Script-driven animation | JS tests SFFloat, SFColor, SFInt32, SFRotation, SFVec3f, MFFloat, MFColor, addDelRoute animate with no script errors |
| ❔ | JS tests SFBool, MFRotation | static; both rely on `eventsProcessed()` with no incoming events, which per spec never runs. Compare with another browser before calling it a bug |
| 🟡 | Click-driven tests (SFTime, 8, 10) | load cleanly; interaction not exercised |

## Tooling

| | item | notes |
| --- | --- | --- |
| ✅ | Visual comparison harness | `tools/visual-test/compare.sh`: FreeWRL vs X_ITE screenshots, NCC score, HTML report |
| ✅ | Screenshot without focus steal | background launch; avoids accidental keystrokes (e.g. `q`) reaching FreeWRL |
| ✅ | `CLAUDE.md` | build, codegen, architecture, Mac notes |

Latest harness run (pass = match ≥ 0.90):

| world | match | result | why |
| --- | --- | --- | --- |
| tests/1.wrl | 0.998 | PASS | |
| tests/2.wrl | 0.915 | PASS | X_ITE rejects MultiTexture in VRML97, so its reference is untextured |
| tests/3.wrl | 0.653 | FAIL | X_ITE rejects `TextureCoordinateGenerator`; reference is wrong, not FreeWRL |
| tests/6.wrl | 0.868 | FAIL | animated; frames captured at different moments |
| cybertown 002/home.wrl | 0.986 | PASS | |

## Changes made (branch `macos-arm64`)

1. **Build**: Homebrew paths, explicit header search paths, deployment target 13.0, `Component_PTM.c` in the project, generated `src_aqua/fwVersion.c`, AGL include replaced, standard headers declared in `config.h`, KHR_debug callback only under `DEBUG_OPENGL`, missing prototypes, duktape, Imlib2 on, ffmpeg movietexture off.
2. **Crashes / Retina**: draw guard until the instance exists; `dllFreeWRL_onDraw` returns status and the app exits after library shutdown; backing-pixel viewport, scaled mouse, HUD density; console → stderr.
3. **Parser**: `NULL` accepted as an empty MFNode.
4. **HTTP**: `pthread_once` curl init; failures log curl error / HTTP status; `fwl_doQuit` callers traced to stderr.
5. **Docs/tools**: `CLAUDE.md`, `tools/visual-test/`, this file.

## Next up

- [ ] Port `MPEG_Utils_ffmpeg.c` to the ffmpeg 5+ API (re-enable MovieTexture)
- [ ] Investigate why the Cybertown EXTERNPROTO file is never fetched
- [ ] Verify `q` quit, mouse picking/navigation (tests 8, 10, SFTime), and audio by hand
- [ ] Bundle dylibs + ad-hoc sign for a standalone `FreeWRL.app`
- [ ] Fonts: ship `VeraMono.ttf` in the bundle or point at system fonts
- [ ] Remove or gate the `fwl_doQuit` stderr trace once quit is confirmed
- [ ] Add more Cybertown worlds to the harness; consider an X3D-profile reference for tests X_ITE rejects
