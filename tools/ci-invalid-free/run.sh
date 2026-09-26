#!/bin/bash
# run.sh APP WORLD SECONDS OUTPREFIX
# Launch FreeWRL under lldb (stops on malloc_error_break, abort, crashes) for SECONDS, then stop it.
# RELOAD_PERIOD (frames) and RELOAD_PATHS (colon list): replace the world every RELOAD_PERIOD frames
# by calling dllFreeWRL_onLoad(fwctx, path), which is what the Load button does.
# Prints one result line.
H=$(cd "$(dirname "$0")" && pwd)
APP=$1 W=$2 SEC=$3 O=$4
EXE=$APP/Contents/MacOS/FreeWRL
{
echo "command script import $H/reloader.py"
echo "target create \"$EXE\""
echo "breakpoint set -n malloc_error_break"
echo "breakpoint set -n abort"
if [ -n "$RELOAD_PERIOD" ]; then
  echo "breakpoint set -S animationTimer:"
  echo "breakpoint command add -F reloader.cb 3"
fi
[ -n "$ASAN_OPTIONS" ] && echo "settings set target.env-vars ASAN_OPTIONS=$ASAN_OPTIONS"
echo "process handle SIGUSR1 SIGUSR2 SIGPIPE -n false -p true -s false"
echo "process launch -o $O.out -e $O.err -- \"$W\""
} > $O.lldb
start=$(date +%s)
( for i in $(seq $((SEC*2))); do sleep 0.5; done; pkill -STOP -f "^$EXE" ) &
wd=$!
lldb --batch -s $O.lldb -o "thread backtrace all" -o "process kill" > $O.lldb.log 2>&1
kill $wd 2>/dev/null; pkill -KILL -f "^$EXE" 2>/dev/null
el=$(( $(date +%s) - start ))
last=$(grep -o 'stop reason = .*' $O.lldb.log | tail -1)
if echo "$last" | grep -qE 'breakpoint [12]\.|SIGABRT|EXC_'; then res="CRASH:$last"
elif grep -q "exited with status" $O.lldb.log; then res="EXIT:$(grep -o 'exited with status = [-0-9]*' $O.lldb.log | tail -1)"
else res=alive-stopped; fi
mal=$(grep -h -m1 -o "malloc: \*\*\*.*" $O.err $O.out 2>/dev/null | head -1)
loads=$(grep -c '^RELOAD' $O.lldb.log)
echo "$(basename $O) elapsed=${el}s reloads=$loads $res malloc=${mal:-none}"
