#!/usr/bin/env python3
"""Make FreeWRL.app self-contained: copy its non-system dylibs (recursively) into
Contents/Frameworks, Imlib2's image loaders into Contents/PlugIns, rewrite every
install name to @rpath, and record what was embedded.

usage: bundle.py <FreeWRL.app>

Writes Contents/Resources/ThirdPartyLicenses/ (license files of each embedded
Homebrew package, plus any from licenses/<package>/<version>/; MANIFEST.tsv maps
binaries to packages, LICENSES.tsv license files to their source) and sets LSMinimumSystemVersion to the
highest minimum macOS of any Mach-O in the bundle. Leaves code unsigned:
package.sh signs afterwards.
"""
import filecmp
import glob
import os
import plistlib
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from machoinfo import MachO, is_macho, is_system, resolve, run, version_tuple  # noqa: E402

# Libraries that load plugins with dlopen, which otool can't see:
# linked library basename -> (plugin dir relative to the library's real directory,
#                             destination inside Contents/PlugIns)
PLUGIN_DIRS = {
    "libImlib2.1.dylib": ("imlib2/loaders", "imlib2/loaders"),
}
LICENSE_RE = re.compile(r"^(COPYING|COPYRIGHT|LICEN[CS]E|NOTICE|PATENTS)|-LICEN[CS]E$", re.I)
# licenses/<package>/<version>/: license files a keg doesn't have, with their SOURCE
EXTRA_LICENSES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "licenses")
# files that must end up in ThirdPartyLicenses/<package>/ (freetype's LICENSE.TXT
# says the license is docs/FTL.TXT or docs/GPLv2.TXT; Homebrew installs neither)
REQUIRED_LICENSES = {
    "freetype": ["LICENSE.TXT", "FTL.TXT"],
}


def die(msg):
    sys.exit("bundle.py: " + msg)


def keg_of(path):
    """/opt/homebrew/Cellar/<name>/<version>/... -> (name, version, keg dir)"""
    m = re.match(r"^(.*/Cellar/([^/]+)/([^/]+))/", path)
    return (m.group(2), m.group(3), m.group(1)) if m else (None, None, None)


