from luaast import (Assign, Bin, Call, Closure, Concat, Do, ForIn, ForNum, Global, If, Index,
                    LocalStmt, LocalVar, MethodFn, Not, Repeat, Return, CallStmt, Table, Unm,
                    Upval, While, ForceJump, Func)


def walk_stmts(stmts, on_stmt, on_expr):
    for s in stmts:
        on_stmt(s)
        for name in ("targets", "exprs"):
            for x in getattr(s, name, None) or []:
                walk_expr(x, on_expr, s)
        for name in ("call", "cond", "start", "limit", "step", "table"):
            x = getattr(s, name, None)
            if x is not None and not isinstance(x, list):
                walk_expr(x, on_expr, s)
        for name in ("then", "orelse", "body"):
            b = getattr(s, name, None)
            if b:
                walk_stmts(b, on_stmt, on_expr)


def walk_expr(e, on_expr, stmt):
    if e is None:
        return
    on_expr(e, stmt)
    if isinstance(e, Closure):
        return
    for name in ("a", "b", "t", "k", "fn", "obj"):
        x = getattr(e, name, None)
        if x is not None and hasattr(x, "line"):
            walk_expr(x, on_expr, stmt)
    for name in ("args", "items"):
        for x in getattr(e, name, None) or []:
            walk_expr(x, on_expr, stmt)
    for kind, items in getattr(e, "parts", None) or []:
        for it in items:
            if kind == "list":
                walk_expr(it, on_expr, stmt)
            else:
                walk_expr(it[0], on_expr, stmt)
                walk_expr(it[1], on_expr, stmt)


class Info:
    def __init__(self, func):
        self.func = func
        self.locals = []
        self.loops_in = []
        self.loops_num = []
        self.children = []
        self.globals = set()

        def on_stmt(s):
            if isinstance(s, LocalStmt):
                self.locals.extend(s.vars)
            elif isinstance(s, ForIn):
                self.loops_in.append((s.kvar, s.vvar))
            elif isinstance(s, ForNum):
                self.loops_num.append(s.var)

        def on_expr(e, stmt):
            if isinstance(e, Global):
                self.globals.add(e.name)
            elif isinstance(e, Closure):
                role = "function"
                if isinstance(stmt, Assign) and len(stmt.exprs) == 1 and stmt.exprs[0] is e:
                    if len(stmt.targets) == 1 and isinstance(stmt.targets[0], Index):
                        role = "method"
                self.children.append((e.func, role))

        walk_stmts(func.body, on_stmt, on_expr)


def collect_globals(func, out):
    info = Info(func)
    out.update(info.globals)
    for ref in func.upnames:
        if not isinstance(ref, LocalVar):
            out.add(ref)
    for child, _ in info.children:
        collect_globals(child, out)


def snapshot(func, out):
    info = Info(func)
    for v in list(func.params) + info.locals:
        out.append((v, v.name))
    for k, v in info.loops_in:
        out.append((k, k.name))
        out.append((v, v.name))
    for v in info.loops_num:
        out.append((v, v.name))
    for child, _ in info.children:
        snapshot(child, out)


def restore(saved):
    for var, name in saved:
        var.name = name


def rename(func, role, avoid):
    info = Info(func)
    if func.proto.lines:
        for child, crole in info.children:
            rename(child, crole, avoid)
        return
    upvalue_users = set()
    child_globals = set()
    for child, _ in info.children:
        for ref in child.upnames:
            if isinstance(ref, LocalVar):
                upvalue_users.add(ref)
            else:
                child_globals.add(ref)
    blocked = set(avoid) | child_globals
    for ref in func.upnames:
        blocked.add(ref.name if isinstance(ref, LocalVar) else ref)
    taken = set()
    if func.vararg:
        taken.add("arg")

    def pick(base, numbered=False):
        n = 1 if numbered else None
        while True:
            name = "%s%d" % (base, n) if n is not None else base
            if name not in taken and name not in blocked:
                taken.add(name)
                return name
            n = 2 if n is None else n + 1

    for i, p in enumerate(func.params):
        if i == 0 and role == "method":
            p.name = pick("self")
        elif i == 0 and p in upvalue_users:
            p.name = pick("Ob")
        else:
            p.name = pick("a", numbered=True)
    for k, v in info.loops_in:
        k.name = pick("k")
        v.name = pick("v")
    for v in info.loops_num:
        v.name = pick("i")
    for v in info.locals:
        v.name = pick("l", numbered=True)
    for child, crole in info.children:
        rename(child, crole, avoid)


def apply(func):
    saved = []
    snapshot(func, saved)
    avoid = set()
    collect_globals(func, avoid)
    rename(func, "function", avoid)
    return saved
