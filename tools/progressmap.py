#!/usr/bin/env python3
"""Draw engine/functions.csv as a treemap.

usage: progressmap.py engine/functions.csv engine/progress.svg

Every function is a rectangle sized by its bytes of code. Functions are grouped
by object file where that is known (from the matching functions around them),
the rest by address. Green: matches the original. Blue: name known, not
decompiled yet. Gray: unknown."""
import csv
import sys

W, H = 950, 475
PAD = 8
COLORS = {'match': '#13c43a', 'named': '#1a6a9e', 'unknown': '#2c3540'}
REGION = 0x10000


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


def main():
    rows = list(csv.DictReader(open(sys.argv[1])))
    rows.sort(key=lambda r: int(r['address'], 16))
    # object of each function: its own, or the one of the matching functions on both sides
    objs = [r['object'] if r['status'] == 'matching' else '' for r in rows]
    prev = [''] * len(rows)
    last = ''
    for i, o in enumerate(objs):
        if o:
            last = o
        prev[i] = last
    nxt = [''] * len(rows)
    last = ''
    for i in range(len(rows) - 1, -1, -1):
        if objs[i]:
            last = objs[i]
        nxt[i] = last
    groups = {}
    for i, r in enumerate(rows):
        if objs[i]:
            g = objs[i]
        elif prev[i] and prev[i] == nxt[i]:
            g = prev[i]
        else:
            g = '%06x' % (int(r['address'], 16) // REGION * REGION)
        kind = 'match' if r['status'] == 'matching' else ('named' if r['name'] else 'unknown')
        groups.setdefault(g, []).append((int(r['size']), kind))
    top = sorted(((sum(s for s, _ in v), k) for k, v in groups.items()), reverse=True)
    rects = []
    for g, gx, gy, gw, gh in squarify([(s, k) for s, k in top], PAD, PAD, W - 2 * PAD, H - 2 * PAD):
        items = sorted(groups[g], key=lambda t: -t[0])
        for kind, x, y, w, h in squarify(items, gx, gy, gw, gh):
            rects.append((kind, x, y, w, h))
        rects.append(('group', gx, gy, gw, gh))
    out = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H),
           '<style>rect{stroke:#0d1117;stroke-width:.3}.match{fill:%s}.named{fill:%s}.unknown{fill:%s}'
           '.group{fill:none;stroke:#0d1117;stroke-width:1.5}</style>' % (COLORS['match'], COLORS['named'], COLORS['unknown']),
           '<rect width="%d" height="%d" rx="6" style="fill:#0d1117;stroke:none"/>' % (W, H)]
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
