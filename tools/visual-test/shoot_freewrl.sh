#!/bin/sh
# usage: shoot_freewrl.sh <world file or url> <out.png> [seconds]
# Launches FreeWRL.app in the background (no focus steal), screenshots its
# window, crops to the 3D view, and quits it. Log goes to <out>.log,
# the uncropped window to <out>.window.png.
#   FREEWRL_APP  app bundle (default ~/Applications/FreeWRL.app)
#   CROP_TOP     title + URL bar to remove, in pixels (Retina default)
#   CROP_BOTTOM  HUD bar to remove, in pixels; default: measured from the capture
#                (FreeWRL 6.x's HUD wraps to one or more rows depending on window width)
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
# HUD height: the HUD background is (61,69,87); along the right edge of the window, find the
# lowest run of that colour and measure from its top to the bottom of the capture
hud_height() {
	magick "$1" -quiet -alpha off -depth 8 -crop "1x%[h]+%[fx:w-3]+0" +repage txt:- 2>/dev/null |
	sed -nE 's/^0,([0-9]+): \(([0-9]+),([0-9]+),([0-9]+).*/\1 \2 \3 \4/p' |
	awk 'function near(a,b){return a-b<=4 && b-a<=4}
		{ h=$1; hud[$1]=near($2,61) && near($3,69) && near($4,87) }
		END{ for(y=h;y>=0 && !hud[y];y--); for(top=y;top>0 && hud[top-1];top--);
			print (y<0 ? 0 : h+1-top) }'
}
BOTTOM=${CROP_BOTTOM:-$(hud_height "$RAW")}
[ "${BOTTOM:-0}" -gt 0 ] || BOTTOM=32 # HUD not found: master-era one-row status bar
# title bar + URL bar on top, HUD at the bottom (the 3D viewport stops above the HUD)
magick "$RAW" -strip -background black -alpha remove -alpha off -gravity North -chop "0x${CROP_TOP:-116}" -gravity South -chop "0x${BOTTOM}" +repage "PNG24:$OUT"
echo "$OUT"
