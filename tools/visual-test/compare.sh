#!/bin/sh
# Render worlds in FreeWRL and in X_ITE (reference) and compare the images.
#
# usage: compare.sh [-r root] [-p port] [-o outdir] [-t threshold] world.wrl [world2.x3d ...]
#   world paths are relative to root (served over http, so /absolute URLs in
#   the world resolve against root, e.g. Cybertown's /externprotos/...)
#   -r  directory to serve                 (default: <repo>/freewrl/tests)
#   -p  port                                (default: 8740)
#   -o  output directory                    (default: ./visual-test-out)
#   -t  minimum match score to pass, 0..1   (default: 0.90)
# match = normalized cross-correlation of the two renders (1 = identical).
# Unlike SSIM it isn't inflated by large empty backgrounds; SSIM is reported too.
# Animated worlds are captured at different moments and won't match.
# Writes per world: freewrl.png, xite.png, diff.png, side.png, freewrl.log,
# xite.log; plus summary.tsv and index.html. Exit status 1 if any world fails.
set -eu
H=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$H/../../freewrl/tests" && pwd)
PORT=8740
OUT=visual-test-out
THRESH=0.90
while getopts "r:p:o:t:" opt; do
	case $opt in
	r) ROOT=$(cd "$OPTARG" && pwd) ;;
	p) PORT=$OPTARG ;;
	o) OUT=$OPTARG ;;
	t) THRESH=$OPTARG ;;
	*) sed -n '2,15p' "$0"; exit 2 ;;
	esac
done
shift $((OPTIND - 1))
[ $# -gt 0 ] || { sed -n '2,15p' "$0"; exit 2; }
command -v magick >/dev/null || { echo "needs ImageMagick: brew install imagemagick" >&2; exit 2; }
FONT=${FONT:-/System/Library/Fonts/Supplemental/Arial.ttf}

BASE="http://127.0.0.1:$PORT"
python3 "$H/serve.py" "$ROOT" "$PORT" > /dev/null 2>&1 &
SERVER=$!
trap 'kill $SERVER 2>/dev/null' EXIT INT TERM
sleep 1

mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
printf 'world\tmatch\tssim\tresult\n' > "$OUT/summary.tsv"
FAILED=0

label() { # image title -> labelled half-size image on stdout
	magick "$1" -resize 50% -gravity North -background '#222' -splice 0x28 \
		-font "$FONT" -fill white -pointsize 20 -annotate +0+3 "$2" miff:-
}

for WORLD in "$@"; do
	WORLD=/${WORLD#/}
	D="$OUT/$(echo "${WORLD#/}" | tr '/' '_')"
	mkdir -p "$D"
	echo "== $WORLD"
	if ! "$H/shoot_freewrl.sh" "$BASE$WORLD" "$D/freewrl.png" >/dev/null 2>&1; then
		printf '%s\t-\t-\tCRASH\n' "$WORLD" >> "$OUT/summary.tsv"
		echo "   FreeWRL: no window (crash?)"; FAILED=1; continue
	fi
	SIZE=$(magick identify -format '%w %h' "$D/freewrl.png")
	if ! "$H/shoot_xite.sh" "$BASE" "$WORLD" "$D/xite.png" $SIZE >/dev/null; then
		printf '%s\t-\t-\tNOREF\n' "$WORLD" >> "$OUT/summary.tsv"
		echo "   X_ITE: no reference image (see $D/xite.log)"; continue
	fi
	# X_ITE output can be off by a pixel from rounding; force identical geometry
	magick "$D/xite.png" -resize "$(echo $SIZE | tr ' ' x)!" "PNG24:$D/xite.png"

	# at half size: less sensitive to antialiasing/texture filtering differences
	A="$D/.a.png" B="$D/.b.png"
	magick "$D/freewrl.png" -resize 50% "$A"
	magick "$D/xite.png" -resize 50% "$B"
	# ImageMagick 7.1.2 reports NCC/SSIM as distortions (0 = identical); convert to similarity
	NCC=$(magick compare -metric NCC "$A" "$B" null: 2>&1 | sed -E 's/.*\(([0-9.e-]+)\).*/\1/')
	MATCH=$(awk "BEGIN{printf \"%.4f\", 1 - $NCC}")
	DSSIM=$(magick compare -metric SSIM "$A" "$B" null: 2>&1 | sed -E 's/.*\(([0-9.e-]+)\).*/\1/')
	SSIM=$(awk "BEGIN{printf \"%.4f\", 1 - $DSSIM}")
	rm -f "$A" "$B"

	magick "$D/freewrl.png" "$D/xite.png" -compose difference -composite -evaluate multiply 2 "$D/diff.png"
	{ label "$D/freewrl.png" FreeWRL; label "$D/xite.png" 'X_ITE (reference)'; label "$D/diff.png" difference; } |
		magick miff:- +append "$D/side.png"

	if awk "BEGIN{exit !($MATCH >= $THRESH)}"; then R=PASS; else R=FAIL; FAILED=1; fi
	printf '%s\t%s\t%s\t%s\n' "$WORLD" "$MATCH" "$SSIM" "$R" >> "$OUT/summary.tsv"
	echo "   match $MATCH  (ssim $SSIM)  $R"
	N=$(grep -ciE "error|fail|expected|unrecognized|not found" "$D/freewrl.log" || true)
	[ "$N" = 0 ] || echo "   FreeWRL log: $N error-looking lines"
	N=$(grep -ciE "error|warn|fail" "$D/xite.log" 2>/dev/null || true)
	[ "${N:-0}" = 0 ] || echo "   X_ITE log: $N error/warning lines"
done

{
	echo '<!doctype html><meta charset=utf-8><title>FreeWRL vs X_ITE</title>'
	echo '<style>body{font:14px system-ui;background:#111;color:#ddd;margin:16px}a{color:#8af}img{max-width:100%}.FAIL,.CRASH,.NOREF{color:#f66}.PASS{color:#6c6}</style>'
	echo "<h1>FreeWRL vs X_ITE</h1><p>pass: match (normalized cross-correlation) ≥ $THRESH</p>"
	tail -n +2 "$OUT/summary.tsv" | while IFS="$(printf '\t')" read -r W M S RES; do
		DIR=$(echo "${W#/}" | tr '/' '_')
		echo "<h2>$W <span class=$RES>$RES</span></h2><p>match $M · ssim $S · logs: <a href=\"$DIR/freewrl.log\">FreeWRL</a> <a href=\"$DIR/xite.log\">X_ITE</a></p>"
		[ "$RES" = CRASH ] || [ "$RES" = NOREF ] || echo "<img src=\"$DIR/side.png\">"
	done
} > "$OUT/index.html"
echo "report: $OUT/index.html"
exit $FAILED
