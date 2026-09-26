# macOS standalone packaging

Builds a `FreeWRL.app` for Apple Silicon that runs without Homebrew: every
non-system library it needs is inside the bundle.

```sh
tools/macos-package/package.sh                   # build Release, package, ad-hoc sign, verify
tools/macos-package/package.sh -z                # ... and zip it (prints the SHA-256)
tools/macos-package/package.sh -s "Developer ID Application" -r -z   # Developer ID, hardened runtime
tools/macos-package/verify.py FreeWRL.app        # the portability gate on its own
```

Output goes to `./macos-package-out` (`-o` to change). `-a <app>` packages an
existing Release build instead of building one. The signing identity is any
`codesign -s` value; the default `-` is ad-hoc.

Packaging needs the Homebrew libraries the Xcode project links (ode, freealut,
imlib2, freetype and what they depend on). The packaged app doesn't.

## What it does

1. `xcodebuild` Release, arm64 (skipped with `-a`).
2. `bundle.py`: walks the executable's dependencies recursively, copies every
   library outside `/usr/lib` and `/System/Library` into `Contents/Frameworks`,
   and copies Imlib2's image loaders (which it `dlopen`s, so `otool` can't see
   them) into `Contents/PlugIns/imlib2/loaders`. Install names become
   `@rpath/<name>`; the run paths are `@executable_path/../Frameworks`
   (executable), `@loader_path` (Frameworks) and `@loader_path/../../../Frameworks`
   (loaders). `main.m` points `IMLIB2_LOADER_PATH` at the bundled loaders.
   Copies each Homebrew package's license files to
   `Contents/Resources/ThirdPartyLicenses/`, plus license files a keg lacks from
   `licenses/<package>/<version>/` (FreeType's `FTL.TXT`; `SOURCE` says where
   each came from), writes `MANIFEST.tsv` (binary → package, version) and
   `LICENSES.tsv` (license file → package, version, source), and sets
   `LSMinimumSystemVersion` to the newest minimum macOS of any binary in the
   bundle.
3. License texts of code compiled into FreeWRL (FreeWRL, Duktape, libtess),
   copied verbatim from the source tree.
4. Signs inside out: each dylib and loader, then the app.
5. `verify.py` and `codesign --verify --deep --strict`.

See [THIRD-PARTY.md](THIRD-PARTY.md) for the embedded libraries and their licenses.

## verify.py

Fails if any Mach-O in the bundle has a dependency, install name or run path
that points outside the bundle (other than Apple's `/usr/lib` and
`/System/Library`) or at Homebrew, `/usr/local`, MacPorts, the source tree, a
temporary or home directory; if a binary isn't arm64/macOS or needs a newer
macOS than `LSMinimumSystemVersion`; if the loaders, fonts, license manifests or
FreeType's `LICENSE.TXT`/`FTL.TXT` are missing; or if an embedded package or a
component compiled into FreeWRL has no license file in `LICENSES.tsv`, or
`LICENSES.tsv` lists a file that isn't there. Paths that only appear as strings
inside a binary are warnings (Imlib2's and libX11's compiled-in data
directories, `__FILE__` names in FreeWRL's asserts); nothing opens them at run
time.

## Hardened runtime

Needs a real signing identity. With an ad-hoc signature there is no Team ID, so
library validation rejects every bundled dylib ("mapping process and mapped
file (non-platform) have different Team IDs") and the app doesn't start. Signed
with one Developer ID identity, everything shares a Team ID and no entitlements
are needed: FreeWRL uses no JIT or writable-executable memory (Duktape is an
interpreter), loads no libraries signed by others, and reads no `DYLD_`
variables.
