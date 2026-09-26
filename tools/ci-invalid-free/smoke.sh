#!/bin/bash
# smoke.sh APP OUTDIR : one FreeWRL at a time; each fixture for 25 s, then checks its log.
H=$(cd "$(dirname "$0")" && pwd); R=$(cd "$H/../.." && pwd)
APP=$1 OUT=$2; mkdir -p $OUT
T=$R/freewrl/tests; G=$T/regression
check() { # name pattern-that-must-appear (or -)
	f=$OUT/smoke-$1
	miss=""
	[ "$2" != - ] && ! grep -qE "$2" $f.out $f.err && miss=" MISSING:'$2'"
	bad=$(grep -hE "failed to load|problem with (VERTEX|FRAGMENT) shader|GL error|Script error" $f.out $f.err | head -1)
	echo "   renderer: $(grep -hm1 GL_RENDERER $f.out $f.err)$miss${bad:+ BAD:'$bad'}"
}
run() { # name world pattern
	$H/run.sh "$APP" "$2" 25 $OUT/smoke-$1
	screencapture -x $OUT/smoke-$1.png 2>/dev/null || true
	check $1 "$3"
}
run t1 $T/1.wrl -
run t16 $T/16.wrl -
run texture $G/texture_formats.wrl -
run fonts $G/text_fonts.wrl -
run route $G/route_dotted.wrl "ROUTE_OK"
run hanim $G/hanim_skin.x3d "Skinning Method: CPU"
run proto $G/proto_replace.wrl -
run t8 $T/8.wrl -
run t10 $T/10.wrl -
run t50 $T/50.wrl -
