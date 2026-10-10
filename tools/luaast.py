import re
import struct

RESERVED = {"and", "break", "do", "else", "elseif", "end", "for", "function", "if",
            "local", "nil", "not", "or", "repeat", "return", "then", "until", "while"}
IDENT = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")

# (left, right) priorities from lparser.c, ORDER OPR
BINPRI = {
    "+": (5, 5), "-": (5, 5), "*": (6, 6), "/": (6, 6), "^": (9, 8), "..": (4, 3),
    "~=": (2, 2), "==": (2, 2), "<": (2, 2), "<=": (2, 2), ">": (2, 2), ">=": (2, 2),
    "and": (1, 1), "or": (1, 1),
}
UNARY_PRIORITY = 7
CMP_INVERT = {"==": "~=", "~=": "==", "<": ">=", ">=": "<", "<=": ">", ">": "<="}


def is_name(s):
    return s is not None and IDENT.match(s) is not None and s not in RESERVED


def f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def number_text(v):
    if v == int(v) and abs(v) < 1e15:
        return str(int(v))
    for digits in range(1, 18):
        s = "%.*g" % (digits, v)
        if f32(float(s)) == v:
            if "e" in s:
                mant, exp = s.split("e")
                s = mant + "e" + str(int(exp))
            return s
    return repr(v)


def string_text(s):
    out = ['"']
    for ch in s:
        o = ord(ch)
        if ch == '"':
            out.append('\\"')
        elif ch == "\\":
            out.append("\\\\")
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\t":
            out.append("\\t")
        elif ch == "\r":
            out.append("\\r")
        elif o < 32 or o == 127:
            out.append("\\%d" % o)
        else:
            out.append(ch)
    out.append('"')
    return "".join(out)


class Node:
    line = None


class Expr(Node):
    pri = 100

    def lines(self):
        return [self.line] if self.line is not None else []


class Nil(Expr):
    def __init__(self, group=None):
        self.group = group


class Num(Expr):
    def __init__(self, v):
        self.v = v


class Str(Expr):
    def __init__(self, s):
        self.s = s


class Global(Expr):
    def __init__(self, name):
        self.name = name


class LocalVar:
    def __init__(self, name, slot):
        self.name = name
        self.slot = slot


class LocalRef(Expr):
    def __init__(self, var):
        self.var = var


class Upval(Expr):
    def __init__(self, name):
        self.name = name


class Index(Expr):
    def __init__(self, t, k):
        self.t = t
        self.k = k


class MethodFn(Expr):
    def __init__(self, obj, name):
        self.obj = obj
        self.name = name


class SelfMark(Expr):
    pass


class Call(Expr):
    def __init__(self, fn, args, nres):
        self.fn = fn
        self.args = args
        self.nres = nres


class MultiRes(Expr):
    def __init__(self, call):
        self.call = call


class Closure(Expr):
    def __init__(self, func, upvals):
        self.func = func
        self.upvals = upvals
        self.end_line = None


class Table(Expr):
    def __init__(self, size):
        self.size = size
        self.parts = []

    def copy(self):
        t = Table(self.size)
        t.parts = [(k, list(items)) for k, items in self.parts]
        t.line = self.line
        t.close_line = getattr(self, "close_line", None)
        return t


class Bin(Expr):
    def __init__(self, op, a, b):
        self.op = op
        self.a = a
        self.b = b


class Not(Expr):
    def __init__(self, a):
        self.a = a


class Unm(Expr):
    def __init__(self, a):
        self.a = a


class Concat(Expr):
    def __init__(self, items):
        self.items = items


class Garbage(Expr):
    pass


class ForceJump(Expr):
    def __init__(self, a, positive):
        self.a = a
        self.positive = positive


def negate(e):
    if isinstance(e, Bin) and e.op in CMP_INVERT:
        n = Bin(CMP_INVERT[e.op], e.a, e.b)
        n.line = e.line
        return n
    if isinstance(e, Not):
        return e.a
    n = Not(e)
    n.line = e.line
    return n


class Stmt(Node):
    pass


class LocalStmt(Stmt):
    def __init__(self, vars_, exprs):
        self.vars = vars_
        self.exprs = exprs


class Assign(Stmt):
    def __init__(self, targets, exprs):
        self.targets = targets
        self.exprs = exprs


class CallStmt(Stmt):
    def __init__(self, call):
        self.call = call


class Return(Stmt):
    def __init__(self, exprs):
        self.exprs = exprs


