#!/bin/sh
# Build a self-contained FreeWRL.app (Apple Silicon) that runs without Homebrew.
#
# usage: package.sh [-t macos] [-D deps-prefix] [-a FreeWRL.app] [-o outdir] [-s identity] [-r]
#                   [-e entitlements.plist] [-z] [-n]
#   -t  oldest macOS the package runs on        (default: 13.0)
#   -D  libraries built by tools/macos-deps/build.sh for that macOS
#                                               (default: build them into <outdir>/deps)
#   -a  package this Release build instead of building one
#   -o  output directory                        (default: ./macos-package-out)
#   -s  code-signing identity                   (default: - , ad-hoc)
#   -r  sign with the hardened runtime
#   -e  entitlements plist for the app          (default: none)
#   -z  also make a zip of the app (ditto), with its SHA-256
#   -n, --notarize  notarize with notarytool, staple, check with Gatekeeper, then zip
#       (implies -z; needs a Developer ID identity with -s and -r; credentials below)
#
# Steps: build FreeType, ODE and freealut for that macOS (tools/macos-deps) -> build the app
# against them (xcodebuild, Release, arm64) -> copy the app -> embed its non-system dylibs,
# rewrite install names to @rpath (bundle.py) -> copy license files -> sign inside out ->
# check the result (verify.py --macos, codesign) [-> notarize, staple, stapler validate,
# spctl] [-> zip]. Homebrew is not used: its bottles only run on the macOS they were built for.
#
# Notarization credentials, from the environment (never printed):
#   NOTARY_KEYCHAIN_PROFILE  a profile saved with `xcrun notarytool store-credentials`, or
#   NOTARY_KEY_ID, NOTARY_ISSUER  App Store Connect API key ID and issuer ID
#                            (APPLE_API_KEY_ID, APPLE_API_ISSUER are accepted too), and
#   NOTARY_KEY               the key's .p8 file
#                            (default: ~/.appstoreconnect/private_keys/AuthKey_<key ID>.p8)
#   NOTARY_ENV_FILE          a shell file setting any of the above, read first
set -eu
H=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$H/../.." && pwd)
APP_IN= OUT=macos-package-out IDENTITY=- RUNTIME= ENTITLEMENTS= ZIP= NOTARIZE= TARGET=13.0 DEPS=
for arg; do
	shift
	case $arg in --notarize) set -- "$@" -n ;; *) set -- "$@" "$arg" ;; esac
done
while getopts "t:D:a:o:s:re:zn" opt; do
	case $opt in
	t) TARGET=$OPTARG ;;
	D) DEPS=$(cd "$OPTARG" && pwd) ;;
	a) APP_IN=$OPTARG ;;
	o) OUT=$OPTARG ;;
	s) IDENTITY=$OPTARG ;;
	r) RUNTIME=1 ;;
	e) ENTITLEMENTS=$(cd "$(dirname "$OPTARG")" && pwd)/$(basename "$OPTARG") ;;
	z) ZIP=1 ;;
	n) NOTARIZE=1 ZIP=1 ;;
	*) sed -n '2,27p' "$0"; exit 2 ;;
	esac
done

if [ -n "$NOTARIZE" ]; then
	# check everything notarization needs before spending time on a build
	[ "$IDENTITY" != - ] && [ -n "$RUNTIME" ] ||
		{ echo "--notarize needs a Developer ID identity (-s) and the hardened runtime (-r)" >&2; exit 2; }
	if [ -n "${NOTARY_ENV_FILE:-}" ]; then
		[ -r "$NOTARY_ENV_FILE" ] || { echo "NOTARY_ENV_FILE not readable: $NOTARY_ENV_FILE" >&2; exit 2; }
		set -a
		. "$NOTARY_ENV_FILE"
		set +a
	fi
	if [ -n "${NOTARY_KEYCHAIN_PROFILE:-}" ]; then
		set -- --keychain-profile "$NOTARY_KEYCHAIN_PROFILE"
	else
		NOTARY_KEY_ID=${NOTARY_KEY_ID:-${APPLE_API_KEY_ID:-}}
		NOTARY_ISSUER=${NOTARY_ISSUER:-${APPLE_API_ISSUER:-}}
		[ -n "$NOTARY_KEY_ID" ] && [ -n "$NOTARY_ISSUER" ] ||
			{ echo "--notarize: set NOTARY_KEYCHAIN_PROFILE, or NOTARY_KEY_ID and NOTARY_ISSUER (see $0 -h)" >&2; exit 2; }
		NOTARY_KEY=${NOTARY_KEY:-$HOME/.appstoreconnect/private_keys/AuthKey_$NOTARY_KEY_ID.p8}
		[ -r "$NOTARY_KEY" ] || { echo "--notarize: API key file not readable (NOTARY_KEY)" >&2; exit 2; }
		set -- --key "$NOTARY_KEY" --key-id "$NOTARY_KEY_ID" --issuer "$NOTARY_ISSUER"
	fi
	# "$@" now holds the notarytool credential arguments
fi

mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)

if [ -z "$APP_IN" ]; then
	if [ -z "$DEPS" ]; then
		echo "== libraries for macOS $TARGET (tools/macos-deps)"
		"$REPO/tools/macos-deps/build.sh" -t "$TARGET" -p "$OUT/deps" -c "$OUT/deps-sources"
		DEPS=$OUT/deps
	fi
	echo "== build (Release, arm64, macOS $TARGET)"
	xcodebuild -project "$REPO/OSX_gui/FreeWRL-Desktop/FreeWRL.xcodeproj" -scheme FreeWRL \
		-configuration Release ARCHS=arm64 CODE_SIGN_IDENTITY=- \
		FW_DEPS="$DEPS" MACOSX_DEPLOYMENT_TARGET="$TARGET" \
		-derivedDataPath "$OUT/DerivedData" build > "$OUT/build.log" 2>&1 ||
		{ tail -20 "$OUT/build.log"; echo "build failed, see $OUT/build.log" >&2; exit 1; }
	APP_IN=$OUT/DerivedData/Build/Products/Release/FreeWRL.app
