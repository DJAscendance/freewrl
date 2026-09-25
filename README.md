<p align="center">
  <img src="docs/assets/freewrl-logo-128.png" alt="FreeWRL logo" width="128" height="128">
</p>

# FreeWRL

FreeWRL is an open-source X3D and VRML97 browser written in C. It runs as a
standalone application, as a browser plugin, or as an embeddable library
(`libFreeWRL`), with JavaScript Script nodes, EAI/SAI, and a mix of desktop
and mobile platform targets.

## Fork notice

**This repository is not the official FreeWRL project.**

- **Original project.** FreeWRL is the original open-source VRML97/X3D
  browser hosted on SourceForge: <https://sourceforge.net/projects/freewrl/>.
  FreeWRL was written by its original authors and contributors, who retain
  their copyrights.
- **This repository.** This GitHub repository is Ryan Bundy's
  (DJAscendance) fork and development mirror of that project. It is not the
  official upstream.
- **Current modernization.** The fork is restoring modern platform support,
  starting with native Apple Silicon macOS support for the FreeWRL 6.7 code
  line.

Bugs in FreeWRL itself belong upstream. Issues specific to the changes in
this fork belong on this repository.

## Current fork status

| Branch | What it is |
| --- | --- |
| `develop` | Upstream SourceForge `develop` at `b3254b11e` ("Version 6.7", 2024-04-20) plus this fork's merged work, including native Apple Silicon macOS support. |
| `master` | Exact mirror of upstream `master` at `e99ab4a00` (2020-02-21), the older stable line. |
| `macos-arm64-develop-port` | The Apple Silicon port of FreeWRL 6.7, merged into `develop` through [pull request #2](https://github.com/DJAscendance/freewrl/pull/2). |
| `macos-arm64` | An earlier Mac port of the 2020 `master` line, kept for reference. |

## Version

The fork's `develop` branch is based on FreeWRL 6.7:

- SourceForge `develop` commit `b3254b11e`, the base of this fork's
  `develop`, is titled `Version 6.7`.
- `freex3d/src/buildversion.h` reports version `6.7.0`.
- `freex3d/versions/FREEWRL` is stale and still reports `5.0.0`.

## Supported formats

- VRML97 (`.wrl`, classic encoding), including gzipped files
- X3D XML encoding (`.x3d`)
- X3D classic VRML encoding (`.x3dv`)
- Collada and STL import (partial)
- Textures: JPEG, PNG, GIF
- Resources loaded from local files or over HTTP(S)

## macOS Apple Silicon status

**Native Apple Silicon macOS source support is available on this fork's
`develop` branch.** It was reviewed and merged through
[pull request #2](https://github.com/DJAscendance/freewrl/pull/2).

- Release and Debug arm64 builds pass with Xcode and Homebrew libraries.
- FreeWRL runs on an OpenGL 4.1 core context on Apple Silicon (the highest
  version macOS offers). Rendering still uses OpenGL; there is no Metal
  renderer.
- Retina interaction has been tested: keyboard hotkeys including `q` quit,
  held-key navigation, mouse picking, HUD clicks, and sensor drag.
- VRML97 and X3D rendering tests and the Cybertown tests passed.
- There is **no standalone downloadable Mac package yet.** The app still links
  Homebrew runtime dylibs, so it only runs on a Mac with those libraries
  installed.

Current QA: the final tested Mac candidate is
`32caaa36a845fc668c9fd36cd2cfd8b047c46733`, with interaction QA token
`FREEWRL_6_7_MACOS_ARM64_GL41_KEYBOARD_AND_INTERACTION_QA_PASS`. The manual
checks are recorded in
[`docs/MANUAL-INTERACTION-CHECKLIST.md`](docs/MANUAL-INTERACTION-CHECKLIST.md),
and the full review history is on
[pull request #2](https://github.com/DJAscendance/freewrl/pull/2).

Detailed engineering status, per-feature evidence, and the OpenGL
compatibility layer are in [`MACOS-STATUS.md`](MACOS-STATUS.md).

## Build instructions

### Linux (autotools)

```sh
cd freex3d
./autogen.sh
./configure --help        # review the available options
./configure --with-target=x11 --with-javascript=duk
make
sudo make install
```

Useful options include `--with-target` (`x11`, `motif`), `--with-javascript`
(`duk` for the bundled duktape, `sm` for SpiderMonkey, `stub` for none),
`--enable-libeai`, and `--enable-debug`.

### macOS (Apple Silicon)

Build from `develop`:

```sh
brew install freetype imlib2 openal-soft freealut ode ffmpeg libxml2
git checkout develop
cd OSX_gui/FreeWRL-Desktop
xcodebuild -project FreeWRL.xcodeproj -scheme FreeWRL \
  -configuration Release ARCHS=arm64 CODE_SIGN_IDENTITY=- build
```

### Windows

Visual Studio 2022 projects are in `freex3d/projectfiles_2022/` (and older
`projectfiles_vc7/`). They have not been changed by this fork.

### Trying it

```sh
freewrl freewrl/tests/1.wrl
```

The numbered worlds in `freewrl/tests/` are described in
`freewrl/tests/README`.

## Original SourceForge project

- Project page: <https://sourceforge.net/projects/freewrl/>
- Source repository: <https://sourceforge.net/p/freewrl/git/>
- Clone: `git clone https://git.code.sf.net/p/freewrl/git freewrl`

## Ryan's SourceForge fork

- <https://sourceforge.net/u/djascendance/freewrl/>
- Clone: `git clone https://git.code.sf.net/u/djascendance/freewrl`

## GitHub mirror and fork

- <https://github.com/DJAscendance/freewrl>
- Pull requests here are how fork changes are reviewed before they are
  offered upstream.

## Repository layout

| Path | Contents |
| --- | --- |
| `freex3d/` | Core engine source (`src/lib`), standalone executable (`src/bin`), autotools build, code generator (`codegen/`), icons |
| `OSX_gui/` | Xcode projects for macOS desktop and iOS |
| `freex3d/projectfiles_*` | Visual Studio projects for Windows |
| `Android/` | Android NDK build |
| `linux_appimage/` | Scripts that bundle an installed FreeWRL into an AppImage |
| `freewrl/tests/` | Numbered sample VRML/X3D worlds |
| `SoundEngine/` | Separate sound engine |
| `docs/` | Fork documentation and web images |

## Known limitations

On macOS:

- OpenGL stops at version 4.1, and Apple has deprecated OpenGL.
- HAnim uses CPU skinning; GPU skinning needs features newer than GL 4.1.
- Lines are always drawn one pixel wide.
- The app depends on Homebrew libraries and is not yet a distributable bundle.

Known FreeWRL 6.7 defects, present upstream and not introduced by the port:

- `GeneratedCubeMapTexture` renders a black reflection.
- `ComposedCubeMapTexture` casts a field to the wrong node structure.
- Directional-light shadows darken areas outside the shadow map.
- `HAnimHumanoid` ignores its own transform fields.

## Contributing

1. Base work on `develop`, not the 2020 `master`.
2. Use a `feature-*` or `fix-*` branch and open a pull request against
   `develop`.
3. Keep every platform compiling; much of the code is conditional on
   platform defines.
4. Node definitions are generated: edit `freex3d/codegen/*.pm` and run
   `perl VRMLC.pm` from `freex3d/codegen/` rather than editing the generated
   files.
5. Fixes to FreeWRL itself are welcome upstream too, on the SourceForge
   project.

## License and attribution

FreeWRL was written by its original authors and the FreeWRL/FreeX3D
contributors, who retain their copyrights. Most source files carry the
notice "Copyright 2009 CRC Canada"; some files name other copyright holders.

The source headers license FreeWRL under the GNU Lesser General Public
License, version 3 or (at your option) any later version. The repository
ships the LGPL v3 text in `freex3d/COPYING.LESSER` and the GNU GPL v3 text,
which the LGPL builds on, in `freex3d/COPYING`. The header boilerplate also
refers to the GPL in its warranty and "copy of the license" lines. Bundled
third-party code (for example SpiderMonkey in `freewrl/JS/`, duktape, libtess,
minizip) keeps its own license.

This fork's changes are offered under the same license. The FreeWRL logo is
the project's own artwork; see [`docs/assets/README.md`](docs/assets/README.md).
