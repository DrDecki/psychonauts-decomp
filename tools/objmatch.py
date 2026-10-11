#!/usr/bin/env python3
"""Find functions of MSVC object files in Psychonauts.exe.

Bytes covered by relocations are ignored, everything else has to be equal.
Call targets of matched functions are checked against each other, so a symbol
can only ever end up at one address.
"""
import struct
import sys

import pefile

from coff import ObjFile


class Exe:
    def __init__(self, path):
        pe = pefile.PE(path, fast_load=True)
        pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
        self.base = pe.OPTIONAL_HEADER.ImageBase
        # imported functions, as the compiler names them: __imp__name for the
        # pointer in the import table, _name for the jump stub in .text
        self.imports = {}
        self.iat = {}
        for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
            for imp in entry.imports:
                if imp.name:
                    n = imp.name.decode("latin-1")
                    self.iat[imp.address] = n
                    self.imports["__imp_" + n] = imp.address
                    self.imports["__imp__" + n] = imp.address
        text = [s for s in pe.sections if s.Name.rstrip(b"\0") == b".text"][0]
        self.text_va = self.base + text.VirtualAddress
        self.text = text.get_data()[:text.Misc_VirtualSize]
        pos = self.text.find(b"\xff\x25")
        while pos >= 0:
            iat = struct.unpack_from("<I", self.text, pos + 2)[0]
            if iat in self.iat:
                self.imports.setdefault("_" + self.iat[iat], self.text_va + pos)
            pos = self.text.find(b"\xff\x25", pos + 1)

    def at(self, va, n):
        o = va - self.text_va
        return self.text[o:o + n]


def masked(fn):
    mask = bytearray(fn.size)
    for off, size, _, _ in fn.relocs():
        for k in range(size):
            if off + k < fn.size:
                mask[off + k] = 1
    return mask


def longest_plain(code, mask):
    best = (0, 0)
    start = None
    for i in range(len(code) + 1):
        if i < len(code) and not mask[i]:
            if start is None:
                start = i
        elif start is not None:
            if i - start > best[1] - best[0]:
                best = (start, i)
            start = None
    return best


def find(exe, fn):
    code = fn.code
    mask = masked(fn)
    a, b = longest_plain(code, mask)
    if b - a < 6:
        return []
    needle = code[a:b]
    hits = []
    pos = exe.text.find(needle)
    while pos >= 0:
        start = pos - a
        if start >= 0 and (exe.text_va + start) % 16 == 0:
            cand = exe.text[start:start + len(code)]
            if len(cand) == len(code) and all(m or x == y for x, y, m in zip(code, cand, mask)):
                hits.append(exe.text_va + start)
        pos = exe.text.find(needle, pos + 1)
    return hits


def targets(exe, fn, va):
    """symbol name -> absolute address for REL32/DIR32 relocations"""
    out = {}
    for off, size, name, typ in fn.relocs():
        raw = exe.at(va + off, 4)
        if len(raw) < 4:
            continue
        val = struct.unpack("<I", raw)[0]
        addend = struct.unpack_from("<i", fn.code, off)[0] if off + 4 <= len(fn.code) else 0
        val = (val - addend) & 0xFFFFFFFF
        if typ == 0x14:
            out[name] = (va + off + 4 + val) & 0xFFFFFFFF
        elif typ == 0x06:
            out[name] = val
    return out


def run(exe, objs, known_in=None):
    known = dict(exe.imports)
    known.update(known_in or {})
    results = {}
    candidates = {}
    for obj in objs:
        for fn in obj.functions:
            hits = find(exe, fn)
            candidates[(obj.path, fn.name)] = (fn, hits)
    changed = True
    while changed:
        changed = False
        for key, (fn, hits) in candidates.items():
            if key in results:
                continue
            ok = []
            for va in hits:
                if fn.key in known and known[fn.key] != va:
                    continue
                t = targets(exe, fn, va)
                if any(known.get(n, v) != v for n, v in t.items()):
                    continue
                ok.append((va, t))
            if len(ok) == 1:
                va, t = ok[0]
                results[key] = va
                known.setdefault(fn.key, va)
                for n, v in t.items():
                    known.setdefault(n, v)
                changed = True
    return results, candidates, known


def load_starts(csv_path):
    import csv
    return sorted(int(r["address"], 16) for r in csv.DictReader(open(csv_path)))


def by_order(objs, results, known, starts):
    """Unmatched functions between two matched ones of the same object file
    take the exe functions in between, if the counts agree."""
    import bisect
    guess = {}
    for obj in objs:
        fns = sorted(obj.functions, key=lambda f: (f.section.index, f.offset))
        anchors = []
        for n, fn in enumerate(fns):
            va = results.get((obj.path, fn.name)) or known.get(fn.key)
            if va in starts_set(starts):
                anchors.append((n, va))
        for (n1, v1), (n2, v2) in zip(anchors, anchors[1:]):
            if fns[n1].section is not fns[n2].section:
                continue
            i1 = bisect.bisect_left(starts, v1)
            i2 = bisect.bisect_left(starts, v2)
            between = fns[n1 + 1:n2]
            if between and i2 - i1 - 1 == len(between):
                for k, fn in enumerate(between):
                    guess.setdefault(fn.key, starts[i1 + 1 + k])
    return guess


_starts_cache = {}
def starts_set(starts):
    key = id(starts)
    if key not in _starts_cache:
        _starts_cache[key] = set(starts)
    return _starts_cache[key]


def main():
    import argparse
    import csv
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("exe")
    ap.add_argument("functions", help="engine/functions.csv")
    ap.add_argument("objs", nargs="+")
    ap.add_argument("--write", action="store_true", help="store matched names in functions.csv")
    ap.add_argument("-v", action="store_true", help="list every function")
    args = ap.parse_args()
    exe = Exe(args.exe)
    objs = [ObjFile(p) for p in args.objs]
    results, candidates, known = run(exe, objs)
    per_obj = {}
    for (path, name), (fn, hits) in candidates.items():
        n = per_obj.setdefault(path, [0, 0])
        n[1] += 1
        if (path, name) in results:
            n[0] += 1
        if args.v:
            va = results.get((path, name))
            print("%-14s %-32s %s" % (path.split("/")[-1], name, "0x%08x" % va if va else "-"))
    for path in sorted(per_obj):
        print("%-14s %4d / %4d" % (path.split("/")[-1], per_obj[path][0], per_obj[path][1]))
    rows = list(csv.DictReader(open(args.functions)))
    if args.write:
        by_va = {int(r["address"], 16): r for r in rows}
        for r in rows:
            if r.get("status") == "matching":
                r["name"] = ""
            r["status"] = ""
            r["object"] = ""
        for (path, name), va in results.items():
            r = by_va.get(va)
            if r is None:
                continue
            r["name"] = name[1:] if name.startswith("_") else name
            r["status"] = "matching"
            parts = path.replace("\\", "/").split("/")
            r["object"] = "/".join(parts[-2:])[:-4] + ".c"
        with open(args.functions, "w", newline="") as f:
            w = csv.DictWriter(f, ["address", "size", "origin", "name", "status", "object"], lineterminator="\n")
            w.writeheader()
            w.writerows(rows)
    total = len(rows)
    done = sum(1 for r in rows if r.get("status") == "matching") if args.write else len(results)
    print("matching: %d of %d object functions" % (len(results), len(candidates)))
    print("engine:   %d of %d functions (%.2f %%)" % (done, total, 100.0 * done / total))


if __name__ == "__main__":
    main()
