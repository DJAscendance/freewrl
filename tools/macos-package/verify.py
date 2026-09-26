#!/usr/bin/env python3
"""Portability gate for a packaged FreeWRL.app.

usage: verify.py [--source-root DIR] [--macos VERSION] <FreeWRL.app>

Fails (exit 1) if any Mach-O in the bundle
  - depends on a non-system library that does not resolve inside the bundle,
  - has an install name, dependency or LC_RPATH naming Homebrew, /usr/local,
    MacPorts, the source tree, a temporary directory or a home directory,
  - is not arm64, not for macOS, or needs a newer macOS than
    LSMinimumSystemVersion claims or than --macos (the oldest macOS the package must
    run on),
or if LSMinimumSystemVersion is newer than --macos, one of the libraries FreeWRL
links (FreeType, ODE, freealut) is not embedded, a library the macOS build no longer
uses (Imlib2, FFmpeg, OpenAL Soft) is embedded, an expected runtime file (fonts,
licenses) is missing, or a package in MANIFEST.tsv or a component compiled into
FreeWRL has no license file recorded in LICENSES.tsv.
Paths that only appear as strings inside binaries are listed as warnings.
"""
import argparse
import os
import plistlib
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from machoinfo import MachO, is_macho, is_system, resolve, version_tuple  # noqa: E402

FORBIDDEN = ["/opt/homebrew/", "/usr/local/", "/opt/local/", "/sw/", "/tmp/", "/private/",
             "/var/folders/", "/Users/", "/Volumes/"]
# files FreeWRL itself opens at runtime
EXPECTED = [
    "Contents/Resources/fonts/VeraMono.ttf",     # FWGLView.m locates fonts/ by this file
    "Contents/Resources/fonts/Vera.ttf",
    "Contents/Resources/fonts/VeraBd.ttf",
    "Contents/Resources/fonts/VeraIt.ttf",
    "Contents/Resources/fonts/VeraBI.ttf",
    "Contents/Resources/fonts/VeraSe.ttf",
    "Contents/Resources/fonts/VeraSeBd.ttf",
    "Contents/Resources/fonts/VeraMoBd.ttf",
    "Contents/Resources/fonts/VeraMoIt.ttf",
    "Contents/Resources/fonts/VeraMoBI.ttf",
    "Contents/Resources/ThirdPartyLicenses/MANIFEST.tsv",
    "Contents/Resources/ThirdPartyLicenses/LICENSES.tsv",
    "Contents/Resources/ThirdPartyLicenses/freetype/LICENSE.TXT",  # refers to FTL.TXT
    "Contents/Resources/ThirdPartyLicenses/freetype/FTL.TXT",
]
# libraries FreeWRL links that tools/macos-deps builds, which must be embedded
EMBEDDED = ["Contents/Frameworks/libfreetype.6.dylib",
            "Contents/Frameworks/libode.8.dylib",
            "Contents/Frameworks/libalut.0.dylib"]
# libraries the macOS build replaced (stb_image, Apple's OpenAL) or turned off (FFmpeg)
UNWANTED = ["libImlib2", "libavcodec", "libavformat", "libavutil", "libavdevice",
            "libswscale", "libswresample", "libopenal"]
# license files of code compiled into FreeWRL (package.sh)
COMPILED_IN = ["FreeWRL", "duktape", "libtess", "stb_image"]


def read_tsv(path):
    with open(path) as f:
        return [line.rstrip("\n").split("\t") for line in f][1:]


