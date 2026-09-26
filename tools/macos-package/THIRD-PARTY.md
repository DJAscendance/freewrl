# Third-party libraries in the macOS app

What the standalone `FreeWRL.app` built by `package.sh` contains and which
distribution materials it ships. This records measured facts. It is not legal
advice, and it does not claim that the package complies with any license.

In the app:

- `Contents/Resources/ThirdPartyLicenses/<package>/`: each package's license files.
- `MANIFEST.tsv`: every embedded binary → Homebrew package and version.
- `LICENSES.tsv`: every license file → package, version and where it was copied from.

Each embedded package's license files come from its Homebrew keg. When a keg
lacks a file its license refers to, the file comes from the package's source
archive: see `licenses/<package>/<version>/SOURCE`. The only case so far is
FreeType, whose `LICENSE.TXT` refers to `docs/FTL.TXT`.

## LGPL-licensed libraries

Both are Homebrew bottles, copied unmodified apart from install names and
signatures. Checked on the 2026-09-25 candidate.

| | freealut | ode |
| --- | --- | --- |
| version | 1.1.0 | 0.16.6 |
| license (Homebrew formula) | `LGPL-2.0-only` | `LGPL-2.1-or-later OR BSD-3-Clause` |
| license files in the app | `freealut/COPYING` (GNU Library GPL v2) | `ode/COPYING` (LGPL 2.1), `ode/LICENSE.TXT` (states the dual license), `ode/LICENSE-BSD.TXT` |
| linking | dynamic: `Contents/Frameworks/libalut.0.dylib`, loaded by FreeWRL as `@rpath/libalut.0.dylib` | dynamic: `Contents/Frameworks/libode.8.dylib`, `@rpath/libode.8.dylib` |
| version shown to users | `MANIFEST.tsv`, `LICENSES.tsv` | same |
| upstream source | Homebrew fetches `https://deb.debian.org/debian/pool/main/f/freealut/freealut_1.1.0.orig.tar.gz` (sha256 `60d1ea87…8df44f`) | `https://bitbucket.org/odedevs/ode/downloads/ode-0.16.6.tar.gz` (sha256 `c91a28c6…4513e8`) |

Replacing an embedded library, tested with freealut:

- The dylib is a separate file. A user can swap in their own build of
  `libalut.0.dylib`, with install name `@rpath/libalut.0.dylib`.
- Doing so breaks the app's signature: `codesign --verify` reports "nested code
  is modified or invalid", and Gatekeeper refuses to open the app ("is damaged
  and can't be opened").
- Re-signed by the user (`codesign --force --deep -s - FreeWRL.app`), the app
  starts and maps the replacement library (seen with `lsof`). It is then no
  longer the Developer ID-signed, notarized app.

## Other licenses to note

- zstd (`BSD-3-Clause OR GPL-2.0-only`, plus BSD-2-Clause and MIT parts) and xz
  (liblzma is 0BSD; Homebrew's `GPL-2.0-or-later` covers xz's tools, which are
  not bundled). Both are present only because Imlib2's TIFF and WebP loaders
  need them.
- Code compiled into FreeWRL: FreeWRL (LGPL-3.0-or-later, `freex3d/COPYING.LESSER`
  plus the GPL v3 it builds on), Duktape 2.0.0 (MIT) and libtess (SGI Free
  Software License B). Their texts are copied from the source tree.
- Apple frameworks and `/usr/lib` libraries are not copied into the app.
