#!/usr/bin/env python3
import hashlib
import json
import os
import struct
import subprocess
import sys
import tempfile
import traceback

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from luaformat import load_chunk, load_chunk_at, FormatError  # noqa: E402
from luaast import Printer, Closure  # noqa: E402
import dec  # noqa: E402
import naming  # noqa: E402


def closure_lines(func):
    lines = []

    def visit_expr(e):
        if e is None:
            return
        if isinstance(e, Closure):
            lines.append(e.func.line_defined)
            visit_stmts(e.func.body)
            return
        for name in ("a", "b", "t", "k", "fn", "obj"):
            x = getattr(e, name, None)
            if x is not None and hasattr(x, "line"):
                visit_expr(x)
        for name in ("args", "items", "upvals"):
            for x in getattr(e, name, None) or []:
                visit_expr(x)
        for kind, items in getattr(e, "parts", None) or []:
            for it in items:
                if kind == "list":
                    visit_expr(it)
                else:
                    visit_expr(it[0])
                    visit_expr(it[1])

    def visit_stmts(stmts):
        for s in stmts:
            for name in ("targets", "exprs"):
                for x in getattr(s, name, None) or []:
                    visit_expr(x)
            for name in ("call", "cond", "start", "limit", "step", "table"):
                x = getattr(s, name, None)
                if x is not None and hasattr(x, "line"):
                    visit_expr(x)
            for name in ("then", "orelse", "body"):
                b = getattr(s, name, None)
                if b:
                    visit_stmts(b)

    visit_stmts(func.body)
    return lines


def to_source(func, debug):
    p = Printer(debug)
    p.targets = closure_lines(func)
    p.block(func.body)
    return p.text()


def split_lpf(data):
    entries = []
    counts = []
    pos = 4
    left = 0
    while pos < len(data):
        if left == 0:
            left = struct.unpack_from("<H", data, pos)[0]
            counts.append(left)
            pos += 2
            continue
        left -= 1
        nlen = struct.unpack_from("<H", data, pos)[0]
        pos += 2
        name = data[pos:pos + nlen].split(b"\0")[0].decode("latin-1")
        pos += nlen
        size = struct.unpack_from("<I", data, pos)[0]
        pos += 4
        if pos + size > len(data):
            raise FormatError("lpf entry %s runs past end" % name)
        entries.append((name, pos, pos + size))
        pos += size
    if left:
        raise FormatError("lpf ends early")
    return data[:4], counts, entries


LPF_MAGIC = b"\xfc\xfc\x01\x00"
LUA_MAGIC = b"\x1bLua\x40"


def valid_name(raw):
    if len(raw) < 2 or len(raw) > 400 or raw[-1] != 0:
        return False
    body = raw[:-1]
    return all(32 <= c < 127 for c in body)


def scan_lpf(data, start):
    """Parse an lpf embedded at start; returns (counts, entries, end) or None."""
    pos = start + 4
    counts = []
    entries = []
    while pos + 2 <= len(data):
        cnt = struct.unpack_from("<H", data, pos)[0]
        if cnt == 0 or cnt > 20000:
            break
        p = pos + 2
        block = []
        ok = True
        for _ in range(cnt):
            if p + 2 > len(data):
                ok = False
                break
            nlen = struct.unpack_from("<H", data, p)[0]
            raw = data[p + 2:p + 2 + nlen]
            if not valid_name(raw):
                ok = False
                break
            q = p + 2 + nlen
            if q + 4 > len(data):
                ok = False
                break
            size = struct.unpack_from("<I", data, q)[0]
            q += 4
            if size == 0 or q + size > len(data):
                ok = False
                break
            body = data[q:q + 4]
            if body != b"\x1bLua" and not all(c in (9, 10, 13) or 32 <= c < 127 for c in data[q:q + min(size, 64)]):
                ok = False
                break
            block.append((raw[:-1].decode("latin-1"), q, q + size))
            p = q + size
        if not ok:
            break
        counts.append(cnt)
        entries.extend(block)
        pos = p
    if not entries:
        return None
    return counts, entries, pos


def source_path(src):
    if not src or not src.startswith("@"):
        return None
    s = src[1:].replace("\\", "/")
    low = s.lower()
    i = low.find("/workresource/")
    if i >= 0:
        s = s[i + 1:]
    elif ":" in s:
        s = s.split(":", 1)[1].lstrip("/")
    return s


def lpf_entry_path(name):
    if name.lower().endswith(".lua"):
        return name
    return name.replace(".", "/") + ".lua"


