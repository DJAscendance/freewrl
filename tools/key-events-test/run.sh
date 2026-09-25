#!/bin/sh
# Builds and runs the Cocoa keyboard mapping test, then checks that the engine's
# private KEYPRESS/KEYDOWN/KEYUP defines still match the canonical KeyAction enum
# in cdllFreeWRL.h (MainLoop.c and Component_KeyDevice.c keep their own copies).
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
out=${TMPDIR:-/tmp}/freewrl_test_key_events.$$
cc -std=c99 -Wall -Wextra -Werror -o "$out" "$here/test_key_events.c"
status=0
"$out" || status=1
rm -f "$out"

for f in freex3d/src/lib/main/MainLoop.c freex3d/src/lib/scenegraph/Component_KeyDevice.c; do
	for pair in KEYPRESS:1 KEYDOWN:2 KEYUP:3; do
		name=${pair%:*}; val=${pair#*:}
		bad=$(grep -E "^#define[[:space:]]+$name[[:space:]]" "$root/$f" | grep -vE "[[:space:]]$val[[:space:]]*$" || true)
		if [ -n "$bad" ]; then
			echo "FAIL $f: $bad (expected $val)"; status=1
		else
			echo "PASS $f: $name == $val"
		fi
	done
done
exit $status
