import lldb, os
PERIOD = int(os.environ.get("RELOAD_PERIOD", "0"))
PATHS = [p for p in os.environ.get("RELOAD_PATHS", "").split(":") if p]
st = {"n": 0, "loads": 0}
def cb(frame, bp_loc, extra_args, internal_dict):
    st["n"] += 1
    if PERIOD and PATHS and st["n"] % PERIOD == 0:
        p = PATHS[st["loads"] % len(PATHS)]
        st["loads"] += 1
        v = frame.EvaluateExpression('(void)dllFreeWRL_onLoad(fwctx, (char*)"%s")' % p)
        err = v.GetError()
        print("RELOAD %d %s %s" % (st["loads"], os.path.basename(p), "" if err.Success() or "no value" in str(err) else err), flush=True)
    return False
def __lldb_init_module(debugger, internal_dict):
    pass
