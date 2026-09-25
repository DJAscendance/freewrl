#!/usr/bin/env python3
"""Portability gate for a packaged FreeWRL.app.

usage: verify.py [--source-root DIR] <FreeWRL.app>

Fails (exit 1) if any Mach-O in the bundle
  - depends on a non-system library that does not resolve inside the bundle,
  - has an install name, dependency or LC_RPATH naming Homebrew, /usr/local,
    MacPorts, the source tree, a temporary directory or a home directory,
  - is not arm64, not for macOS, or needs a newer macOS than
    LSMinimumSystemVersion claims,
or if an expected runtime file (Imlib2 loaders, fonts, licenses) is missing.
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
    "Contents/PlugIns/imlib2/loaders/jpeg.so",   # JPEG, PNG, GIF: the X3D image formats
    "Contents/PlugIns/imlib2/loaders/png.so",
    "Contents/PlugIns/imlib2/loaders/gif.so",
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
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-root", action="append", default=[],
                    help="also forbid paths under this directory (repeatable)")
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

    for e in EXPECTED:
        if not os.path.isfile(os.path.join(app, e)):
            errors.append("missing %s" % e)

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
