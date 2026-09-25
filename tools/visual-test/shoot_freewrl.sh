#!/bin/sh
# usage: shoot_freewrl.sh <world file or url> <out.png> [seconds]
# Launches FreeWRL.app in the background (no focus steal), screenshots its
# window, crops to the 3D view, and quits it. Log goes to <out>.log,
# the uncropped window to <out>.window.png.
#   FREEWRL_APP  app bundle (default ~/Applications/FreeWRL.app)
#   CROP_TOP / CROP_BOTTOM  window chrome to remove, in pixels (Retina defaults)
set -eu
H=$(cd "$(dirname "$0")" && pwd)
APP=${FREEWRL_APP:-$HOME/Applications/FreeWRL.app}
OUT=$2
LOG="${OUT%.png}.log"
RAW="${OUT%.png}.window.png"

[ -x "$H/.bin/winid" ] || { mkdir -p "$H/.bin"; swiftc -O "$H/winid.swift" -o "$H/.bin/winid"; }

pkill -f "FreeWRL.app/Contents/MacOS/FreeWRL" 2>/dev/null || true
sleep 1
: > "$LOG"
open -g -n --stdout "$LOG" --stderr "$LOG" -a "$APP" --args "$1"
sleep "${3:-12}"
WID=$("$H/.bin/winid" FreeWRL || true)
if [ -z "$WID" ]; then
	echo "no FreeWRL window (crashed? see $LOG and ~/Library/Logs/DiagnosticReports)" >&2
	exit 1
fi
screencapture -x -o -l "$WID" "$RAW"
pkill -f "FreeWRL.app/Contents/MacOS/FreeWRL" 2>/dev/null || true
# title bar + URL bar on top, HUD status bar at the bottom
magick "$RAW" -strip -background black -alpha remove -alpha off -gravity North -chop "0x${CROP_TOP:-116}" -gravity South -chop "0x${CROP_BOTTOM:-32}" +repage "PNG24:$OUT"
echo "$OUT"