def main():
    if len(sys.argv) != 2:
        die(__doc__)
    app = os.path.abspath(sys.argv[1])
    contents = os.path.join(app, "Contents")
    with open(os.path.join(contents, "Info.plist"), "rb") as f:
        info = plistlib.load(f)
    exe = os.path.join(contents, "MacOS", info["CFBundleExecutable"])
    fwdir = os.path.join(contents, "Frameworks")
    plugdir = os.path.join(contents, "PlugIns")
    os.makedirs(fwdir, exist_ok=True)

    # in-bundle file -> the original file its @rpath/@loader_path refs resolve against
    source = {exe: exe}
    queue = [exe]
    bundled = {}   # real source path -> bundled basename
    owner = {}     # bundled basename -> real source path
    changes = {}   # in-bundle file -> [(old install name, bundled basename)]
    exe_rpaths = MachO(exe).rpaths

    while queue:
        f = queue.pop(0)
        src = source[f]
        m = MachO(src)
        changes[f] = []
        for dep in m.deps:
            if is_system(dep):
                continue
            found = resolve(dep, src, m.rpaths + exe_rpaths, exe)
            if not found:
                die("%s: cannot resolve %s" % (src, dep))
            real = os.path.realpath(found)
            if real.startswith(contents + "/"):
                continue  # already inside the bundle
            if real not in bundled:
                lib = MachO(real)
                name = os.path.basename(lib.id or dep)
                if name in owner and owner[name] != real:
                    die("two different libraries named %s: %s, %s" % (name, owner[name], real))
                bundled[real], owner[name] = name, real
                dest = os.path.join(fwdir, name)
                shutil.copy2(real, dest)
                os.chmod(dest, 0o644)
                source[dest] = real
                queue.append(dest)
                if name in PLUGIN_DIRS:
                    rel, dst = PLUGIN_DIRS[name]
                    pdir = os.path.join(os.path.dirname(real), rel)
                    pdest = os.path.join(plugdir, dst)
                    os.makedirs(pdest, exist_ok=True)
                    plugins = sorted(p for p in glob.glob(os.path.join(pdir, "*")) if is_macho(p))
                    if not plugins:
                        die("%s: no plugins in %s" % (name, pdir))
                    for p in plugins:
                        d = os.path.join(pdest, os.path.basename(p))
                        shutil.copy2(os.path.realpath(p), d)
                        os.chmod(d, 0o644)
                        source[d] = os.path.realpath(p)
                        queue.append(d)
            changes[f].append((dep, bundled[real]))

    for f, deps in changes.items():
        m = MachO(f)
        args = []
        if f.startswith(fwdir + "/"):
            args += ["-id", "@rpath/" + os.path.basename(f)]
            want = "@loader_path"
        elif f == exe:
            want = "@executable_path/../Frameworks"
        else:
            want = "@loader_path/" + os.path.relpath(fwdir, os.path.dirname(f))
        for old, name in deps:
            args += ["-change", old, "@rpath/" + name]
        for rp in m.rpaths:
            if rp != want:
                args += ["-delete_rpath", rp]
        if want not in m.rpaths:
            args += ["-add_rpath", want]
        if f != exe:
            subprocess.run(["codesign", "--remove-signature", f], check=True)
        if args:
            run("install_name_tool", *args, f)

    # licenses of the embedded Homebrew packages
    licdir = os.path.join(contents, "Resources", "ThirdPartyLicenses")
    shutil.rmtree(licdir, ignore_errors=True)
    os.makedirs(licdir)
    rows = []
    kegs = {}
    for real, name in sorted(bundled.items(), key=lambda kv: kv[1]):
        pkg, ver, keg = keg_of(real)
        if not pkg:
            die("%s is not from a Homebrew keg; add its license by hand" % real)
        kegs[pkg] = (ver, keg)
        rows.append((name, "Frameworks", pkg, ver))
    for d in sorted(source):
        if d.startswith(plugdir + "/"):
            pkg, ver, keg = keg_of(source[d])
            rows.append((os.path.relpath(d, plugdir), "PlugIns", pkg, ver))
    lics = []  # (package, version, license file, where it came from)
    for pkg, (ver, keg) in sorted(kegs.items()):
        # Homebrew puts a package's license files in the keg root, a few in share/doc/<name>/
        files = [os.path.join(d, p) for d in [keg] + sorted(glob.glob(os.path.join(keg, "share/doc/*")))
                 if os.path.isdir(d) for p in sorted(os.listdir(d))
                 if LICENSE_RE.search(p) and os.path.isfile(os.path.join(d, p))]
        if not files:
            die("%s %s: no license file in %s" % (pkg, ver, keg))
        os.makedirs(os.path.join(licdir, pkg))
        for p in files:
            d = os.path.join(licdir, pkg, os.path.basename(p))
            if os.path.exists(d):  # same name in the keg root and share/doc
                if not filecmp.cmp(p, d, shallow=False):
                    die("%s %s: two different %s" % (pkg, ver, os.path.basename(p)))
                continue
            shutil.copy2(p, d)
            os.chmod(d, 0o644)
            lics.append((pkg, ver, os.path.basename(p),
                         "Homebrew keg " + os.path.relpath(p, os.path.dirname(os.path.dirname(keg)))))
        # license files the keg lacks, taken from the package's source archive (see SOURCE)
        extra = os.path.join(EXTRA_LICENSES, pkg, ver)
        if os.path.isdir(extra):
            with open(os.path.join(extra, "SOURCE")) as f:
                origin = f.readline().strip().rstrip(":")
            for n in sorted(os.listdir(extra)):
                if n != "SOURCE":
                    d = os.path.join(licdir, pkg, n)
                    shutil.copyfile(os.path.join(extra, n), d)
                    os.chmod(d, 0o644)
                    lics.append((pkg, ver, n, origin))
        for n in REQUIRED_LICENSES.get(pkg, []):
            if not os.path.isfile(os.path.join(licdir, pkg, n)):
                die("%s %s: %s missing; add it from the %s source to %s"
                    % (pkg, ver, n, ver, os.path.relpath(extra, os.path.dirname(EXTRA_LICENSES))))
    with open(os.path.join(licdir, "MANIFEST.tsv"), "w") as f:
        f.write("file\tlocation\thomebrew package\tversion\n")
        for r in rows:
            f.write("\t".join(r) + "\n")
    # package.sh appends the code compiled into FreeWRL
    with open(os.path.join(licdir, "LICENSES.tsv"), "w") as f:
        f.write("package\tversion\tlicense file\tsource\n")
        for r in lics:
            f.write("\t".join(r) + "\n")

    # the bundle can't run on a macOS older than its newest-targeted binary
    machos = [p for p in (os.path.join(r, n) for r, _, ns in os.walk(contents) for n in ns) if is_macho(p)]
    need = max((MachO(p).minos for p in machos), key=version_tuple)
    info["LSMinimumSystemVersion"] = need
    with open(os.path.join(contents, "Info.plist"), "wb") as f:
        plistlib.dump(info, f)

    print("embedded %d libraries, %d plugins from %d packages; LSMinimumSystemVersion %s"
          % (len(bundled), sum(1 for r in rows if r[1] == "PlugIns"), len(kegs), need))


if __name__ == "__main__":
    main()
