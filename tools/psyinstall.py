#!/usr/bin/env python3
import os
import shutil
import sys


def files_in(root):
    for base, _, files in os.walk(root):
        for fn in files:
            full = os.path.join(base, fn)
            yield os.path.relpath(full, root)


def install(game, moddir, backup):
    n = 0
    for rel in sorted(files_in(moddir)):
        target = os.path.join(game, rel)
        saved = os.path.join(backup, rel)
        if os.path.exists(target) and not os.path.exists(saved):
            os.makedirs(os.path.dirname(saved), exist_ok=True)
            shutil.copy2(target, saved)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        shutil.copy2(os.path.join(moddir, rel), target)
        print("installed", rel)
        n += 1
    print("%d file(s) installed, originals kept in %s" % (n, backup))


def restore(game, backup):
    n = 0
    for rel in sorted(files_in(backup)):
        shutil.copy2(os.path.join(backup, rel), os.path.join(game, rel))
        print("restored", rel)
        n += 1
    print("%d original file(s) restored" % n)


def main():
    if len(sys.argv) < 4 or sys.argv[1] not in ("install", "restore"):
        print("usage: psyinstall.py install <game dir> <mod dir> <backup dir>")
        print("       psyinstall.py restore <game dir> <backup dir>")
        sys.exit(1)
    if sys.argv[1] == "install":
        install(sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        restore(sys.argv[2], sys.argv[3])


if __name__ == "__main__":
    main()
