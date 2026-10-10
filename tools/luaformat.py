import struct

OPNAMES = [
    "END", "RETURN", "CALL", "TAILCALL", "PUSHNIL", "POP", "PUSHINT", "PUSHSTRING",
    "PUSHNUM", "PUSHNEGNUM", "PUSHUPVALUE", "GETLOCAL", "GETGLOBAL", "GETTABLE",
    "GETDOTTED", "GETINDEXED", "PUSHSELF", "CREATETABLE", "SETLOCAL", "SETGLOBAL",
    "SETTABLE", "SETLIST", "SETMAP", "ADD", "ADDI", "SUB", "MULT", "DIV", "POW",
    "CONCAT", "MINUS", "NOT", "JMPNE", "JMPEQ", "JMPLT", "JMPLE", "JMPGT", "JMPGE",
    "JMPT", "JMPF", "JMPONT", "JMPONF", "JMP", "PUSHNILJMP", "FORPREP", "FORLOOP",
    "LFORPREP", "LFORLOOP", "CLOSURE", "EXTARG", "OP50", "CALLX", "CLOSUREX",
]

S_OPS = {"PUSHINT", "ADDI"}
J_OPS = {"JMPNE", "JMPEQ", "JMPLT", "JMPLE", "JMPGT", "JMPGE", "JMPT", "JMPF",
         "JMPONT", "JMPONF", "JMP", "FORPREP", "FORLOOP", "LFORPREP", "LFORLOOP"}
AB_OPS = {"CALL", "TAILCALL", "SETTABLE", "SETLIST", "CLOSURE"}
COND_JUMPS = {"JMPNE", "JMPEQ", "JMPLT", "JMPLE", "JMPGT", "JMPGE", "JMPT", "JMPF",
              "JMPONT", "JMPONF"}
CMP_JUMPS = {"JMPNE", "JMPEQ", "JMPLT", "JMPLE", "JMPGT", "JMPGE"}
MAX_INT = 2147483645
MULT_RET = 31


class FormatError(Exception):
    pass


class Reader:
    def __init__(self, data, pos=0, end=None):
        self.d = data
        self.p = pos
        self.end = len(data) if end is None else end

    def raw(self, n):
        if self.p + n > self.end:
            raise FormatError("read past end at 0x%x" % self.p)
        v = self.d[self.p:self.p + n]
        self.p += n
        return v

    def byte(self):
        return self.raw(1)[0]

    def int(self):
        return struct.unpack("<i", self.raw(4))[0]

    def size(self):
        return struct.unpack("<I", self.raw(4))[0]

    def number(self):
        return struct.unpack("<f", self.raw(4))[0]

    def string(self):
        n = self.size()
        if n == 0:
            return None
        if n > self.end - self.p:
            raise FormatError("bad string length at 0x%x" % (self.p - 4))
        return self.raw(n)[:-1].decode("latin-1")

    def count(self):
        n = self.int()
        if n < 0 or n > 1000000:
            raise FormatError("bad count at 0x%x" % (self.p - 4))
        return n


HEADER = b"\x1bLua\x40\x01\x04\x04\x02\x10\x06\x05\x04"


class Proto:
    pass


def load_function(r):
    f = Proto()
    f.source = r.string()
    f.line_defined = r.int()
    f.num_params = r.int()
    f.is_vararg = r.byte()
    f.max_stack = r.int()
    f.locals = []
    for _ in range(r.count()):
        name = r.string()
        f.locals.append((name, r.int(), r.int()))
    f.lines = [r.int() for _ in range(r.count())]
    f.kstr = [r.string() for _ in range(r.count())]
    f.knum = [r.number() for _ in range(r.count())]
    f.kproto = [load_function(r) for _ in range(r.count())]
    n = r.count()
    f.raw = list(struct.unpack("<%dH" % n, r.raw(2 * n)))
    return f


def load_chunk_at(data, pos=0, end=None):
    r = Reader(data, pos, end)
    if r.raw(len(HEADER)) != HEADER:
        raise FormatError("not a Psychonauts Lua 4.0 chunk")
    r.number()
    f = load_function(r)
    return f, r.p


def load_chunk(data, pos=0, end=None):
    f, p = load_chunk_at(data, pos, end)
    real_end = len(data) if end is None else end
    if p != real_end:
        raise FormatError("%d trailing bytes" % (real_end - p))
    return f


class Ins:
    __slots__ = ("op", "a", "b", "u", "s", "tgt", "line", "pc")

    def __repr__(self):
        return "<%d %s a%d b%d u%d s%d t%s>" % (self.pc, self.op, self.a, self.b, self.u, self.s, self.tgt)


def internal_code(f):
    raw = f.raw
    old2new = []
    code = []
    i = 0
    while i < len(raw):
        w = raw[i]
        op = OPNAMES[w & 63]
        u = w >> 6
        ins = Ins()
        ins.a = w >> 11
        ins.b = u & 31
        ins.u = u
        ins.s = u - 511
        ins.tgt = None
        ins.line = None
        if op == "EXTARG":
            if i + 1 >= len(raw):
                raise FormatError("EXTARG at end")
            w2 = raw[i + 1]
            op2 = OPNAMES[w2 & 63]
            if op2 not in ("CALLX", "CLOSUREX"):
                raise FormatError("EXTARG followed by %s" % op2)
            ins.op = "CALL" if op2 == "CALLX" else "CLOSURE"
            ins.a = u
            ins.b = w2 >> 6
            old2new.append(len(code))
            old2new.append(len(code))
            i += 2
        else:
            if op in ("CALLX", "CLOSUREX", "OP50"):
                raise FormatError("unexpected %s" % op)
            ins.op = op
            old2new.append(len(code))
            i += 1
        ins.pc = len(code)
        code.append(ins)
    old2new.append(len(code))
    starts = []
    i = 0
    while i < len(raw):
        starts.append(i)
        if OPNAMES[raw[i] & 63] == "EXTARG":
            i += 2
        else:
            i += 1
    for idx, ins in enumerate(code):
        if ins.op in J_OPS:
            old = starts[idx]
            target_old = old + 1 + ins.s
            if target_old < 0 or target_old > len(raw):
                raise FormatError("jump out of range")
            ins.tgt = old2new[target_old]
    lines = None
    if f.lines:
        lines = [None] * len(code)
        cur = 0
        marks = []
        for v in f.lines:
            if v == MAX_INT:
                break
            if v < 0:
                cur += -v
            else:
                cur += 1
                marks.append((old2new[v], cur))
        line = None
        mi = 0
        for idx in range(len(code)):
            while mi < len(marks) and marks[mi][0] <= idx:
                line = marks[mi][1]
                mi += 1
            code[idx].line = line
    locals_ = [(n, old2new[s], old2new[e]) for n, s, e in f.locals]
    return code, locals_
