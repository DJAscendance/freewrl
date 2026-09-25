#!/bin/sh
# usage: shoot_xite.sh <server base url> <world path> <out.png> <width px> <height px> [hold ms]
# Renders the world with X_ITE in headless Chrome at the given pixel size
# (rendered at 2x device scale, so CSS size is half). Needs serve.py running.
# Browser console goes to <out>.log.
#   CHROME  browser binary (default Google Chrome)
#   XITE    X_ITE dist dir URL (default: pinned jsdelivr build in viewer.html)
set -eu
BASE=$1 WORLD=$2 OUT=$3 W=$4 HT=$5 HOLD=${6:-15000}
CHROME=${CHROME:-"/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"}
CW=$((W / 2)) CH=$((HT / 2))
# own throwaway profile: sharing the default one with a running Chrome blocks/hands off
PROFILE=$(mktemp -d -t freewrl-vt-chrome)
RAWLOG="$PROFILE.log"
PID=
cleanup() { [ -n "$PID" ] && kill $PID 2>/dev/null; rm -rf "$PROFILE" "$RAWLOG"; }
# sh skips the EXIT trap when killed by a signal: clean up (and stop Chrome) on those too
trap cleanup EXIT
trap 'cleanup; exit 130' INT TERM
URL="$BASE/__vt/viewer.html?world=$WORLD&w=$CW&h=$CH&hold=$HOLD${XITE:+&xite=$XITE}"
rm -f "$OUT"
"$CHROME" --headless=new --use-angle=swiftshader --enable-unsafe-swiftshader \
	--hide-scrollbars --force-device-scale-factor=2 --window-size="$CW,$CH" \
	--user-data-dir="$PROFILE" --no-first-run --no-default-browser-check \
	--enable-logging=stderr --v=0 \
	--screenshot="$OUT" "$URL" > "$RAWLOG" 2>&1 &
PID=$!
# Current headless Chrome writes the screenshot but doesn't exit while X_ITE keeps
# rendering, so stop it once the file is written; give up after hold + 30s.
LIMIT=$((HOLD / 1000 + 30))
i=0 LAST=-1
while kill -0 $PID 2>/dev/null; do
	i=$((i + 1))
	if [ -s "$OUT" ]; then
		SZ=$(wc -c < "$OUT")
		[ "$SZ" = "$LAST" ] && { kill $PID 2>/dev/null || true; break; }
		LAST=$SZ
	fi
	if [ $i -gt $LIMIT ]; then kill $PID 2>/dev/null || true; echo "X_ITE: Chrome timed out after ${LIMIT}s" >&2; break; fi
	sleep 1
done
wait $PID 2>/dev/null || true
grep CONSOLE "$RAWLOG" | grep -vE "parser-blocking|GPU stall" |
	sed -E 's/^.*CONSOLE[:(][0-9]+\)?\] //; s/, source: .*$//' > "${OUT%.png}.log" || true
[ -s "$OUT" ] || { echo "X_ITE: no screenshot" >&2; exit 1; }
magick "$OUT" -strip -background black -alpha remove -alpha off "PNG24:$OUT"
echo "$OUT"
