# FreeWRL on macOS (Apple Silicon): develop port status

Fork: https://github.com/DJAscendance/freewrl, branch `macos-arm64-develop-port` (local candidate, not pushed).
Base: upstream SourceForge `develop` @ `b3254b11e` (2024-04-20, "Version 6.7", FreeWRL 6.7.0).
Reference: branch `macos-arm64` ([PR #1](https://github.com/DJAscendance/freewrl/pull/1)), the same Mac work on upstream `master` @ `e99ab4a00`. That port builds and runs, and its verified checklist lives in its own `MACOS-STATUS.md`.
Last updated 2026-09-25.

## Upstream lines

| line | commit | date | version | notes |
| --- | --- | --- | --- | --- |
| upstream `master` | `e99ab4a00` | 2020-02-21 | 4.4.0 | older stable line, unchanged since 2020 |
| upstream `develop` | `b3254b11e` | 2024-04-20 | 6.7.0 | active line; 951 commits ahead of `master`, which it fully contains |

`e99ab4a00` is the last commit on upstream `master`, but upstream kept working after it. `develop` is the current code.

## Status on develop

| | item | notes |
| --- | --- | --- |
| ❌ | Xcode Release arm64 build | stops in `OpenGL_Utils.c` (`GL_MAX_VERTEX_OUTPUT_COMPONENTS`) and `RenderFuncs.c` (`glVertexAttribIPointer`): the renderer needs GL 3.2+ |
| ✅ | Project file list | `Component_PTM.c` → `Component_TextureProjector.c`; added `gltf_loader.c`, `BVHreader.c`, `Component_MIDI.c`, `input/Decompose.c` |
| ✅ | clang/MSVC-only fixes so far | missing prototypes, `<GL/glu.h>` on Apple, rvalue address-of in `gltf_loader.c`, `errno.h` |
| 🟡 | Runtime fixes from `macos-arm64` | Retina/mouse, draw guard, quit via `dllFreeWRL_onDraw` return, `MFNode NULL`, curl init. They apply unchanged because the functions they touch are the same on `develop`. Not run yet |
| ❔ | Visual harness / Cybertown | not run: no build |

## Blocker: OpenGL level

`develop` moved the renderer from GLSL 1.x (as on `master`) to desktop GL 3.x/4.x:

- The uber-shader takes its version from `get_GLSL_max_version()` and always adds `core`. On the Mac's legacy 2.1 context that gives `#version 120 core`, which is invalid.
- Shadow, depth, and quad shaders are hard-coded to `#version 330 core`, and the cube shader to `#version 450 core`.
- HAnim skinning uses shader storage buffers (GL 4.3), which macOS does not have at any version.
- Texture units use `glBindTextureUnit` (GL 4.5). This branch emulates it in `display.h`.
- There are integer vertex attributes (`glVertexAttribIPointer`, GL 3.0) and `GL_MAX_VERTEX_OUTPUT_COMPONENTS` (GL 3.2).

Moving forward requires a design decision:

1. Create a GL 4.1 core context (`NSOpenGLProfileVersion4_1Core`). That means a VAO bound at all times, GLSL capped at 410 (`450` → `410`), a fallback for the HAnim SSBO path, and checking every compatibility-profile call the library still makes.
2. Or run on Metal through ANGLE / MoltenGL. That is a bigger change and not evaluated here.

## Changes on this branch

1. **Build (WIP)**: Xcode project, `config.h`, xib, AGL, and KHR_debug changes, all as on `macos-arm64`. `fwVersion.c` follows `buildversion.h`. Updated file list, compile fixes for clang, GL 4.5 shim.
2. **Runtime**: cherry-picked from `macos-arm64` (Retina/crash/quit, `MFNode NULL`, curl), without the `fwl_doQuit` stderr trace.
3. **Docs/tools**: `CLAUDE.md` corrected for upstream `master`/`develop`, `tools/visual-test/`, this file.
