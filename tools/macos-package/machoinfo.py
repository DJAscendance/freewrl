"""Mach-O load-command helpers for the macOS packaging tools (uses otool/file)."""
import os
import subprocess

SYSTEM_PREFIXES = ("/usr/lib/", "/System/Library/")
DYLIB_CMDS = ("LC_LOAD_DYLIB", "LC_LOAD_WEAK_DYLIB", "LC_REEXPORT_DYLIB",
              "LC_LAZY_LOAD_DYLIB", "LC_LOAD_UPWARD_DYLIB")


def run(*args):
    return subprocess.run(args, check=True, capture_output=True, text=True).stdout


def is_macho(path):
    if os.path.islink(path) or not os.path.isfile(path):
        return False
    with open(path, "rb") as f:
        magic = f.read(4)
    return magic in (b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe",  # 64/32-bit LE
                     b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca")  # fat


def is_system(name):
    return name.startswith(SYSTEM_PREFIXES)


class MachO:
    """Load commands of one thin or fat Mach-O file (first architecture listed)."""

    def __init__(self, path):
        self.path = path
        self.id = None
        self.deps = []      # install names, in load order
        self.rpaths = []
        self.platform = None
        self.minos = None
        self.sdk = None
        self.archs = run("lipo", "-archs", path).split()
        cmd = None
        for line in run("otool", "-l", path).splitlines():
            f = line.split()
            if not f:
                continue
            if f[0] == "cmd":
                cmd = f[1]
            elif f[0] == "name" and cmd in DYLIB_CMDS:
                self.deps.append(f[1])
            elif f[0] == "name" and cmd == "LC_ID_DYLIB":
                self.id = f[1]
            elif f[0] == "path" and cmd == "LC_RPATH":
                self.rpaths.append(f[1])
            elif cmd == "LC_BUILD_VERSION" and f[0] in ("platform", "minos", "sdk"):
                setattr(self, f[0], f[1])
            elif cmd == "LC_VERSION_MIN_MACOSX" and f[0] in ("version", "sdk"):
                self.platform = "MACOS"
                setattr(self, "minos" if f[0] == "version" else "sdk", f[1])
            if cmd == "LC_BUILD_VERSION" and f[0] == "ntools":
                cmd = None  # stop at the first architecture's build version
        # otool -l prints every architecture of a fat file; keep unique values
        self.deps = list(dict.fromkeys(self.deps))
        self.rpaths = list(dict.fromkeys(self.rpaths))


def expand(name, loader, executable):
    """Replace @loader_path / @executable_path; leave @rpath alone."""
    if name.startswith("@loader_path/"):
        return os.path.normpath(os.path.join(os.path.dirname(loader), name[len("@loader_path/"):]))
    if name.startswith("@executable_path/"):
        return os.path.normpath(os.path.join(os.path.dirname(executable), name[len("@executable_path/"):]))
    return name


def resolve(name, loader, rpaths, executable):
    """Where dyld would find install name `name` loaded by `loader`.

    rpaths: the LC_RPATH list that applies (the loader's, then the executable's).
    Returns an existing path or None. System libraries live in the dyld shared
    cache, so they are returned unresolved."""
    if is_system(name):
        return name
    if name.startswith("@rpath/"):
        rest = name[len("@rpath/"):]
        for rp in rpaths:
            cand = os.path.join(expand(rp, loader, executable), rest)
            if os.path.exists(cand):
                return os.path.normpath(cand)
        return None
    cand = expand(name, loader, executable)
    return cand if os.path.exists(cand) else None


def version_tuple(v):
    return tuple(int(x) for x in v.split(".")) if v else (0,)
