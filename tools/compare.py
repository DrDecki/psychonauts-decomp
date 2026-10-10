#!/usr/bin/env python3
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from luaformat import load_chunk, internal_code  # noqa: E402

def get(p, path):
    parts = path.split(".")[1:]
    for x in parts:
        p = p.kproto[int(x)]
    return p

def dis(p):
    code, _ = internal_code(p)
    out = []
    for ins in code:
        arg = ""
        if ins.op in ("GETGLOBAL", "SETGLOBAL", "PUSHSTRING", "GETDOTTED", "PUSHSELF"):
            arg = repr(p.kstr[ins.u])
        elif ins.tgt is not None:
            arg = "-> %d" % ins.tgt
        elif ins.op in ("CALL", "TAILCALL", "SETTABLE", "SETLIST", "CLOSURE"):
            arg = "%d %d" % (ins.a, ins.b)
        elif ins.op in ("PUSHINT", "ADDI"):
            arg = str(ins.s)
        else:
            arg = str(ins.u)
        out.append("%3d %-11s %s" % (ins.pc, ins.op, arg))
    return out

def main():
    lub, luafile, path = sys.argv[1], sys.argv[2], sys.argv[3]
    luac = os.environ.get("PSYLUAC")
    orig = load_chunk(open(lub, "rb").read())
    tmp = tempfile.mktemp(suffix=".lub")
    args = [luac, "-n", orig.source or "=(none)", "-o", tmp, luafile]
    if not orig.lines:
        args.insert(1, "-s")
    subprocess.run(args, check=True)
    new = load_chunk(open(tmp, "rb").read())
    a = dis(get(orig, path))
    b = dis(get(new, path))
    pa, pb = get(orig, path), get(new, path)
    print("maxstack", pa.max_stack, pb.max_stack, "kstr equal", pa.kstr == pb.kstr)
    if pa.kstr != pb.kstr:
        print(" orig kstr", pa.kstr[:30])
        print(" new  kstr", pb.kstr[:30])
    n = max(len(a), len(b))
    for i in range(n):
        x = a[i] if i < len(a) else ""
        y = b[i] if i < len(b) else ""
        mark = "   " if x == y else " * "
        print("%-40s%s%s" % (x, mark, y))

main()
