#!/usr/bin/env python3
import hashlib
import json
import os
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from luaformat import load_chunk, FormatError  # noqa: E402


class PackError(Exception):
    pass


def sha1(path):
    return hashlib.sha1(open(path, "rb").read()).hexdigest()


def compile_lua(luac, path, source, debug):
    fd, tmp = tempfile.mkstemp(suffix=".lub")
    os.close(fd)
    args = [luac, "-n", source or "=(none)", "-o", tmp, path]
    if not debug:
        args.insert(1, "-s")
    r = subprocess.run(args, capture_output=True, text=True)
    try:
        if r.returncode != 0:
            raise PackError("%s: %s" % (path, " ".join((r.stderr or "").split())))
        data = open(tmp, "rb").read()
    finally:
        os.unlink(tmp)
    load_chunk(data)
    return data


def entry_bytes(item, original, luadir, luac, changed):
    path = os.path.join(luadir, item["file"])
    if not os.path.exists(path):
        raise PackError("missing %s" % path)
    if sha1(path) == item.get("sha1"):
        return original
    changed.append(item["file"])
    if item.get("kind") == "text":
        return open(path, "rb").read()
    return compile_lua(luac, path, item.get("source"), item.get("debug", False))


def rebuild_lpf(pack, data, luadir, luac, changed):
    off, end = pack["offset"], pack["end"]
    pos = off + 4
    originals = []
    for cnt in pack["blocks"]:
        pos += 2
        for _ in range(cnt):
            nlen = struct.unpack_from("<H", data, pos)[0]
            pos += 2 + nlen
            size = struct.unpack_from("<I", data, pos)[0]
            pos += 4
            originals.append(data[pos:pos + size])
            pos += size
    if pos != end or len(originals) != len(pack["entries"]):
        raise PackError("pack layout differs from manifest")
    out = bytearray(bytes.fromhex(pack["magic"]))
    i = 0
    for cnt in pack["blocks"]:
        out += struct.pack("<H", cnt)
        for _ in range(cnt):
            item = pack["entries"][i]
            body = entry_bytes(item, originals[i], luadir, luac, changed)
            name = item["name"].encode("latin-1") + b"\0"
            out += struct.pack("<H", len(name)) + name + struct.pack("<I", len(body)) + body
            i += 1
    return bytes(out)


def main():
    if len(sys.argv) not in (5, 6):
        print("usage: psypack.py <game dir> <decomp dir> <psyluac> <mod dir> [backup dir]")
        sys.exit(1)
    game, decomp, luac, moddir = sys.argv[1:5]
    backup = sys.argv[5] if len(sys.argv) == 6 else None
    luadir = os.path.join(decomp, "lua")
    manifest = json.load(open(os.path.join(decomp, "manifest.json")))
    written = 0
    errors = 0
    for rel, info in sorted(manifest.items()):
        changed = []
        src = os.path.join(game, rel)
        if backup and os.path.exists(os.path.join(backup, rel)):
            src = os.path.join(backup, rel)
        try:
            data = open(src, "rb").read()
            if "lub" in info:
                new = entry_bytes(info["lub"], data, luadir, luac, changed)
            else:
                pieces = []
                for pack in info.get("packs", []):
                    pieces.append((pack["offset"], pack["end"], rebuild_lpf(pack, data, luadir, luac, changed)))
                for item in info.get("loose", []):
                    body = entry_bytes(dict(item, kind="bytecode"), data[item["offset"]:item["end"]], luadir, luac, changed)
                    if len(body) != item["end"] - item["offset"] and rel.lower().endswith(".pkg"):
                        raise PackError("%s: changed chunk size inside .pkg is not supported yet" % item["file"])
                    pieces.append((item["offset"], item["end"], body))
                pieces.sort()
                out = bytearray()
                pos = 0
                for a, b, body in pieces:
                    out += data[pos:a] + body
                    pos = b
                out += data[pos:]
                new = bytes(out)
        except (PackError, FormatError, OSError) as ex:
            print("ERROR %s: %s" % (rel, ex))
            errors += 1
            continue
        if not changed:
            continue
        if new == data:
            print("%s: %d script(s) recompiled, file unchanged" % (rel, len(changed)))
            continue
        dst = os.path.join(moddir, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        with open(dst, "wb") as fo:
            fo.write(new)
        written += 1
        print("%s: %s (%+d bytes)" % (rel, ", ".join(changed), len(new) - len(data)))
    print("files written to %s: %d, errors: %d" % (moddir, written, errors))
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
