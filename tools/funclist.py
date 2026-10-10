#!/usr/bin/env python3
import bisect
import struct
import sys

import capstone
import pefile


class Image:
    def __init__(self, path):
        self.pe = pefile.PE(path)
        self.base = self.pe.OPTIONAL_HEADER.ImageBase
        self.data = self.pe.get_memory_mapped_image()
        self.sections = []
        for s in self.pe.sections:
            name = s.Name.rstrip(b"\0").decode("latin-1")
            start = self.base + s.VirtualAddress
            self.sections.append((name, start, start + s.Misc_VirtualSize, s.Characteristics))
        self.text = [s for s in self.sections if s[3] & 0x20000000]
        self.imports = {}
        for entry in getattr(self.pe, "DIRECTORY_ENTRY_IMPORT", []):
            dll = entry.dll.decode("latin-1").lower()
            for imp in entry.imports:
                name = imp.name.decode("latin-1") if imp.name else "ord_%d" % imp.ordinal
                self.imports[imp.address] = "%s!%s" % (dll, name)

    def is_code(self, va):
        return any(a <= va < b for _, a, b, _ in self.text)

    def section(self, va):
        for s in self.sections:
            if s[1] <= va < s[2]:
                return s
        return None

    def read(self, va, n):
        off = va - self.base
        return self.data[off:off + n]

    def dword(self, va):
        return struct.unpack("<I", self.read(va, 4))[0]


class Finder:
    def __init__(self, img):
        self.img = img
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.md.detail = True
        self.funcs = {}
        self.owner = {}
        self.queue = []
        self.jumptables = 0
        self.data_ranges = []
        self.used = {}

    def add(self, va):
        if self.img.is_code(va) and va not in self.funcs and va not in self.queue:
            self.queue.append(va)

    def run(self):
        while self.queue:
            va = self.queue.pop()
            if va in self.funcs:
                continue
            if va in self.owner and self.owner[va] != va:
                continue
            self.explore(va)

    def insn_at(self, va):
        code = self.img.read(va, 16)
        for ins in self.md.disasm(code, va, 1):
            return ins
        return None

    def jump_table(self, ins, blocks):
        op = ins.operands[0]
        if op.type != capstone.x86.X86_OP_MEM or op.mem.scale != 4 or op.mem.base != 0:
            return []
        table = op.mem.disp & 0xFFFFFFFF
        if self.img.section(table) is None:
            return []
        targets = []
        va = table
        while True:
            if va in self.used or (targets and va in self.funcs):
                break
            t = self.img.dword(va)
            if not self.img.is_code(t) or abs(t - ins.address) > 0x10000:
                break
            targets.append(t)
            va += 4
            if len(targets) > 1024:
                break
        if targets:
            self.jumptables += 1
            if self.img.is_code(table):
                self.data_ranges.append((table, va))
        return targets

    def explore(self, start):
        todo = [start]
        seen = set()
        end = start
        while todo:
            va = todo.pop()
            while True:
                if va in seen or not self.img.is_code(va):
                    break
                if va != start and va in self.funcs:
                    break
                if va in self.used and self.used[va] != start:
                    break
                if any(a <= va < b for a, b in self.data_ranges):
                    break
                ins = self.insn_at(va)
                if ins is None:
                    break
                seen.add(va)
                for k in range(ins.size):
                    self.used.setdefault(va + k, start)
                end = max(end, va + ins.size)
                nxt = va + ins.size
                g = set(ins.groups)
                if capstone.x86.X86_GRP_CALL in g:
                    op = ins.operands[0]
                    if op.type == capstone.x86.X86_OP_IMM:
                        self.add(op.imm & 0xFFFFFFFF)
                    va = nxt
                    continue
                if capstone.x86.X86_GRP_RET in g or ins.mnemonic in ("int3", "hlt", "ud2"):
                    break
                if capstone.x86.X86_GRP_JUMP in g:
                    op = ins.operands[0]
                    if op.type == capstone.x86.X86_OP_IMM:
                        tgt = op.imm & 0xFFFFFFFF
                        if ins.mnemonic == "jmp":
                            if tgt in self.funcs or (tgt < start and tgt not in seen):
                                self.add(tgt)
                            else:
                                todo.append(tgt)
                            break
                        todo.append(tgt)
                        va = nxt
                        continue
                    if ins.mnemonic == "jmp":
                        todo.extend(self.jump_table(ins, todo))
                        break
                    va = nxt
                    continue
                va = nxt
        self.funcs[start] = (end - start, seen)
        for v in seen:
            self.owner.setdefault(v, start)

    def covered(self):
        spans = []
        for start, (size, seen) in self.funcs.items():
            spans.append((start, start + size))
        spans.extend(self.data_ranges)
        spans.sort()
        return spans

    def gaps(self):
        spans = self.covered()
        starts = [a for a, _ in spans]
        found = []
        for _, a, b, _ in self.img.text:
            va = a
            while va < b:
                i = bisect.bisect_right(starts, va) - 1
                if i >= 0 and spans[i][0] <= va < spans[i][1]:
                    va = spans[i][1]
                    continue
                if va in self.used:
                    va += 1
                    continue
                byte = self.img.read(va, 1)
                if not byte or byte[0] in (0xCC, 0x90, 0x00):
                    va += 1
                    continue
                nxt = bisect.bisect_right(starts, va)
                found.append(va)
                va = starts[nxt] if nxt < len(starts) else b
        return found