fi
[ -x "$APP_IN/Contents/MacOS/FreeWRL" ] || { echo "not a FreeWRL.app: $APP_IN" >&2; exit 1; }

APP=$OUT/FreeWRL.app
Z=$OUT/FreeWRL-macos-arm64.zip
rm -f "$Z" "$Z.sha256"
echo "== copy $APP_IN"
rm -rf "$APP"
ditto "$APP_IN" "$APP"
rm -rf "$APP/Contents/_CodeSignature"

echo "== embed libraries"
python3 "$H/bundle.py" "$APP"

echo "== licenses of code compiled into FreeWRL"
L=$APP/Contents/Resources/ThirdPartyLicenses
mkdir -p "$L/FreeWRL" "$L/duktape" "$L/libtess" "$L/stb_image"
cp "$REPO/freex3d/COPYING" "$REPO/freex3d/COPYING.LESSER" "$L/FreeWRL/"
# verbatim license comments from the vendored sources
extract() { # file first-line-pattern last-line-pattern out
	sed -n "/$2/,/$3/p" "$1" > "$4"
	[ -s "$4" ] || { echo "license text not found in $1" >&2; exit 1; }
}
extract "$REPO/freex3d/src/lib/world_script/duktape/duktape.c" '^\/\* LICENSE.txt \*\/' '^\*\/' "$L/duktape/LICENSE.txt"
extract "$REPO/freex3d/src/libtess/tess.c" 'SGI FREE SOFTWARE LICENSE B' '^ \*\/' "$L/libtess/LICENSE"
extract "$REPO/freex3d/src/lib/opengl/stb_image.h" '^This software is available under 2 licenses' '^\*\/' "$L/stb_image/LICENSE"
chmod 644 "$L"/*/*
REV=$(git -C "$REPO" rev-parse --short=9 HEAD 2>/dev/null || echo unknown)
git -C "$REPO" diff --quiet HEAD 2>/dev/null || REV="$REV (modified)"
DUK=$(sed -n 's/^#define DUK_VERSION  *\([0-9]*\)L$/\1/p' "$REPO/freex3d/src/lib/world_script/duktape/duktape.h")
DUK=$((DUK / 10000)).$((DUK / 100 % 100)).$((DUK % 100))
STB=$(sed -n '1s/.*stb_image - v\([0-9.]*\) .*/\1/p' "$REPO/freex3d/src/lib/opengl/stb_image.h")
printf '%s\n' \
	"FreeWRL	source $REV	COPYING	freex3d/COPYING" \
	"FreeWRL	source $REV	COPYING.LESSER	freex3d/COPYING.LESSER" \
	"duktape	$DUK	LICENSE.txt	license comment in freex3d/src/lib/world_script/duktape/duktape.c" \
	"libtess	vendored	LICENSE	license comment in freex3d/src/libtess/tess.c" \
	"stb_image	$STB	LICENSE	license comment in freex3d/src/lib/opengl/stb_image.h" >> "$L/LICENSES.tsv"

echo "== sign ($IDENTITY${RUNTIME:+, hardened runtime})"
# (a function, so "$@" keeps the notarytool credentials)
sign() {
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
}
sign

echo "== verify"
python3 "$H/verify.py" --source-root "$REPO" --macos "$TARGET" "$APP"
codesign --verify --deep --strict --verbose=2 "$APP"

if [ -n "$NOTARIZE" ]; then
	codesign -dvv "$APP" 2>&1 | grep -q '^Authority=Developer ID Application:' ||
		{ echo "--notarize: the app is not signed with a Developer ID Application identity" >&2; exit 1; }
	echo "== notarize"
	SUB=$OUT/FreeWRL-notary-submission.zip
	rm -f "$SUB" "$OUT/notary-submit.json" "$OUT/notary-log.json"
	ditto -c -k --keepParent "$APP" "$SUB"
	# submit and wait; a failed wait (timeout, network) stops here too
	xcrun notarytool submit "$SUB" "$@" --wait --output-format json > "$OUT/notary-submit.json" ||
		{ cat "$OUT/notary-submit.json" >&2; echo "notarytool submit failed" >&2; exit 1; }
	field() { python3 -c 'import json,sys; print(json.load(open(sys.argv[1])).get(sys.argv[2], ""))' "$OUT/notary-submit.json" "$1"; }
	ID=$(field id) STATUS=$(field status)
	echo "submission $ID: $STATUS"
	[ -n "$ID" ] && xcrun notarytool log "$ID" "$@" "$OUT/notary-log.json" > /dev/null ||
		echo "warning: could not fetch the notarization log" >&2
	[ "$STATUS" = Accepted ] ||
		{ echo "notarization $STATUS; see $OUT/notary-log.json" >&2; exit 1; }
	rm -f "$SUB"
	echo "== staple"
	xcrun stapler staple "$APP"
	xcrun stapler validate "$APP"
	echo "== Gatekeeper"
	spctl --assess --type execute --verbose=4 "$APP" 2>&1 | tee "$OUT/spctl.txt"
	grep -q 'source=Notarized Developer ID' "$OUT/spctl.txt" ||
		{ echo "spctl did not report a notarized Developer ID app" >&2; exit 1; }
fi

if [ -n "$ZIP" ]; then
	ditto -c -k --keepParent "$APP" "$Z"
	(cd "$OUT" && shasum -a 256 "$(basename "$Z")") | tee "$Z.sha256"
fi
echo "packaged: $APP"
