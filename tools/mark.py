#!/usr/bin/env python3
"""Lua's license wants altered files plainly marked. Adds a note under the
$Id line of every file in src/lua that differs from the Lua 4.0 release.

usage: python3 tools/mark.py <lua-4.0 release folder>
"""
import os
import sys

NOTE = "** Changed by Double Fine for Psychonauts, reconstructed from Psychonauts.exe\n"
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "lua")


def original(release, name):
    for sub in ("src", "src/lib", "include"):
        p = os.path.join(release, sub, name)
        if os.path.exists(p):
            return open(p, encoding="latin-1").read()
    return None


def main():
    for name in sorted(os.listdir(ROOT)):
        if not name.endswith((".c", ".h")):
            continue
        path = os.path.join(ROOT, name)
        text = open(path, encoding="latin-1").read()
        orig = original(sys.argv[1], name)
        changed = orig is None or text.replace(NOTE, "") != orig
        lines = text.splitlines(True)
        has = NOTE in lines
        if changed and not has:
            lines.insert(2, NOTE)
        elif not changed and has:
            lines.remove(NOTE)
        else:
            continue
        open(path, "w", encoding="latin-1", newline="").write("".join(lines))
        print(("marked " if changed else "unmarked ") + name)


if __name__ == "__main__":
    main()
