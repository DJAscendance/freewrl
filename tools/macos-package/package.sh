#!/bin/sh
# Build a self-contained FreeWRL.app (Apple Silicon) that runs without Homebrew.
#
# usage: package.sh [-a FreeWRL.app] [-o outdir] [-s identity] [-r] [-e entitlements.plist] [-z]
#   -a  package this Release build instead of building one
#   -o  output directory                        (default: ./macos-package-out)
#   -s  code-signing identity                   (default: - , ad-hoc)
#   -r  sign with the hardened runtime
#   -e  entitlements plist for the app          (default: none)
#   -z  also make a zip of the app (ditto), with its SHA-256
#
# Steps: build (xcodebuild, Release, arm64) -> copy the app -> embed its non-system
# dylibs and Imlib2's loaders, rewrite install names to @rpath (bundle.py) -> copy
# license files -> sign inside out -> check the result (verify.py, codesign).
# Needs the Homebrew libraries the Xcode project links (see MACOS-STATUS.md) at
# packaging time only; the packaged app does not use them.
set -eu
H=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$H/../.." && pwd)
APP_IN= OUT=macos-package-out IDENTITY=- RUNTIME= ENTITLEMENTS= ZIP=
while getopts "a:o:s:re:z" opt; do
	case $opt in
	a) APP_IN=$OPTARG ;;
	o) OUT=$OPTARG ;;
	s) IDENTITY=$OPTARG ;;
	r) RUNTIME=1 ;;
	e) ENTITLEMENTS=$(cd "$(dirname "$OPTARG")" && pwd)/$(basename "$OPTARG") ;;
	z) ZIP=1 ;;
	*) sed -n '2,17p' "$0"; exit 2 ;;
	esac
done
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)

if [ -z "$APP_IN" ]; then
	echo "== build (Release, arm64)"
	xcodebuild -project "$REPO/OSX_gui/FreeWRL-Desktop/FreeWRL.xcodeproj" -scheme FreeWRL \
		-configuration Release ARCHS=arm64 CODE_SIGN_IDENTITY=- \
		-derivedDataPath "$OUT/DerivedData" build > "$OUT/build.log" 2>&1 ||
		{ tail -20 "$OUT/build.log"; echo "build failed, see $OUT/build.log" >&2; exit 1; }
	APP_IN=$OUT/DerivedData/Build/Products/Release/FreeWRL.app
fi
[ -x "$APP_IN/Contents/MacOS/FreeWRL" ] || { echo "not a FreeWRL.app: $APP_IN" >&2; exit 1; }

APP=$OUT/FreeWRL.app
echo "== copy $APP_IN"
rm -rf "$APP"
ditto "$APP_IN" "$APP"
rm -rf "$APP/Contents/_CodeSignature"

echo "== embed libraries"
python3 "$H/bundle.py" "$APP"

echo "== licenses of code compiled into FreeWRL"
L=$APP/Contents/Resources/ThirdPartyLicenses
mkdir -p "$L/FreeWRL" "$L/duktape" "$L/libtess"
cp "$REPO/freex3d/COPYING" "$REPO/freex3d/COPYING.LESSER" "$L/FreeWRL/"
# verbatim license comments from the vendored sources
extract() { # file first-line-pattern last-line-pattern out
	sed -n "/$2/,/$3/p" "$1" > "$4"
	[ -s "$4" ] || { echo "license text not found in $1" >&2; exit 1; }
}
extract "$REPO/freex3d/src/lib/world_script/duktape/duktape.c" '^\/\* LICENSE.txt \*\/' '^\*\/' "$L/duktape/LICENSE.txt"
extract "$REPO/freex3d/src/libtess/tess.c" 'SGI FREE SOFTWARE LICENSE B' '^ \*\/' "$L/libtess/LICENSE"
chmod 644 "$L"/*/*
REV=$(git -C "$REPO" rev-parse --short=9 HEAD 2>/dev/null || echo unknown)
git -C "$REPO" diff --quiet HEAD 2>/dev/null || REV="$REV (modified)"
DUK=$(sed -n 's/^#define DUK_VERSION  *\([0-9]*\)L$/\1/p' "$REPO/freex3d/src/lib/world_script/duktape/duktape.h")
DUK=$((DUK / 10000)).$((DUK / 100 % 100)).$((DUK % 100))
printf '%s\n' \
	"FreeWRL	source $REV	COPYING	freex3d/COPYING" \
	"FreeWRL	source $REV	COPYING.LESSER	freex3d/COPYING.LESSER" \
	"duktape	$DUK	LICENSE.txt	license comment in freex3d/src/lib/world_script/duktape/duktape.c" \
	"libtess	vendored	LICENSE	license comment in freex3d/src/libtess/tess.c" >> "$L/LICENSES.tsv"

echo "== sign ($IDENTITY${RUNTIME:+, hardened runtime})"
set -- --force --sign "$IDENTITY"
[ -n "$RUNTIME" ] && set -- "$@" --options runtime
[ "$IDENTITY" != - ] && set -- "$@" --timestamp
# nested code first (dylibs, plugins), then the app, which seals everything else
find "$APP/Contents/Frameworks" "$APP/Contents/PlugIns" -type f \( -name '*.dylib' -o -name '*.so' \) |
	sort | while read -r f; do codesign "$@" "$f" || exit 1; done
if [ -n "$ENTITLEMENTS" ]; then
	codesign "$@" --entitlements "$ENTITLEMENTS" "$APP"
else
	codesign "$@" "$APP"
fi

echo "== verify"
python3 "$H/verify.py" --source-root "$REPO" "$APP"
codesign --verify --deep --strict --verbose=2 "$APP"

if [ -n "$ZIP" ]; then
	Z=$OUT/FreeWRL-macos-arm64.zip
	rm -f "$Z"
	ditto -c -k --keepParent "$APP" "$Z"
	shasum -a 256 "$Z" | tee "$Z.sha256"
fi
echo "packaged: $APP"
