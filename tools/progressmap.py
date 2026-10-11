#!/usr/bin/env python3
"""Draw engine/functions.csv as a treemap.

usage: progressmap.py engine/functions.csv engine/progress.svg

Every function is a rectangle sized by its bytes of code. Functions are grouped
into eight blocks by address, inside them by object file where that is known
(from the matching functions around them), the rest in pieces of similar size. Green: matches the original. Blue: name known, not
decompiled yet. Gray: unknown."""
import csv
import sys

W, H = 950, 475
PAD = 8
COLORS = {'match': ('#1fe04a', '#0aa82e'), 'named': ('#1f7bb5', '#145a87'), 'unknown': ('#343f4a', '#262e37')}
BLOCKS = 8
UNIT = 0x6000


def squarify(items, x, y, w, h):
    """items: [(size, data)] sorted descending. Returns [(data, x, y, w, h)]."""
    out = []
    items = [it for it in items if it[0] > 0]
    total = float(sum(s for s, _ in items))
    if not items or w <= 0 or h <= 0:
        return out
    scale = w * h / total
    rest = [(s * scale, d) for s, d in items]
    while rest:
        short = min(w, h)
        row = [rest[0]]
        i = 1

        def worst(r):
            s = sum(a for a, _ in r)
            mx = max(a for a, _ in r)
            mn = min(a for a, _ in r)
            return max(short * short * mx / (s * s), s * s / (short * short * mn))
        while i < len(rest) and worst(row + [rest[i]]) <= worst(row):
            row.append(rest[i])
            i += 1
        rest = rest[i:]
        s = sum(a for a, _ in row)
        if w >= h:
            cw = s / h
            cy = y
            for a, d in row:
                ch = a / cw
                out.append((d, x, cy, cw, ch))
                cy += ch
            x += cw
            w -= cw
        else:
            ch = s / w
            cx = x
            for a, d in row:
                cwid = a / ch
                out.append((d, cx, y, cwid, ch))
                cx += cwid
            y += ch
            h -= ch
    return out


def chunks(seq, limit, idx=1):
    """split a list into runs of about limit bytes, item[idx] is the size"""
    out, cur, n = [], [], 0
    for item in seq:
        cur.append(item)
        n += item[idx]
        if n >= limit:
            out.append(cur)
            cur, n = [], 0
    if cur:
        out.append(cur)
    return out


def main():
    rows = list(csv.DictReader(open(sys.argv[1])))
    rows.sort(key=lambda r: int(r['address'], 16))
    objs = [r['object'] if r['status'] == 'matching' else '' for r in rows]
    # an unmatched function between two matching ones of the same object belongs to it
    prev, last = [], ''
    for o in objs:
        last = o or last
        prev.append(last)
    nxt, last = [''] * len(rows), ''
    for i in range(len(rows) - 1, -1, -1):
        last = objs[i] or last
        nxt[i] = last
    funcs = []
    for i, r in enumerate(rows):
        unit = objs[i] or (prev[i] if prev[i] and prev[i] == nxt[i] else '')
        kind = 'match' if r['status'] == 'matching' else ('named' if r['name'] else 'unknown')
        funcs.append((int(r['address'], 16), int(r['size']), kind, unit))
    total = sum(f[1] for f in funcs)
    # top level: the code in eight parts by address, like the folders on decomp.dev
    blocks = chunks([(a, s, (k, u)) for a, s, k, u in funcs], total / BLOCKS + 1)
    rects = []
    tops = [(sum(f[1] for f in b), i) for i, b in enumerate(blocks)]
    for bi, bx, by, bw, bh in squarify(sorted(tops, reverse=True), PAD, PAD, W - 2 * PAD, H - 2 * PAD):
        # units: known object files, the rest cut into pieces of similar size to them
        units, run, runkey = [], [], None
        for a, s, (k, u) in blocks[bi]:
            key = u or None
            if run and key != runkey:
                units += [run] if runkey else chunks(run, UNIT, 0)
                run = []
            run.append((s, k))
            runkey = key
        if run:
            units += [run] if runkey else chunks(run, UNIT, 0)
        usz = sorted(((sum(s for s, _ in un), i) for i, un in enumerate(units)), reverse=True)
        for ui, ux, uy, uw, uh in squarify(usz, bx, by, bw, bh):
            for kind, x, y, w, h in squarify(sorted(units[ui], key=lambda t: -t[0]), ux, uy, uw, uh):
                rects.append((kind, x, y, w, h))
            rects.append(('unit', ux, uy, uw, uh))
        rects.append(('block', bx, by, bw, bh))
    grad = ''.join('<linearGradient id="%s" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="%s"/>'
                   '<stop offset="1" stop-color="%s"/></linearGradient>' % (k, c[0], c[1]) for k, c in COLORS.items())
    out = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H),
           '<defs>%s</defs>' % grad,
           '<style>rect{stroke:#0d1117;stroke-width:.5}.match{fill:url(#match)}.named{fill:url(#named)}'
           '.unknown{fill:url(#unknown)}.unit{fill:none;stroke-width:1}.block{fill:none;stroke-width:2.5}</style>',
           '<rect width="%d" height="%d" style="fill:#0d1117;stroke:none"/>' % (W, H)]
    for kind, x, y, w, h in rects:
        out.append('<rect class="%s" x="%.1f" y="%.1f" width="%.1f" height="%.1f"/>' % (kind, x, y, w, h))
    out.append('</svg>')
    open(sys.argv[2], 'w').write('\n'.join(out) + '\n')
    n = {k: 0 for k in COLORS}
    for kind, *_ in rects:
        if kind in n:
            n[kind] += 1
    print(sys.argv[2], n)


if __name__ == '__main__':
    main()