def check_licenses(licdir):
    """Every embedded package and compiled-in component has license files, and
    every file LICENSES.tsv lists is there."""
    try:
        packages = {r[2] for r in read_tsv(os.path.join(licdir, "MANIFEST.tsv"))}
        rows = read_tsv(os.path.join(licdir, "LICENSES.tsv"))
    except OSError:
        return []  # reported as missing above
    errors = []
    for r in rows:
        if len(r) != 4 or not all(r):
            errors.append("LICENSES.tsv: bad row %r" % (r,))
        elif not os.path.isfile(os.path.join(licdir, r[0], r[2])):
            errors.append("LICENSES.tsv lists %s/%s, not in the bundle" % (r[0], r[2]))
    for pkg in sorted(packages | set(COMPILED_IN)):
        if not any(r[0] == pkg for r in rows):
            errors.append("no license file recorded for %s" % pkg)
    print("licenses: %d files for %d packages" % (len(rows), len({r[0] for r in rows})))
    return errors


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", action="append", default=[],
                    help="also forbid paths under this directory (repeatable)")
    ap.add_argument("--macos", help="oldest macOS the package must run on, e.g. 14.0")
    ap.add_argument("app")
    a = ap.parse_args()
    app = os.path.abspath(a.app)
    contents = os.path.join(app, "Contents")
    forbidden = FORBIDDEN + [os.path.abspath(r) + "/" for r in a.source_root]
    errors, warnings = [], []

    with open(os.path.join(contents, "Info.plist"), "rb") as f:
        info = plistlib.load(f)
    exe = os.path.join(contents, "MacOS", info["CFBundleExecutable"])
    claimed = info.get("LSMinimumSystemVersion", "0")
    exe_rpaths = MachO(exe).rpaths

    def bad(p):
        return next((x for x in forbidden if p.startswith(x)), None)

    machos = sorted(p for p in (os.path.join(r, n) for r, _, ns in os.walk(contents) for n in ns)
                    if is_macho(p))
    newest = "0"
    print("%-58s %-6s %-6s %-6s %s" % ("Mach-O", "arch", "minos", "sdk", "deps (non-system)"))
    for p in machos:
        rel = os.path.relpath(p, app)
        m = MachO(p)
        if m.archs != ["arm64"]:
            errors.append("%s: architectures %s, expected arm64" % (rel, m.archs))
        if m.platform not in ("MACOS", "1"):
            errors.append("%s: platform %s" % (rel, m.platform))
        if version_tuple(m.minos) > version_tuple(newest):
            newest = m.minos
        if version_tuple(m.minos) > version_tuple(claimed):
            errors.append("%s: needs macOS %s, Info.plist claims %s" % (rel, m.minos, claimed))
        if a.macos and version_tuple(m.minos) > version_tuple(a.macos):
            errors.append("%s: needs macOS %s, newer than --macos %s" % (rel, m.minos, a.macos))
        if any(os.path.basename(p).startswith(u) for u in UNWANTED):
            errors.append("%s: the macOS build does not use this library" % rel)
        if m.id and (bad(m.id) or not (m.id.startswith("@rpath/") or p == exe)):
            errors.append("%s: install name %s" % (rel, m.id))
        for rp in m.rpaths:
            if bad(rp) or not rp.startswith(("@executable_path", "@loader_path")):
                errors.append("%s: LC_RPATH %s" % (rel, rp))
            elif not os.path.realpath(rp.replace("@executable_path", os.path.dirname(exe))
                                        .replace("@loader_path", os.path.dirname(p))).startswith(contents):
                errors.append("%s: LC_RPATH %s leaves the bundle" % (rel, rp))
        local = []
        for dep in m.deps:
            if is_system(dep):
                continue
            if bad(dep):
                errors.append("%s: depends on %s" % (rel, dep))
                continue
            # a dlopen'ed plugin also searches the executable's run paths
            found = resolve(dep, p, m.rpaths + exe_rpaths, exe)
            if not found:
                errors.append("%s: %s does not resolve" % (rel, dep))
            elif not os.path.realpath(found).startswith(contents + "/"):
                errors.append("%s: %s resolves outside the bundle: %s" % (rel, dep, found))
            local.append(os.path.basename(dep))
        print("%-58s %-6s %-6s %-6s %s" % (rel, ",".join(m.archs), m.minos, m.sdk, " ".join(local)))
        strs = subprocess.run(["strings", "-a", p], capture_output=True, text=True).stdout.splitlines()
        hits = sorted({s.strip() for s in strs if any(x in s for x in forbidden if x not in ("/tmp/", "/private/"))})
        if hits:
            warnings.append("%s: %d embedded path string(s), e.g. %s" % (rel, len(hits), hits[0]))

    if a.macos and version_tuple(claimed) > version_tuple(a.macos):
        errors.append("LSMinimumSystemVersion %s is newer than --macos %s" % (claimed, a.macos))
    for e in EMBEDDED:
        if not os.path.isfile(os.path.join(app, e)):
            errors.append("not embedded: %s" % e)
    for e in EXPECTED:
        if not os.path.isfile(os.path.join(app, e)):
            errors.append("missing %s" % e)
    errors += check_licenses(os.path.join(contents, "Resources", "ThirdPartyLicenses"))

    print("\n%d Mach-O files; newest minimum macOS %s; LSMinimumSystemVersion %s" % (len(machos), newest, claimed))
    for w in warnings:
        print("warning:", w)
    for e in errors:
        print("ERROR:", e)
    if errors:
        print("FAIL: %d error(s)" % len(errors))
        sys.exit(1)
    print("PASS: every non-system dependency resolves inside the bundle")


if __name__ == "__main__":
    main()
