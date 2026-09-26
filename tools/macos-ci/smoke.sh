#!/bin/bash
# smoke.sh APP OUTDIR : run each fixture for 25 s (one FreeWRL at a time), check its log, print
# one PASS/FAIL line per fixture. Exit 1 if any fixture fails.
# A fixture fails on a crash, an allocator abort, a GL/shader/script error, a texture that fails
# to load (except in texture_unsupported_mac.wrl, where failing is the expected result), or a
# missing expected log line. A clean exit before the 25 s is the known intermittent early exit:
# the fixture is run once more and the retry is reported.
H=$(cd "$(dirname "$0")" && pwd); R=$(cd "$H/../.." && pwd)
APP=$1 OUT=$2; mkdir -p "$OUT"
T=$R/freewrl/tests; G=$T/regression
BAD='failed to load|problem with (VERTEX|FRAGMENT) shader|GL error|Script error'
fails=0
run() { # name world must-appear(or -) [expected-failures]
	local name=$1 world=$2 want=$3 expect_fail=$4 f=$OUT/smoke-$1 res bad miss note="" nfail
	res=$("$H/run.sh" "$APP" "$world" 25 "$f")
	if echo "$res" | grep -qE 'EXIT:exited with status = [0-8] '; then
		note=" early-exit(retried)"
		res=$("$H/run.sh" "$APP" "$world" 25 "$f")
	fi
	screencapture -x "$f.png" 2>/dev/null || true
	miss="" bad=""
	[ "$want" != - ] && ! grep -qE "$want" "$f.out" "$f.err" && miss=" MISSING:'$want'"
	if [ -n "$expect_fail" ]; then
		nfail=$(grep -hc "failed to load image" "$f.out" "$f.err" | paste -sd+ - | bc)
		[ "$nfail" = "$expect_fail" ] || miss="$miss EXPECTED:$expect_fail-load-failures GOT:$nfail"
		bad=$(grep -hE "$BAD" "$f.out" "$f.err" | grep -v "failed to load image" | head -1)
	else
		bad=$(grep -hE "$BAD" "$f.out" "$f.err" | head -1)
	fi
	verdict=PASS
	echo "$res" | grep -qE 'CRASH:|malloc=[^n]' && verdict=FAIL
	[ -n "$miss$bad" ] && verdict=FAIL
	[ $verdict = FAIL ] && fails=$((fails + 1))
	echo "$verdict $name: $res$note$miss${bad:+ BAD:'$bad'}"
	echo "     renderer: $(grep -hm1 GL_RENDERER "$f.out" "$f.err")"
}
run t1 "$T/1.wrl" -
run t16 "$T/16.wrl" -
run texture "$G/texture_formats.wrl" -
run texture_stb "$G/texture_formats_stb.wrl" -
run texture_unsupported "$G/texture_unsupported_mac.wrl" - 2
run fonts "$G/text_fonts.wrl" -
run route "$G/route_dotted.wrl" "ROUTE_OK"
run hanim "$G/hanim_skin.x3d" "Skinning Method: CPU"
run proto "$G/proto_replace.wrl" -
run gzip_route "$G/gzip_route.wrl" "ROUTE_OK"
run gzip_proto "$G/gzip_proto.wrl" -
run t8 "$T/8.wrl" -
run t10 "$T/10.wrl" -
run t50 "$T/50.wrl" -
# GLCoreCompat attribute arrays left enabled across draws (fix/glcore-client-attrib-oob)
[ -f "$G/glcore_stale_attribs.wrl" ] && run glcore_stale "$G/glcore_stale_attribs.wrl" -
# test 50: audio initialisation (a hosted runner may have no output device; reported, not gated)
if grep -qh "initAL failed" "$OUT/smoke-t50.out" "$OUT/smoke-t50.err"; then
	echo "INFO t50 audio: initAL failed (no usable OpenAL device on this machine)"
else
	echo "INFO t50 audio: OpenAL initialised (no initAL failure logged)"
fi
echo "smoke: $fails fixture(s) failed"
[ $fails = 0 ]
