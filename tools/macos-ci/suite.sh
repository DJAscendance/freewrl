#!/bin/bash
# suite.sh APP OUTDIR KIND   (KIND: release | asan)
# One FreeWRL at a time: world-replacement cycles (PROTO worlds among others), then the texture
# fixtures repeatedly. Prints one line per run and a gate summary; exits 1 if the gate fails.
#   CYC  replacement cycles, CSEC seconds each (the world is replaced every 90 frames)
#   TEX  runs of texture_formats.wrl, TSTB runs of texture_formats_stb.wrl, TSEC seconds each
# Gate: no crash, no allocator abort, no texture "failed to load" or GL/shader error; with asan,
# no AddressSanitizer report other than the known, separately tracked defects listed below, and
# none at all in the two fixed PROTO lifetime defects.
H=$(cd "$(dirname "$0")" && pwd); R=$(cd "$H/../.." && pwd)
APP=$1 OUT=$2 KIND=$3; mkdir -p "$OUT"
T=$R/freewrl/tests; G=$T/regression
# replaced in turn; 8.wrl, 10.wrl, proto_replace.wrl and gzip_proto.wrl declare PROTOs (no audio)
export RELOAD_PATHS="$T/8.wrl:$G/texture_formats.wrl:$T/10.wrl:$G/proto_replace.wrl:$T/1.wrl:$T/16.wrl:$G/text_fonts.wrl:$G/route_dotted.wrl:$G/hanim_skin.x3d:$G/gzip_proto.wrl:$G/texture_formats_stb.wrl"
if [ "$KIND" = asan ]; then
	export ASAN_OPTIONS=halt_on_error=0:abort_on_error=0:log_path=$OUT/asan
	CYC=${CYC:-3} CSEC=${CSEC:-300} TEX=${TEX:-5} TSTB=${TSTB:-2} TSEC=${TSEC:-60}
else
	CYC=${CYC:-5} CSEC=${CSEC:-180} TEX=${TEX:-50} TSTB=${TSTB:-10} TSEC=${TSEC:-40}
fi
BAD='failed to load|problem with (VERTEX|FRAGMENT) shader|GL error'
for ((i=1; i<=CYC; i++)); do RELOAD_PERIOD=90 "$H/run.sh" "$APP" "$G/texture_formats.wrl" $CSEC "$OUT/cycle-$i"; done | tee "$OUT/cycles.txt"
unset RELOAD_PATHS
{
for ((i=1; i<=TEX; i++)); do "$H/run.sh" "$APP" "$G/texture_formats.wrl" $TSEC "$OUT/texture-$i"; done
for ((i=1; i<=TSTB; i++)); do "$H/run.sh" "$APP" "$G/texture_formats_stb.wrl" $TSEC "$OUT/texstb-$i"; done
} | tee "$OUT/textures.txt"

fail=0
crashes=$(cat "$OUT/cycles.txt" "$OUT/textures.txt" | grep -c 'CRASH:')
mallocs=$(cat "$OUT/cycles.txt" "$OUT/textures.txt" | grep -vc 'malloc=none$')
early=$(cat "$OUT/cycles.txt" "$OUT/textures.txt" | grep -cE 'EXIT:exited with status = [0-8] ')
reloads=$(grep -o 'reloads=[0-9]*' "$OUT/cycles.txt" | cut -d= -f2 | paste -sd+ - | bc)
texbad=$(cat "$OUT"/texture-*.out "$OUT"/texture-*.err "$OUT"/texstb-*.out "$OUT"/texstb-*.err 2>/dev/null | grep -cE "$BAD")
cycbad=$(cat "$OUT"/cycle-*.out "$OUT"/cycle-*.err 2>/dev/null | grep -cE "$BAD")
echo "GATE runs: $CYC cycles (${reloads:-0} world replacements), $TEX texture_formats + $TSTB texture_formats_stb runs"
echo "GATE crashes=$crashes allocator-aborts=$mallocs texture-errors=$texbad cycle-errors=$cycbad early-clean-exits=$early (known defect, not gated)"
[ "$crashes" = 0 ] && [ "$mallocs" = 0 ] && [ "$texbad" = 0 ] && [ "$cycbad" = 0 ] || fail=1
if [ "$KIND" = asan ]; then
	# fixed PROTO lifetime defects: ed0bc2072 (declarations in the node table, read by the render
	# loop after being freed) and a0f08d1a5 (nested declarations freed by several contexts)
	PROTO='gc_broto_instance|startOfLoopNodeUpdates|getTypeNode|hasSiblingAffectorField|walk_fields|freeMallocedNodeFields|deleteVector_'
	# known defects tracked separately (MACOS-STATUS.md), not this lane's
	KNOWN='Vector.c:[0-9]+ in vector_removeElement|Frustum.c:[0-9]+ in extent6f_union_extent6f|glBufferData_Exec|compile_IndexedLineSet|textureTransform_start'
	cat "$OUT"/asan.* 2>/dev/null | grep '^SUMMARY' | sort | uniq -c > "$OUT/asan-summary.txt"
	proto=$(grep -cE "$PROTO" "$OUT/asan-summary.txt")
	other=$(grep -vE "$PROTO|$KNOWN" "$OUT/asan-summary.txt" | grep -c .)
	echo "ASan reports (count, kind, place):"; sed 's/^/  /' "$OUT/asan-summary.txt"
	echo "GATE asan PROTO-defect reports=$proto unknown reports=$other (known separate defects listed above, not gated)"
	[ "$proto" = 0 ] && [ "$other" = 0 ] || fail=1
fi
[ $fail = 0 ] && echo "GATE PASS" || echo "GATE FAIL"
exit $fail
