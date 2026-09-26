#!/bin/bash
# suite.sh APP OUTDIR KIND   (KIND: release | asan)
# Runs one FreeWRL at a time: PROTO-world reload cycles, then the texture fixture repeatedly.
H=$(cd "$(dirname "$0")" && pwd); R=$(cd "$H/../.." && pwd)
APP=$1 OUT=$2 KIND=$3; mkdir -p $OUT
T=$R/freewrl/tests; G=$T/regression
# worlds replaced in turn; 8.wrl, 10.wrl and proto_replace.wrl declare PROTOs (no audio worlds)
export RELOAD_PATHS="$T/8.wrl:$G/texture_formats.wrl:$T/10.wrl:$G/proto_replace.wrl:$T/1.wrl:$T/16.wrl:$G/text_fonts.wrl:$G/route_dotted.wrl:$G/hanim_skin.x3d"
if [ "$KIND" = asan ]; then
  export ASAN_OPTIONS=halt_on_error=0:abort_on_error=0:log_path=$OUT/asan
  CYC=${CYC:-3} CSEC=${CSEC:-300} TEX=${TEX:-5} TSEC=${TSEC:-60}
else
  CYC=${CYC:-5} CSEC=${CSEC:-180} TEX=${TEX:-10} TSEC=${TSEC:-60}
fi
for ((i=1; i<=CYC; i++)); do RELOAD_PERIOD=90 $H/run.sh $APP $G/texture_formats.wrl $CSEC $OUT/cycle-$i; done
unset RELOAD_PATHS
for ((i=1; i<=TEX; i++)); do $H/run.sh $APP $G/texture_formats.wrl $TSEC $OUT/texture-$i; done
if [ "$KIND" = asan ]; then
  echo "ASan reports (excluding known testVector/GLEngine noise):"
  cat $OUT/asan.* 2>/dev/null | grep '^SUMMARY' | grep -vE 'Frustum.c:20[04]|glBufferData_Exec' | sort | uniq -c
fi