def compare(a, b, stats, debug, problems, path="main"):
    stats["functions"] += 1
    bad = []
    if a.line_defined != b.line_defined:
        bad.append("lineDefined")
    if a.num_params != b.num_params or a.is_vararg != b.is_vararg:
        bad.append("params")
    if a.max_stack != b.max_stack:
        bad.append("maxstack")
    if a.kstr != b.kstr:
        bad.append("strings")
    if [struct.pack("<f", x) for x in a.knum] != [struct.pack("<f", x) for x in b.knum]:
        bad.append("numbers")
    if a.raw != b.raw:
        bad.append("code")
    if len(a.kproto) != len(b.kproto):
        bad.append("functions")
    if debug and (a.locals != b.locals or a.lines != b.lines):
        bad.append("debuginfo")
    if not bad:
        stats["matched"] += 1
    else:
        problems.append("%s %s" % (path, ",".join(bad)))
    for i, p in enumerate(a.kproto):
        if i < len(b.kproto):
            compare(p, b.kproto[i], stats, debug, problems, "%s.%d" % (path, i))
        else:
            stats["functions"] += count_funcs(p)


def count_funcs(p):
    return 1 + sum(count_funcs(q) for q in p.kproto)


FAILED_DIR = None
LUADIR = None


def keep_failed(label, data):
    if FAILED_DIR is None:
        return
    h = hashlib.sha1(data).hexdigest()[:12]
    name = label.replace("/", "__").replace("@", "_at_")[-80:]
    os.makedirs(FAILED_DIR, exist_ok=True)
    for fn in os.listdir(FAILED_DIR):
        if fn.startswith(h):
            return
    with open(os.path.join(FAILED_DIR, "%s_%s.lub" % (h, name)), "wb") as fo:
        fo.write(data)


def file_hash(path):
    return hashlib.sha1(open(path, "rb").read()).hexdigest()


def write_source(out_path, text):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="latin-1", newline="\n") as fo:
        fo.write(text)


def verify(orig, out_path, luac, debug):
    fd, tmp = tempfile.mkstemp(suffix=".lub")
    os.close(fd)
    args = [luac, "-n", orig.source or "=(none)", "-o", tmp, out_path]
    if not debug:
        args.insert(1, "-s")
    r = subprocess.run(args, capture_output=True, text=True)
    try:
        if r.returncode != 0:
            return None, "recompile failed: %s" % " ".join((r.stderr or "").split())[:200]
        new = load_chunk(open(tmp, "rb").read())
    finally:
        os.unlink(tmp)
    result = {"functions": 0, "matched": 0}
    problems = []
    compare(orig, new, result, debug, problems)
    return result, problems


def handle_chunk(label, data, out_path, luac, stats, report, info=None):
    try:
        orig = load_chunk(data)
    except FormatError as ex:
        report.append("%s: cannot load: %s" % (label, ex))
        stats["failed"] += 1
        return
    if info is not None:
        info["source"] = orig.source
        info["debug"] = bool(orig.lines)
    debug = bool(orig.lines)
    stats["chunks"] += 1
    try:
        func = dec.decompile_chunk(orig)
        saved = naming.apply(func) if luac else None
        text = to_source(func, debug)
    except Exception as ex:
        stats["functions"] += count_funcs(orig)
        stats["failed"] += 1
        report.append("%s: decompile failed: %s" % (label, ex))
        keep_failed(label, data)
        if os.environ.get("PSYDEC_TRACE"):
            traceback.print_exc()
        return
    write_source(out_path, text)
    if luac is None:
        stats["functions"] += count_funcs(orig)
        report.append("%s: written, not verified" % label)
    else:
        result, problems = verify(orig, out_path, luac, debug)
        if result is None or problems:
            naming.restore(saved)
            write_source(out_path, to_source(func, debug))
            result, problems = verify(orig, out_path, luac, debug)
            if result is not None:
                stats["plain_names"] += 1
        if result is None:
            stats["functions"] += count_funcs(orig)
            stats["failed"] += 1
            report.append("%s: %s" % (label, problems))
            keep_failed(label, data)
        else:
            stats["functions"] += result["functions"]
            stats["matched"] += result["matched"]
            if problems:
                keep_failed(label, data)
            report.append("%s: %d/%d%s" % (label, result["matched"], result["functions"],
                                           ("  " + "; ".join(problems[:5])) if problems else ""))
    if info is not None:
        info["sha1"] = file_hash(out_path)


def handle_lpf_entries(rel, data, counts, entries, base, luac, stats, report):
    seen = {}
    items = []
    for name, s, e in entries:
        p = lpf_entry_path(name)
        if p in seen:
            seen[p] += 1
            root_, ext = os.path.splitext(p)
            p = "%s_%d%s" % (root_, seen[p], ext)
        else:
            seen[p] = 1
        out_path = os.path.join(base, p)
        chunk = data[s:e]
        kind = "bytecode" if chunk[:4] == b"\x1bLua" else "text"
        item = {"name": name, "file": os.path.relpath(out_path, LUADIR), "kind": kind}
        items.append(item)
        if kind == "text":
            os.makedirs(os.path.dirname(out_path), exist_ok=True)
            with open(out_path, "wb") as fo:
                fo.write(chunk)
            item["sha1"] = file_hash(out_path)
            stats["text"] += 1
            report.append("%s/%s: plain text source" % (rel, name))
        else:
            handle_chunk("%s/%s" % (rel, name), chunk, out_path, luac, stats, report, item)
    return items


