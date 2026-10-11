"""Minimal reader for MSVC i386 COFF object files."""
import struct

REL_SIZES = {0x06: 4, 0x07: 4, 0x0A: 2, 0x0B: 4, 0x14: 4}


class Section:
    def __init__(self, index, name, data, chars, relocs):
        self.index = index
        self.name = name
        self.data = data
        self.chars = chars
        self.relocs = relocs

    @property
    def is_code(self):
        return bool(self.chars & 0x20)

    @property
    def is_comdat(self):
        return bool(self.chars & 0x1000)


class Symbol:
    def __init__(self, index, name, value, section, type_, storage, aux):
        self.index = index
        self.name = name
        self.value = value
        self.section = section
        self.type = type_
        self.storage = storage
        self.aux = aux

    @property
    def is_function(self):
        return (self.type >> 4) == 2


class Function:
    def __init__(self, name, section, offset, size, static):
        self.name = name
        self.section = section
        self.offset = offset
        self.size = size
        self.static = static

    @property
    def code(self):
        return self.section.data[self.offset:self.offset + self.size]

    def relocs(self):
        """(offset inside function, size, symbol name, type)"""
        out = []
        for va, sym, typ, name in self.section.relocs:
            if self.offset <= va < self.offset + self.size:
                out.append((va - self.offset, REL_SIZES.get(typ, 4), name, typ))
        return out


class ObjFile:
    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            raw = f.read()
        self.raw = raw
        machine, nsec, _, symptr, nsym, optsize, _ = struct.unpack_from("<HHIIIHH", raw, 0)
        if machine != 0x14C:
            raise ValueError("%s: not an i386 object" % path)
        strtab_off = symptr + nsym * 18
        def string(off):
            end = raw.index(b"\0", strtab_off + off)
            return raw[strtab_off + off:end].decode("latin-1")
        self.symbols = []
        i = 0
        bysym = {}
        while i < nsym:
            off = symptr + i * 18
            name = raw[off:off + 8]
            if name[:4] == b"\0\0\0\0":
                name = string(struct.unpack_from("<I", name, 4)[0])
            else:
                name = name.rstrip(b"\0").decode("latin-1")
            value, secnum, typ, storage, naux = struct.unpack_from("<IhHBB", raw, off + 8)
            aux = raw[off + 18:off + 18 + naux * 18]
            sym = Symbol(i, name, value, secnum, typ, storage, aux)
            self.symbols.append(sym)
            bysym[i] = sym
            i += 1 + naux
        self.sections = []
        base = 20 + optsize
        for s in range(nsec):
            off = base + s * 40
            name = raw[off:off + 8]
            if name[:1] == b"/":
                name = string(int(name[1:].rstrip(b"\0")))
            else:
                name = name.rstrip(b"\0").decode("latin-1")
            vsize, vaddr, rawsize, rawptr, relptr, lnptr, nrel, nln, chars = struct.unpack_from("<IIIIIIHHI", raw, off + 8)
            data = raw[rawptr:rawptr + rawsize] if rawptr else b"\0" * rawsize
            relocs = []
            for r in range(nrel):
                va, si, t = struct.unpack_from("<IIH", raw, relptr + r * 10)
                relocs.append((va, si, t, self.qualified(bysym[si]) if si in bysym else "?"))
            self.sections.append(Section(s + 1, name, data, chars, relocs))
        self.functions = self._functions()

    def qualified(self, sym):
        """static symbols ($SG strings, static functions) are local to this object"""
        if sym.storage == 3 or (sym.storage != 2 and sym.section > 0):
            return "%s:%s" % (self.path.split("/")[-1], sym.name)
        return sym.name

    def section(self, index):
        return self.sections[index - 1]

    def _functions(self):
        by_sec = {}
        for sym in self.symbols:
            if sym.section <= 0 or sym.storage not in (2, 3):
                continue
            sec = self.section(sym.section)
            if not sec.is_code:
                continue
            if sym.storage == 3 and sym.value == 0 and sym.name == sec.name:
                continue
            if not sym.is_function and sym.storage == 3:
                continue
            by_sec.setdefault(sym.section, []).append(sym)
        funcs = []
        for secnum, syms in by_sec.items():
            sec = self.section(secnum)
            syms.sort(key=lambda s: s.value)
            for n, sym in enumerate(syms):
                end = syms[n + 1].value if n + 1 < len(syms) else len(sec.data)
                fn = Function(sym.name, sec, sym.value, end - sym.value, sym.storage == 3)
                fn.key = self.qualified(sym)
                funcs.append(fn)
        return funcs