class Break(Stmt):
    pass


class If(Stmt):
    def __init__(self, cond, then, orelse):
        self.cond = cond
        self.then = then
        self.orelse = orelse


class While(Stmt):
    def __init__(self, cond, body):
        self.cond = cond
        self.body = body


class Repeat(Stmt):
    def __init__(self, body, cond):
        self.body = body
        self.cond = cond


class ForNum(Stmt):
    def __init__(self, var, start, limit, step, body):
        self.var = var
        self.start = start
        self.limit = limit
        self.step = step
        self.body = body


class ForIn(Stmt):
    def __init__(self, kvar, vvar, table, body):
        self.kvar = kvar
        self.vvar = vvar
        self.table = table
        self.body = body


class Do(Stmt):
    def __init__(self, body):
        self.body = body


class ScopeEnd(Stmt):
    def __init__(self, vars_):
        self.vars = vars_


class Func:
    def __init__(self, proto, params, vararg, body, upnames):
        self.proto = proto
        self.params = params
        self.vararg = vararg
        self.body = body
        self.upnames = upnames
        self.line_defined = proto.line_defined


class Printer:
    def __init__(self, debug_lines):
        self.debug = debug_lines
        self.out = []
        self.line = 1
        self.col0 = True
        self.indent = 0
        self.targets = []

    def goto(self, line):
        if line is None:
            return
        while self.line < line:
            self.out.append("\n")
            self.line += 1
            self.col0 = True

    def soft_newline(self):
        if self.debug:
            return
        nxt = self.targets[0] if self.targets else None
        if nxt is None or self.line + 1 <= nxt:
            if not self.col0:
                self.out.append("\n")
                self.line += 1
                self.col0 = True

    def w(self, text):
        if self.col0:
            self.out.append("  " * self.indent)
            self.col0 = False
        elif self.out and not self.out[-1].endswith((" ", "\n", "(", "[", "{")) and (text.startswith("..") or not text.startswith((")", "]", "}", ",", ";", ".", ":"))):
            self.out.append(" ")
        self.out.append(text)

    def wraw(self, text):
        if self.col0:
            self.out.append("  " * self.indent)
            self.col0 = False
        self.out.append(text)

    def text(self):
        return "".join(self.out) + "\n"

    def block(self, stmts):
        n = len(stmts)
        for i, s in enumerate(stmts):
            if isinstance(s, (Return, Break)) and i < n - 1:
                d = Do([s])
                d.line = s.line
                s = d
            if self.debug:
                self.goto(getattr(s, "line", None))
            else:
                fl = first_closure_line(s)
                if fl is not None and fl > self.line:
                    self.goto(fl)
                else:
                    self.soft_newline()
            self.stmt(s)

    def names(self, vars_):
        return ", ".join(v.name for v in vars_)

    def explist(self, exprs):
        first = True
        for e in exprs:
            if isinstance(e, MultiRes):
                continue
            if not first:
                self.wraw(", ")
            self.expr(e, -1)
            first = False

    def stmt(self, s):
        if isinstance(s, LocalStmt):
            self.w("local " + self.names(s.vars))
            exprs = [e for e in s.exprs if not isinstance(e, MultiRes)]
            if exprs and not all(isinstance(e, Nil) for e in exprs):
                self.w("=")
                self.explist(s.exprs)
        elif isinstance(s, Assign):
            if len(s.targets) == 1 and isinstance(s.exprs[0], Closure) and len(s.exprs) == 1 and self.funcname(s.targets[0]) is not None:
                self.function_stmt(s.targets[0], s.exprs[0])
                return
            first = True
            for t in s.targets:
                if not first:
                    self.wraw(", ")
                self.expr(t, -1)
                first = False
            self.w("=")
            self.explist(s.exprs)
        elif isinstance(s, CallStmt):
            self.expr(s.call, -1)
        elif isinstance(s, Return):
            self.w("return")
            if s.exprs:
                self.explist(s.exprs)
        elif isinstance(s, Break):
            self.w("break")
        elif isinstance(s, If):
            self.w("if")
            self.expr(s.cond, -1)
            self.w("then")
            self.indent += 1
            self.block(s.then)
            self.indent -= 1
            orelse = s.orelse
            while orelse is not None:
                if len(orelse) == 1 and isinstance(orelse[0], If) and not self.debug:
                    s2 = orelse[0]
                    self.soft_newline()
                    self.w("elseif")
                    self.expr(s2.cond, -1)
                    self.w("then")
                    self.indent += 1
                    self.block(s2.then)
                    self.indent -= 1
                    orelse = s2.orelse
                    continue
                self.soft_newline()
                self.w("else")
                self.indent += 1
                self.block(orelse)
                self.indent -= 1
                break
            self.soft_newline()
            if self.debug:
                self.goto(getattr(s, "end_line", None))
            self.w("end")
        elif isinstance(s, While):
            self.w("while")
            if s.cond is None:
                self.w("1")
            else:
                self.expr(s.cond, -1)
            self.w("do")
            self.indent += 1
            self.block(s.body)
            self.indent -= 1
            self.soft_newline()
            self.w("end")
        elif isinstance(s, Repeat):
            self.w("repeat")
            self.indent += 1
            self.block(s.body)
            self.indent -= 1
            self.soft_newline()
            self.w("until")
            self.expr(s.cond, -1)
        elif isinstance(s, ForNum):
            self.w("for " + s.var.name + " =")
            self.expr(s.start, -1)
            self.wraw(", ")
            self.expr(s.limit, -1)
            if s.step is not None:
                self.wraw(", ")
                self.expr(s.step, -1)
            self.w("do")
            self.indent += 1
            self.block(s.body)
            self.indent -= 1
            self.soft_newline()
            self.w("end")
        elif isinstance(s, ForIn):
            self.w("for %s, %s in" % (s.kvar.name, s.vvar.name))
            self.expr(s.table, -1)
            self.w("do")
            self.indent += 1
            self.block(s.body)
            self.indent -= 1
            self.soft_newline()
            self.w("end")
        elif isinstance(s, Do):
            self.w("do")
            self.indent += 1
            self.block(s.body)
            self.indent -= 1
            self.soft_newline()
            self.w("end")
        else:
            raise ValueError("cannot print %r" % s)

    def funcname(self, t):
        if isinstance(t, Global) and is_name(t.name):
            return t.name
        if isinstance(t, Index) and isinstance(t.t, Global) and is_name(t.t.name) and isinstance(t.k, Str) and is_name(t.k.s):
            return t.t.name + "." + t.k.s
        return None

    def function_stmt(self, target, clo):
        func = clo.func
        name = self.funcname(target)
        self.targets_pop(func)
        self.goto_def(func)
        params = [p.name for p in func.params]
        if func.vararg:
            params.append("...")
        self.w("function " + name)
        self.wraw("(" + ", ".join(params) + ")")
        self.func_body(func, clo)

    def targets_pop(self, func):
        if self.targets and self.targets[0] == func.line_defined:
            self.targets.pop(0)

    def goto_def(self, func):
        self.goto(func.line_defined)

    def func_body(self, func, clo):
        self.indent += 1
        self.block(func.body)
        self.indent -= 1
        if self.debug:
            self.goto(clo.end_line)
        else:
            self.soft_newline()
        self.w("end")

    def expr(self, e, limit):
        need = self.needs_parens(e, limit)
        if need:
            self.w("(")
        self.expr_inner(e)
        if need:
            self.wraw(")")

    def needs_parens(self, e, limit):
        if limit < 0:
            return False
        if isinstance(e, Bin):
            return BINPRI[e.op][0] <= limit
        if isinstance(e, Concat):
            return BINPRI[".."][0] <= limit
        return False

    def expr_inner(self, e):
        if isinstance(e, Nil):
            self.w("nil")
        elif isinstance(e, Num):
            self.w(number_text(e.v))
        elif isinstance(e, Str):
            self.w(string_text(e.s))
        elif isinstance(e, Global):
            self.w(e.name)
        elif isinstance(e, LocalRef):
            self.w(e.var.name)
        elif isinstance(e, Upval):
            ref = e.name
            self.w("%" + (ref.name if isinstance(ref, LocalVar) else ref))
        elif isinstance(e, Index):
            self.prefix(e.t)
            if isinstance(e.k, Str) and is_name(e.k.s):
                self.wraw("." + e.k.s)
            else:
                self.wraw("[")
                self.expr(e.k, -1)
                self.wraw("]")
        elif isinstance(e, Call):
            if isinstance(e.fn, MethodFn):
                self.prefix(e.fn.obj)
                self.wraw(":" + e.fn.name)
            else:
                self.prefix(e.fn)
            self.wraw("(")
            first = True
            for a in e.args:
                if isinstance(a, MultiRes):
                    continue
                if not first:
                    self.wraw(", ")
                self.expr(a, -1)
                first = False
            self.wraw(")")
        elif isinstance(e, Closure):
            func = e.func
            self.targets_pop(func)
            self.goto_def(func)
            params = [p.name for p in func.params]
            if func.vararg:
                params.append("...")
            self.w("function")
            self.wraw("(" + ", ".join(params) + ")")
            self.func_body(func, e)
        elif isinstance(e, Table):
            self.table(e)
        elif isinstance(e, Bin):
            l, r = BINPRI[e.op]
            self.binop_left(e.a, l)
            self.w(e.op)
            self.expr(e.b, r)
        elif isinstance(e, Concat):
            n = len(e.items)
            for i, it in enumerate(e.items):
                if i:
                    self.w("..")
                if i < n - 1:
                    self.binop_left(it, 4, concat_chain=True)
                else:
                    self.expr(it, 3)
        elif isinstance(e, Not):
            self.w("not")
            self.unary_operand(e.a)
        elif isinstance(e, Unm):
            self.w("-")
            self.unary_operand(e.a)
        elif isinstance(e, ForceJump):
            if e.positive:
                self.binop_left(e.a, 2)
                self.w("~= nil")
            else:
                self.w("not (")
                self.binop_left(e.a, 2)
                self.w("~= nil")
                self.wraw(")")
        else:
            raise ValueError("cannot print %r" % e)

    def unary_operand(self, a):
        if isinstance(a, (Bin, Concat)):
            op = a.op if isinstance(a, Bin) else ".."
            l, r = BINPRI[op]
            if l <= UNARY_PRIORITY:
                self.w("(")
                self.expr_inner(a)
                self.wraw(")")
                return
        if isinstance(a, (Num,)) and a.v < 0:
            self.w("(")
            self.expr_inner(a)
            self.wraw(")")
            return
        self.expr_inner(a)

    def binop_left(self, a, opleft, concat_chain=False):
        if isinstance(a, Bin):
            l2, r2 = BINPRI[a.op]
            if opleft > r2:
                self.w("(")
                self.expr_inner(a)
                self.wraw(")")
                return
        elif isinstance(a, Concat):
            if opleft > 3 or concat_chain:
                self.w("(")
                self.expr_inner(a)
                self.wraw(")")
                return
        elif isinstance(a, (Not, Unm)):
            if opleft > UNARY_PRIORITY:
                self.w("(")
                self.expr_inner(a)
                self.wraw(")")
                return
        self.expr_inner(a)

    def prefix(self, e):
        if isinstance(e, (Global, LocalRef, Upval, Index, Call)):
            self.expr_inner(e)
        else:
            self.w("(")
            self.expr_inner(e)
            self.wraw(")")

    def table(self, t):
        self.w("{")
        first_part = True
        for kind, items in t.parts:
            if not first_part:
                self.wraw("; ")
            first_part = False
            first = True
            if kind == "list":
                for it in items:
                    if not first:
                        self.wraw(", ")
                    if self.debug:
                        self.goto(it.line)
                    self.expr(it, -1)
                    first = False
            else:
                for k, v in items:
                    if not first:
                        self.wraw(", ")
                    if self.debug:
                        self.goto(k.line)
                    if isinstance(k, Str) and is_name(k.s):
                        self.w(k.s)
                    else:
                        self.w("[")
                        self.expr(k, -1)
                        self.wraw("]")
                    self.w("=")
                    self.expr(v, -1)
                    first = False
        if self.debug:
            self.goto(getattr(t, "close_line", None))
        self.wraw("}") if not self.col0 else self.w("}")


def first_closure_line(node):
    found = []

    def visit(x):
        if found or x is None:
            return
        if isinstance(x, Closure):
            found.append(x.func.line_defined)
            return
        if isinstance(x, (list, tuple)):
            for y in x:
                visit(y)
            return
        if not isinstance(x, (Node, Func)):
            return
        if isinstance(x, (If, While, Repeat, ForNum, ForIn, Do)):
            for name in ("cond", "start", "limit", "step", "table"):
                visit(getattr(x, name, None))
            return
        for name in ("targets", "exprs", "call", "fn", "obj", "args", "t", "k", "a", "b", "items"):
            visit(getattr(x, name, None))
        parts = getattr(x, "parts", None)
        if parts:
            for kind, items in parts:
                for it in items:
                    visit(it)

    visit(node)
    return found[0] if found else None
