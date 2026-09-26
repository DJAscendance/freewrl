#!/bin/bash
# corpus.sh APP CORPUS_DIR OUTDIR [SECONDS]
# Load every .wrl/.wrz/.x3d/.x3dv under CORPUS_DIR in FreeWRL, one at a time, for SECONDS each
# (default 20), and append one line per world to OUTDIR/results.tsv:
#   result  gzip  seconds  world  detail
# result: ok | crash | abort | early-exit | error (a parse, GL, shader, script or load error in
# the log). gzip: gz when the file starts with the gzip magic (FreeWRL inflates those whatever
# their name).
# Restartable: worlds already in results.tsv are skipped, so after an interruption (runner
# timeout, cancelled job) run it again with the same OUTDIR. Logs of each world are kept only
# when it did not come out ok. Meant for CI runners; it launches FreeWRL hundreds of times.
H=$(cd "$(dirname "$0")" && pwd)
APP=$1 CORPUS=$2 OUT=$3 SEC=${4:-20}
[ -x "$APP/Contents/MacOS/FreeWRL" ] && [ -d "$CORPUS" ] && [ -n "$OUT" ] ||
	{ sed -n '2,12p' "$0"; exit 2; }
mkdir -p "$OUT/logs"
R=$OUT/results.tsv
[ -f "$R" ] || printf 'result\tgzip\tseconds\tworld\tdetail\n' > "$R"
ERR='failed to load|problem with (VERTEX|FRAGMENT) shader|GL error|Script error|Parse error|parse error|syntax error'
find "$CORPUS" -type f \( -iname '*.wrl' -o -iname '*.wrz' -o -iname '*.x3d' -o -iname '*.x3dv' \) -print0 | sort -z |
while IFS= read -r -d '' w; do
	rel=${w#"$CORPUS"/}
	cut -f4 "$R" | grep -Fxq -- "$rel" && continue
	gz=plain; [ "$(head -c2 "$w" | od -An -tx1 | tr -d ' \n')" = 1f8b ] && gz=gz
	key=$(printf '%s' "$rel" | shasum | cut -c1-12)
	o=$OUT/logs/$key
	line=$("$H/run.sh" "$APP" "$w" "$SEC" "$o")
	el=$(echo "$line" | sed -n 's/.*elapsed=\([0-9]*\)s.*/\1/p')
	if echo "$line" | grep -q 'CRASH:'; then res=crash detail=$(echo "$line" | grep -o 'CRASH:.*' | sed 's/ malloc=.*//')
	elif echo "$line" | grep -qv 'malloc=none$'; then res=abort detail=$(echo "$line" | grep -o 'malloc=.*')
	elif echo "$line" | grep -qE 'EXIT:exited with status = [0-8] '; then res=early-exit detail=$(echo "$line" | grep -o 'EXIT:[^m]*')
	elif grep -qhE "$ERR" "$o.out" "$o.err" 2>/dev/null; then res=error detail=$(grep -hE "$ERR" "$o.out" "$o.err" | head -1 | tr '\t' ' ' | cut -c1-160)
	else res=ok detail=""; fi
	printf '%s\t%s\t%s\t%s\t%s\n' "$res" "$gz" "${el:-?}" "$rel" "$detail" >> "$R"
	[ $res = ok ] && rm -f "$o".*
	echo "$res $gz $rel"
done
echo "== totals"
tail -n +2 "$R" | cut -f1,2 | sort | uniq -c