def handle_container(rel, data, luadir, luac, stats, report):
    stem = rel.rsplit(".", 1)[0]
    result = {"packs": [], "loose": []}
    covered = []
    if rel.lower().endswith(".lpf"):
        candidates = [0]
    else:
        candidates = []
        i = data.find(LPF_MAGIC)
        while i >= 0:
            candidates.append(i)
            i = data.find(LPF_MAGIC, i + 1)
    n = 0
    for off in candidates:
        if any(a <= off < b for a, b in covered):
            continue
        r = scan_lpf(data, off)
        if r is None:
            continue
        counts, entries, end = r
        base = os.path.join(luadir, stem) if n == 0 else os.path.join(luadir, "%s_pack%d" % (stem, n + 1))
        items = handle_lpf_entries(rel, data, counts, entries, base, luac, stats, report)
        result["packs"].append({"offset": off, "end": end, "magic": data[off:off + 4].hex(), "blocks": counts, "entries": items})
        covered.append((off, end))
        n += 1
    i = data.find(LUA_MAGIC)
    while i >= 0:
        if not any(a <= i < b for a, b in covered):
            try:
                proto, end = load_chunk_at(data, i)
            except (FormatError, struct.error, IndexError, UnicodeDecodeError):
                end = None
            if end is not None:
                sp = source_path(proto.source)
                name = sp if sp else "chunk_%08x.lua" % i
                if name.lower().endswith(".lua"):
                    name = name[:-4]
                out_path = os.path.join(luadir, stem, name + ".lua")
                if os.path.exists(out_path):
                    out_path = os.path.join(luadir, stem, "%s_%08x.lua" % (name, i))
                item = {"offset": i, "end": end, "file": os.path.relpath(out_path, luadir)}
                handle_chunk("%s@%08x" % (rel, i), data[i:end], out_path, luac, stats, report, item)
                result["loose"].append(item)
                covered.append((i, end))
                i = data.find(LUA_MAGIC, end)
                continue
        i = data.find(LUA_MAGIC, i + 1)
    return result


def main():
    if len(sys.argv) < 3:
        print("usage: psydec.py <game dir> <out dir> [psyluac]")
        sys.exit(1)
    game, outdir = sys.argv[1], sys.argv[2]
    luac = sys.argv[3] if len(sys.argv) > 3 else None
    global FAILED_DIR, LUADIR
    FAILED_DIR = os.path.join(outdir, "failed")
    luadir = os.path.join(outdir, "lua")
    LUADIR = luadir
    os.makedirs(luadir, exist_ok=True)
    stats = {"chunks": 0, "functions": 0, "matched": 0, "failed": 0, "text": 0, "plain_names": 0}
    report = []
    manifest = {}
    for root, dirs, files in os.walk(game):
        dirs.sort()
        for fn in sorted(files):
            full = os.path.join(root, fn)
            rel = os.path.relpath(full, game).replace("\\", "/")
            low = fn.lower()
            if low.endswith(".lub"):
                out_path = os.path.join(luadir, rel[:-4] + ".lua")
                item = {"file": os.path.relpath(out_path, luadir)}
                handle_chunk(rel, open(full, "rb").read(), out_path, luac, stats, report, item)
                manifest[rel] = {"lub": item}
            elif low.endswith((".lpf", ".ppf", ".pkg")):
                data = open(full, "rb").read()
                manifest[rel] = handle_container(rel, data, luadir, luac, stats, report)
    with open(os.path.join(outdir, "report.txt"), "w") as fo:
        fo.write("\n".join(report) + "\n")
    with open(os.path.join(outdir, "manifest.json"), "w") as fo:
        json.dump(manifest, fo, indent=1)
    total = stats["functions"]
    print("bytecode chunks %d, plain text sources %d, failed %d" % (stats["chunks"], stats["text"], stats["failed"]))
    if luac:
        pct = 100.0 * stats["matched"] / total if total else 0.0
        print("functions %d, byte identical %d" % (total, stats["matched"]))
        if stats["plain_names"]:
            print("chunks that kept generic names: %d" % stats["plain_names"])
        print("MATCHING: %.1f %%" % pct)
    print("sources in %s, details in %s" % (luadir, os.path.join(outdir, "report.txt")))


if __name__ == "__main__":
    main()