def pointers_into_code(img):
    out = set()
    for name, a, b, ch in img.sections:
        if ch & 0x20000000:
            continue
        va = a
        data = img.read(a, b - a)
        for off in range(0, len(data) - 3, 4):
            v = struct.unpack_from("<I", data, off)[0]
            if img.is_code(v):
                out.add(v)
    return out


PROLOGUES = (b"\x55\x8b\xec", b"\x8b\xff\x55\x8b\xec")
JUNK = {"in", "out", "insb", "insd", "insw", "outsb", "outsd", "outsw", "int3", "hlt", "arpl",
        "bound", "into", "lds", "les", "aaa", "aas", "daa", "das", "aam", "aad", "cli", "sti",
        "salc", "int1", "iretd", "retf", "lock", "wait", "sahf", "lahf", "cmc", "xlatb", "ljmp", "lcall"}


def after_boundary(img, va):
    prev = img.read(va - 3, 3)
    return prev[-1] in (0xCC, 0xC3) or prev[0] == 0xC2


def plausible(f, va):
    if va % 16:
        return False
    code = f.img.read(va, 48)
    if code.startswith(PROLOGUES):
        return True
    if not after_boundary(f.img, va):
        return False
    if code[:2] == b"\x00\x00":
        return False
    n = 0
    for ins in f.md.disasm(code, va):
        if ins.mnemonic in JUNK or ins.mnemonic.startswith("f") and n == 0 and ins.mnemonic not in ("fld", "fldz", "fld1"):
            return False
        n += 1
        if ins.mnemonic in ("ret", "jmp"):
            return True
        if n >= 6:
            return True
    return False


def main():
    img = Image(sys.argv[1])
    out = sys.argv[2] if len(sys.argv) > 2 else "functions.csv"
    f = Finder(img)
    origin = {}

    def explore_all(kind):
        before = set(f.funcs)
        f.run()
        for va in f.funcs:
            if va not in before:
                origin.setdefault(va, kind)

    f.add(img.base + img.pe.OPTIONAL_HEADER.AddressOfEntryPoint)
    explore_all("code")
    rounds = 0
    pointers = sorted(pointers_into_code(img))
    while True:
        new = [g for g in f.gaps() if g not in f.funcs and g % 16 == 0 and img.read(g, 5).startswith(PROLOGUES)]
        for g in new:
            f.add(g)
        explore_all("gap")
        added = 0
        for v in pointers:
            if v not in f.used and v not in f.funcs and plausible(f, v):
                f.add(v)
                added += 1
        explore_all("pointer")
        if not new and not added:
            break
        rounds += 1
    names = {}
    for start in f.funcs:
        ins = f.insn_at(start)
        if ins and ins.mnemonic == "jmp" and ins.operands[0].type == capstone.x86.X86_OP_MEM:
            target = ins.operands[0].mem.disp & 0xFFFFFFFF
            if target in img.imports:
                names[start] = img.imports[target]
    used = len(f.used)
    data = sum(b - a for a, b in f.data_ranges)
    code = sum(b - a for _, a, b, _ in img.text)
    with open(out, "w") as fo:
        fo.write("address,size,origin,name\n")
        for start in sorted(f.funcs):
            fo.write("0x%08x,%d,%s,%s\n" % (start, f.funcs[start][0], origin.get(start, "code"), names.get(start, "")))
    unaligned = sum(1 for va in f.funcs if va % 16)
    kinds = {}
    for v in origin.values():
        kinds[v] = kinds.get(v, 0) + 1
    print("functions %d (%s), import thunks %d" % (len(f.funcs), ", ".join("%s %d" % kv for kv in sorted(kinds.items())), len(names)))
    print("jump tables %d, gap rounds %d, unaligned starts %d" % (f.jumptables, rounds, unaligned))
    print("instruction bytes %d, jump table bytes %d, of %d code bytes (%.1f %% covered)" % (used, data, code, 100.0 * (used + data) / code))
    print("written to %s" % out)


if __name__ == "__main__":
    main()
