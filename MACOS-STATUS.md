# FreeWRL on macOS (Apple Silicon): develop port status

Fork: https://github.com/DJAscendance/freewrl, branch `macos-arm64-develop-port` (local candidate, not pushed).
Base: upstream SourceForge `develop` @ `b3254b11e` (2024-04-20, "Version 6.7", FreeWRL 6.7.0).
Reference: branch `macos-arm64` ([PR #1](https://github.com/DJAscendance/freewrl/pull/1)), the Mac port of upstream `master` @ `e99ab4a00`.
Tested on: MacBook Pro M1, macOS 27.0, Xcode 27.0, Homebrew. Last updated 2026-09-25.

Legend: ✅ verified (with evidence) · 🟡 implemented, not tested · ⛔ unsupported on macOS · ❌ blocked / broken · ❔ open question

## Upstream lines

| line | commit | date | version | notes |
| --- | --- | --- | --- | --- |
| upstream `master` | `e99ab4a00` | 2020-02-21 | 4.4.0 | older stable line, unchanged since 2020 |
| upstream `develop` | `b3254b11e` | 2024-04-20 | 6.7.0 | active line; 951 commits ahead of `master`, which it fully contains |

## OpenGL

FreeWRL 6.x renders with GLSL 330+ shaders. macOS supports OpenGL only up to 4.1 core, so the app creates a 4.1 core context and the library runs a core-profile path (`FW_GL_CORE_PROFILE`, mostly `freex3d/src/lib/opengl/GLCoreCompat.c`).

| | item | evidence / notes |
| --- | --- | --- |
| ✅ | 4.1 core context | startup log: `GL_VERSION 4.1 Metal - 91.7`, `GL_SHADING_LANGUAGE_VERSION 4.10`, `GL_RENDERER Apple M1` |
| ✅ | Ubershader at GLSL 410 | `get_GLSL_max_version()` returns 410; no shader compile or link errors in the logs of tests 1, 2, 3, 6, 16, 49, 50 or Cybertown 002 |
| ✅ | HUD and Text shaders | GLES2-style sources get a `#version 410 core` prelude; before that there were 538 compile errors on test 1 |
| ✅ | Default VAO | bound at GL init; `renderQuad` restores it instead of binding 0 |
| ✅ | Client-memory vertex/index arrays | streamed into VBOs at draw time; Box (test 2, 16), HUD and Text draw |
| ✅ | Sampler/texture-unit conflicts | unused sampler2D/samplerCube uniforms sat on unit 0; core rejected every scene draw (`GL_INVALID_OPERATION`, black scene). Samplers are now parked per type on spare units at each shape |
| ✅ | GL_ALPHA / GL_LUMINANCE_ALPHA textures | uploaded as R8/RG8 with a swizzle; HUD icons and Text glyphs render |
| 🟡 | `glBindTextureUnit(unit, 0)` emulation | unbinds 2D, cube and 3D on the unit with the active unit preserved; asserts on a non-zero texture (the library never passes one) |
| 🟡 | `glCheckNamedFramebufferStatus` emulation | binds, checks, restores; only used by GeneratedCubeMap/GeneratedTexture/shadow code, none of it exercised yet |
| ⛔ | HAnim GPU skinning | needs shader storage buffers (GL 4.3). HAnim switches to its existing CPU skinning (`rdr_caps.av_ssbo`, logged at startup). No HAnim world tested yet |
| ⛔ | Wide lines (`glLineWidth` > 1) | not in a core profile; affects LineProperties line types. Not tested |
| ⛔ | GLSL 450 cube shader | exists but is dead code upstream (`if (0)`) |

## Build

| | item | notes |
| --- | --- | --- |
| ✅ | Release arm64, clean | `xcodebuild -project OSX_gui/FreeWRL-Desktop/FreeWRL.xcodeproj -scheme FreeWRL -configuration Release ARCHS=arm64 CODE_SIGN_IDENTITY=- -derivedDataPath <dir> build` → exit 0; 1113 warning lines (unused variables, unreachable code, visibility) |
| ✅ | Debug arm64, clean | same with `-configuration Debug` → exit 0. It failed before: the Debug target overrode the header search paths and linker flags |
| ✅ | Homebrew dylibs | ode, ffmpeg (avcodec 63, avformat 63, avutil 61, swscale 10, swresample 7), openal-soft, freealut, imlib2, freetype |
| ❌ | Standalone / distributable app | still links `/opt/homebrew`; needs bundling and signing |
| ❔ | ffmpeg linked but unused | `MOVIETEXTURE_FFMPEG` is off; the project still links the dylibs |

## Runtime (Release build)

| | item | evidence |
| --- | --- | --- |
| ✅ | tests/1.wrl | lit cone; harness match 0.995 vs X_ITE |
| ✅ | JPEG textures, MultiTexture, Box | tests/2.wrl; match 0.951 |
| ✅ | Background, Inline, Sphere, Box | tests/16.wrl; match 0.920 |
| ✅ | Animation + ROUTEs | tests/6.wrl cones animate; tests/49.wrl text cycles through its strings |
| ✅ | Text | tests/49.wrl renders glyphs |
| ✅ | Cybertown 002 over HTTP | renders the room; match 0.966, no parse/GL/download errors |
| ❔ | Brightness | 6.7 renders Cybertown brighter than master and X_ITE (mean luminance 0.651 vs 0.566 on master). Upstream reworked lighting on `develop` (Phong/Gouraud, PBR, sRGB helpers). Not yet compared against a Windows 6.7 build, so not attributed |
| 🟡 | HUD layout | 6.x's HUD has ~27 buttons; at Retina density in a 672-pt window it wraps to two rows and the 3D view stops above it. Works, but reported as not fitting the menu bar; needs a look |
| 🟡 | Sound | tests/50.wrl loads; one "resource failed to load" (same as on master); audio not checked |
| 🟡 | Mouse picking / navigation, `q` quit | not re-tested on this branch |

## Visual harness (tests: 1, 2, 3, 6, 16, 49; Cybertown 002)

| world | match | result |
| --- | --- | --- |
| tests/1.wrl | 0.9947 | PASS |
| tests/2.wrl | 0.9508 | PASS |
| tests/3.wrl | 0.8358 | REFERENCE_INVALID (X_ITE rejects TextureCoordinateGenerator) |
| tests/6.wrl | 0.8684 | NONDETERMINISTIC (animated) |
| tests/16.wrl | 0.9197 | PASS |
| tests/49.wrl | 0.6015 | NONDETERMINISTIC (text cycles; different string captured) |
| cybertown 002/home.wrl | 0.9660 | PASS |

## Fixed on this branch (upstream bugs, all platforms)

- ROUTE parsing: since `7615eadcd` (Feb 2024) `node.field` lexed as one identifier, so every classic VRML ROUTE failed, and the error path aborted the parse thread (freeing uninitialized pointers). Crashed tests 2, 16 and 49.
- Missing prototypes, including four pointer-returning functions that an implicit declaration truncates on 64-bit targets.
- A variable defined in `Component_Shape.h` (duplicate symbol under clang's default `-fno-common`).

## Next up

- [ ] HUD: fit the menu bar at Retina density (fewer rows or a smaller scale)
- [ ] Brightness: compare with a Windows 6.7 build or check the shading/gamma path before calling it a regression
- [ ] Test an HAnim world (CPU skinning fallback), a LineProperties world, GeneratedCubeMapTexture and shadows
- [ ] Re-verify mouse picking/navigation and quit
- [ ] Port `MPEG_Utils_ffmpeg.c` to ffmpeg 5+; bundle dylibs and sign
