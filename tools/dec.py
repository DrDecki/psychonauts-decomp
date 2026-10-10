from luaast import *
from luaformat import internal_code, COND_JUMPS, CMP_JUMPS, MULT_RET

CMP_OP = {"JMPNE": "~=", "JMPEQ": "==", "JMPLT": "<", "JMPLE": "<=", "JMPGT": ">", "JMPGE": ">="}
EXPR_OPS = {"PUSHNIL", "PUSHINT", "PUSHSTRING", "PUSHNUM", "PUSHNEGNUM", "PUSHUPVALUE",
            "GETLOCAL", "GETGLOBAL", "GETTABLE", "GETDOTTED", "GETINDEXED", "PUSHSELF",
            "CREATETABLE", "SETLIST", "SETMAP", "ADD", "ADDI", "SUB", "MULT", "DIV", "POW",
            "CONCAT", "MINUS", "NOT", "CLOSURE"}
STORE_OPS = {"SETLOCAL", "SETGLOBAL", "SETTABLE"}
ARITH = {"ADD": "+", "SUB": "-", "MULT": "*", "DIV": "/", "POW": "^"}


class DecompileError(Exception):
    pass


class Abort(Exception):
    pass


class Loop:
    def __init__(self, exit_pc, nloc):
        self.exit = exit_pc
        self.nloc = nloc


class FuncDecompiler:
    def __init__(self, proto, upnames, ctx):
        self.f = proto
        self.ctx = ctx
        self.upnames = upnames
        self.code, self.debug_locals = internal_code(proto)
        self.debug = bool(proto.lines)
        self.stack = []
        self.nloc = 0
        self.vars = []
        self.out = None
        self.sandbox = 0
        self.local_counter = 0
        self.pending_extras = []
        self.active_loops = set()
        self.rejected_heads = {}
        self.back_jumps = {}
        for ins in self.code:
            if ins.tgt is not None and ins.tgt <= ins.pc and ins.op not in ("FORLOOP", "LFORLOOP"):
                self.back_jumps.setdefault(ins.tgt, []).append(ins.pc)

    # ------------------------------------------------------------ names

    def debug_name(self, slot, pc):
        if not self.debug_locals:
            return None
        i = 0
        for name, start, end in self.debug_locals:
            if start > pc:
                break
            if pc < end:
                if i == slot:
                    return name
                i += 1
        return None

    def debug_name_at_start(self, slot, startpc):
        active = []
        for name, start, end in self.debug_locals:
            if start <= startpc and startpc < end:
                active.append(name)
        if slot < len(active):
            return active[slot]
        return None

    def new_name(self):
        return self.ctx.fresh("l")

    def make_var(self, slot, startpc):
        name = None
        if self.debug_locals:
            name = self.debug_name_at_start(slot, startpc)
        if name is None:
            name = self.new_name()
        return LocalVar(name, slot)

    # ------------------------------------------------------------ helpers

    def push(self, e, ins):
        if ins is not None and e.line is None:
            e.line = ins.line
        self.stack.append(e)

    def pop(self):
        if len(self.stack) <= self.nloc:
            raise DecompileError("pop below locals")
        return self.stack.pop()

    def first_line(self, entries):
        lines = [e.line for e in entries if e is not None and e.line is not None]
        return min(lines) if lines else None

    def declare(self, upto, startpc=None):
        if upto <= self.nloc:
            return
        if self.sandbox:
            raise Abort()
        if upto > len(self.stack):
            raise DecompileError("declare beyond stack")
        while upto < len(self.stack) and isinstance(self.stack[upto], MultiRes):
            upto += 1
        entries = self.stack[self.nloc:upto]
        for e in entries:
            if isinstance(e, (Garbage, SelfMark, MethodFn)):
                raise DecompileError("garbage declared as local")
        groups = []
        i = 0
        while i < len(entries):
            e = entries[i]
            g = [e]
            j = i + 1
            if isinstance(e, Call):
                while j < len(entries) and isinstance(entries[j], MultiRes) and entries[j].call is e:
                    g.append(entries[j])
                    j += 1
            elif isinstance(e, Nil) and e.group is not None:
                while j < len(entries) and isinstance(entries[j], Nil) and entries[j].group is e.group:
                    g.append(entries[j])
                    j += 1
            groups.append(g)
            i = j
        slot = self.nloc
        for g in groups:
            vars_ = []
            for e in g:
                v = self.make_var(slot, startpc if startpc is not None else 0)
                self.vars.append(v)
                vars_.append(v)
                self.stack[slot] = LocalRef(v)
                slot += 1
            st = LocalStmt(vars_, g)
            st.line = self.first_line(g)
            self.out.append(st)
        self.nloc = upto

    def declare_debug(self, pc):
        if not self.debug_locals or self.sandbox:
            return
        starting = [l for l in self.debug_locals if l[1] == pc]
        if not starting:
            return
        n = 0
        for name, start, end in self.debug_locals:
            if start <= pc < end:
                n += 1
        if n > self.nloc and n <= len(self.stack):
            entries = self.stack[self.nloc:n]
            if all(not isinstance(e, (Garbage, SelfMark, MethodFn, LocalRef)) for e in entries):
                vars_ = []
                for slot in range(self.nloc, n):
                    v = LocalVar(self.debug_name(slot, pc) or self.new_name(), slot)
                    self.vars.append(v)
                    vars_.append(v)
                    self.stack[slot] = LocalRef(v)
                st = LocalStmt(vars_, entries)
                st.line = self.first_line(entries)
                self.out.append(st)
                self.nloc = n

    def local_ref(self, slot, pc):
        if slot >= self.nloc:
            self.declare(slot + 1, pc)
        e = self.stack[slot]
        if not isinstance(e, LocalRef):
            raise DecompileError("slot %d is not a local" % slot)
        return LocalRef(e.var)

    def dest(self, x):
        if x is None:
            return None
        seen = 0
        while x < len(self.code) and self.code[x].op == "JMP" and seen < 64:
            x = self.code[x].tgt
            seen += 1
        return x

    def same_dest(self, a, b):
        return self.dest(a) == self.dest(b)

    def find_loop_end(self, prep, op):
        for j in range(prep + 1, len(self.code)):
            ins = self.code[j]
            if ins.op == op and ins.tgt == prep + 1:
                return j
        raise DecompileError("loop end not found")

    def kstr(self, i):
        return self.f.kstr[i]

    # ------------------------------------------------------------ expression step

    def step(self, pc):
        ins = self.code[pc]
        op = ins.op
        if op == "PUSHNIL":
            group = object() if ins.u > 1 else None
            for _ in range(ins.u):
                self.push(Nil(group), ins)
        elif op == "PUSHINT":
            self.push(Num(ins.s), ins)
        elif op == "PUSHNUM":
            self.push(Num(self.f.knum[ins.u]), ins)
        elif op == "PUSHNEGNUM":
            self.push(Num(-self.f.knum[ins.u]), ins)
        elif op == "PUSHSTRING":
            self.push(Str(self.kstr(ins.u)), ins)
        elif op == "PUSHUPVALUE":
            if ins.u >= len(self.upnames):
                raise DecompileError("bad upvalue")
            self.push(Upval(self.upnames[ins.u]), ins)
        elif op == "GETLOCAL":
            self.push(self.local_ref(ins.u, pc), ins)
        elif op == "GETGLOBAL":
            self.push(Global(self.kstr(ins.u)), ins)
        elif op == "GETTABLE":
            k = self.pop()
            t = self.pop()
            e = Index(t, k)
            e.line = t.line
            self.push(e, ins)
        elif op == "GETDOTTED":
            t = self.pop()
            k = Str(self.kstr(ins.u))
            k.line = ins.line
            e = Index(t, k)
            e.line = t.line
            self.push(e, ins)
        elif op == "GETINDEXED":
            t = self.pop()
            k = self.local_ref(ins.u, pc)
            e = Index(t, k)
            e.line = t.line
            self.push(e, ins)
        elif op == "PUSHSELF":
            obj = self.pop()
            m = MethodFn(obj, self.kstr(ins.u))
            m.line = obj.line
            self.push(m, ins)
            self.push(SelfMark(), ins)
        elif op == "CREATETABLE":
            self.push(Table(ins.u), ins)
        elif op == "SETLIST":
            n = ins.b
            items = self.stack[len(self.stack) - n:]
            del self.stack[len(self.stack) - n:]
            t = self.stack[-1]
            if not isinstance(t, Table):
                raise DecompileError("SETLIST without table")
            t = t.copy()
            if t.parts and t.parts[-1][0] == "list":
                t.parts[-1][1].extend(items)
            else:
                t.parts.append(("list", list(items)))
            self.stack[-1] = t
        elif op == "SETMAP":
            n = ins.u * 2
            items = self.stack[len(self.stack) - n:]
            del self.stack[len(self.stack) - n:]
            t = self.stack[-1]
            if not isinstance(t, Table):
                raise DecompileError("SETMAP without table")
            t = t.copy()
            pairs = [(items[i], items[i + 1]) for i in range(0, n, 2)]
            if t.parts and t.parts[-1][0] == "rec":
                t.parts[-1][1].extend(pairs)
            else:
                t.parts.append(("rec", pairs))
            self.stack[-1] = t
        elif op in ARITH:
            b = self.pop()
            a = self.pop()
            e = Bin(ARITH[op], a, b)
            e.line = a.line
            self.push(e, ins)
        elif op == "ADDI":
            a = self.pop()
            if ins.s < 0:
                e = Bin("-", a, Num(-ins.s))
            else:
                e = Bin("+", a, Num(ins.s))
            e.line = a.line
            self.push(e, ins)
        elif op == "CONCAT":
            n = ins.u
            items = self.stack[len(self.stack) - n:]
            if len(items) != n or len(self.stack) - n < self.nloc:
                raise DecompileError("bad CONCAT")
            del self.stack[len(self.stack) - n:]
            e = Concat(list(items))
            e.line = items[0].line
            self.push(e, ins)
        elif op == "MINUS":
            a = self.pop()
            if isinstance(a, Num):
                e = Num(-a.v)
            else:
                e = Unm(a)
            e.line = a.line
            self.push(e, ins)
        elif op == "NOT":
            a = self.pop()
            e = Not(a)
            e.line = a.line
            self.push(e, ins)
        elif op == "CLOSURE":
            nup = ins.b
            ups = self.stack[len(self.stack) - nup:] if nup else []
            if len(self.stack) - nup < self.nloc:
                raise DecompileError("closure upvalues below locals")
            if nup:
                del self.stack[len(self.stack) - nup:]
            names = []
            for u in ups:
                if isinstance(u, LocalRef):
                    names.append(u.var)
                elif isinstance(u, Global):
                    names.append(u.name)
                else:
                    raise DecompileError("bad upvalue expression")
            if ins.a >= len(self.f.kproto):
                raise DecompileError("bad closure index")
            func = self.ctx.decompile(self.f.kproto[ins.a], names)
            c = Closure(func, ups)
            c.end_line = ups[0].line if ups and ups[0].line is not None else ins.line
            c.line = func.line_defined
            self.push(c, ins)
        else:
            raise DecompileError("step on %s" % op)
        return pc + 1

    # ------------------------------------------------------------ calls

    def build_call(self, a, nres):
        if a < self.nloc or a >= len(self.stack):
            raise DecompileError("bad call base")
        fn = self.stack[a]
        if isinstance(fn, MethodFn):
            args = self.stack[a + 2:]
        else:
            args = self.stack[a + 1:]
        for x in args:
            if isinstance(x, (Garbage, SelfMark, MethodFn)):
                raise DecompileError("bad call argument")
        call = Call(fn, list(args), nres)
        call.line = fn.line
        del self.stack[a:]
        return call

    # ------------------------------------------------------------ value chains

    def is_value_jump(self, pc, depth=0):
        ins = self.code[pc]
        if ins.op in ("JMPONT", "JMPONF"):
            return True
        if depth > 20:
            return False
        t = ins.tgt
        if t is None or t <= pc or t >= len(self.code):
            return False
        c = self.code
        if c[t].op == "PUSHNILJMP":
            return True
        if c[t].op == "PUSHINT" and c[t].s == 1 and t > 0 and c[t - 1].op == "PUSHNILJMP":
            return True
        prev = c[t - 1]
        if prev.op in ("JMPONT", "JMPONF"):
            return True
        if prev.op in COND_JUMPS and prev.pc != pc and prev.tgt is not None and prev.tgt > prev.pc:
            return self.is_value_jump(prev.pc, depth + 1)
        if prev.op == "JMP" and prev.tgt is not None and prev.tgt > t:
            x = prev.tgt
            if x >= 2 and c[x - 1].op == "PUSHINT" and c[x - 2].op == "PUSHNILJMP":
                return True
        return False

    def chain_final(self, jpc):
        c = self.code
        maxt = c[jpc].tgt
        q = jpc + 1
        while True:
            if q >= len(c):
                raise DecompileError("value chain runs off end")
            if q == maxt:
                if c[q].op == "PUSHNILJMP":
                    return q + 2
                if c[q].op == "PUSHINT" and c[q].s == 1 and c[q - 1].op == "PUSHNILJMP":
                    return q + 1
                return q
            ins = c[q]
            if ins.tgt is not None and ins.op not in ("FORLOOP", "LFORLOOP"):
                if ins.tgt > maxt:
                    maxt = ins.tgt
            q += 1
            if q > maxt:
                raise DecompileError("value chain overshoot")

    def atom_from_jump(self, ins):
        if ins.op in CMP_JUMPS:
            b = self.pop()
            a = self.pop()
            e = Bin(CMP_OP[ins.op], a, b)
            e.line = a.line
            return e, True
        x = self.pop()
        return x, ins.op in ("JMPT", "JMPONT")

    def value_chain(self, jpc):
        ins = self.code[jpc]
        nconsume = 2 if ins.op in CMP_JUMPS else 1
        h0 = len(self.stack) - nconsume
        if h0 < self.nloc:
            raise DecompileError("value chain below locals")
        final = self.chain_final(jpc)
        items = []
        starts = []
        e, jif = self.atom_from_jump(ins)
        items.append(("atom", e, jif, ins))
        starts.append(None)
        pc = jpc + 1
        tail_nil = None
        tail_one = None
        leaf = None
        c = self.code
        if c[final - 1].op == "PUSHINT" and final >= 2 and c[final - 2].op == "PUSHNILJMP":
            tail_one = final - 1
            tail_nil = final - 2
        stop = tail_nil if tail_nil is not None else final
        while pc < stop:
            ins = c[pc]
            if ins.op == "JMP" and tail_nil is not None and ins.tgt == final:
                if len(self.stack) != h0 + 1:
                    raise DecompileError("value chain leaf mismatch")
                leaf = self.stack.pop()
                items.append(("leaf", leaf, None, None))
                pc += 1
                if c[pc].op == "POP" and c[pc].u == 1:
                    pc += 1
                if pc != tail_nil:
                    raise DecompileError("bad value tail")
                break
            if len(self.stack) == h0:
                starts.append(pc)
            if ins.op in COND_JUMPS:
                n = 2 if ins.op in CMP_JUMPS else 1
                if len(self.stack) - n == h0:
                    e, jif = self.atom_from_jump(ins)
                    items.append(("atom", e, jif, ins))
                    pc += 1
                    continue
                if len(self.stack) - n < h0:
                    raise DecompileError("value chain stack underflow")
                pc = self.value_chain(pc)
                continue
            if ins.op == "CALL":
                pc = self.exec_call(pc)
                continue
            if ins.op in EXPR_OPS:
                pc = self.step(pc)
                continue
            raise DecompileError("unexpected %s in value chain" % ins.op)
        if leaf is None and tail_nil is None:
            if len(self.stack) != h0 + 1:
                raise DecompileError("value chain end mismatch %d %d" % (len(self.stack), h0 + 1))
            leaf = self.stack.pop()
            items.append(("leaf", leaf, None, None))
        if len(starts) != len(items):
            raise DecompileError("value chain start bookkeeping")

        def label(it):
            ins = it[3]
            t = ins.tgt
            if t == final and ins.op in ("JMPONT", "JMPONF"):
                return "T" if ins.op == "JMPONT" else "F"
            if tail_one is not None and t == tail_one:
                return "T"
            if tail_nil is not None and t == tail_nil:
                return "F"
            if t in starts:
                return ("S", starts.index(t))
            return ("X", t)

        labels = [label(it) if it[0] == "atom" else None for it in items]
        expr = self.build(items, labels, 0, len(items), "T", "F")
        if expr is None:
            raise DecompileError("cannot rebuild value chain")
        if items[-1][0] == "atom" and tail_nil is not None:
            expr = self.force_last(expr, items[-1])
        self.push(expr, None)
        expr.line = items[0][1].line
        return final

    def force_last(self, expr, last):
        e = last[1]
        if isinstance(e, Bin) and e.op in CMP_INVERT:
            return expr
        return self.replace_leaf(expr, e)

    def replace_leaf(self, expr, target):
        if expr is target:
            return ForceJump(target, True)
        if isinstance(expr, Not) and expr.a is target:
            return ForceJump(target, False)
        if isinstance(expr, Bin) and expr.op in ("and", "or"):
            return Bin(expr.op, expr.a, self.replace_leaf(expr.b, target))
        return expr

    def build(self, items, labels, lo, hi, T, F):
        if hi - lo == 1:
            it = items[lo]
            if it[0] == "leaf":
                return it[1]
            lab = labels[lo]
            e, jif = it[1], it[2]
            if lab == T:
                return e if jif else negate(e)
            if lab == F:
                return negate(e) if jif else e
            return None
        if items[hi - 1][0] != "leaf" and labels[hi - 1] not in (T, F):
            return None
        for k in range(hi - 1, lo, -1):
            for op in ("and", "or"):
                if items[k - 1][0] == "leaf":
                    continue
                if op == "and":
                    lT, lF = ("S", k), F
                    if labels[k - 1] != lF:
                        continue
                else:
                    lT, lF = T, ("S", k)
                    if labels[k - 1] != lT:
                        continue
                ok = True
                for m in range(lo, k):
                    lab = labels[m]
                    if lab in (lT, lF):
                        continue
                    if isinstance(lab, tuple) and lab[0] == "S" and lo < lab[1] < k:
                        continue
                    ok = False
                    break
                if not ok:
                    continue
                for m in range(k, hi):
                    lab = labels[m]
                    if lab is None or lab in (T, F):
                        continue
                    if isinstance(lab, tuple) and lab[0] == "S" and k < lab[1] < hi:
                        continue
                    ok = False
                    break
                if not ok:
                    continue
                left = self.build(items, labels, lo, k, lT, lF)
                if left is None:
                    continue
                right = self.build(items, labels, k, hi, T, F)
                if right is None:
                    continue
                e = Bin(op, left, right)
                e.line = left.line
                return e
        return None

    # ------------------------------------------------------------ call execution

    def exec_call(self, pc):
        ins = self.code[pc]
        b = ins.b
        if b == 0:
            raise DecompileError("zero result call inside expression")
        call = self.build_call(ins.a, b)
        call.line = call.fn.line
        if b == MULT_RET:
            self.push(call, None)
        else:
            self.push(call, None)
            for _ in range(b - 1):
                m = MultiRes(call)
                m.line = call.line
                self.stack.append(m)
        return pc + 1

    # ------------------------------------------------------------ statements

    def emit(self, st, line=None):
        if line is not None:
            st.line = line
        self.out.append(st)

    def block(self, start, end, loop, stop_at=None):
        saved_out = self.out
        saved_nloc = self.nloc
        saved_vars = len(self.vars)
        self.out = []
        pc = start
        result = None
        while pc < end:
            r = self.statement(pc, end, loop, stop_at)
            if isinstance(r, tuple):
                pc, result = r
                break
            pc = r
        stmts = self.finish_block(self.out, saved_nloc)
        self.out = saved_out
        del self.stack[saved_nloc:]
        self.nloc = saved_nloc
        return stmts, pc, result

    def finish_block(self, stmts, base_nloc):
        if stmts and isinstance(stmts[-1], ScopeEnd) and stmts[-1].vars and stmts[-1].vars[0].slot == base_nloc:
            stmts = stmts[:-1]
        result = []
        for st in stmts:
            if not isinstance(st, ScopeEnd):
                result.append(st)
                continue
            first_var = st.vars[0]
            k = None
            for i in range(len(result) - 1, -1, -1):
                s2 = result[i]
                if isinstance(s2, LocalStmt) and first_var in s2.vars:
                    k = i
                    break
            if k is None:
                raise DecompileError("scope end without declaration")
            body = result[k:]
            del result[k:]
            d = Do(body)
            d.line = body[0].line if body else None
            result.append(d)
        return result

    def loop_sources(self, pc, end):
        if pc not in self.back_jumps:
            return []
        rej = self.rejected_heads.get(pc, ())
        return [x for x in self.back_jumps[pc] if x < end and x not in self.active_loops and x not in rej]

    def statement(self, pc, end, loop, stop_at):
        self.declare_debug(pc)
        srcs = self.loop_sources(pc, end)
        if srcs and len(self.stack) == self.nloc:
            rejected = set()
            while True:
                cand = [x for x in srcs if x not in rejected]
                if not cand:
                    break
                M = max(cand)
                snap = self.snapshot()
                out_len = len(self.out)
                try:
                    return self.loop_stmt(pc, M, end, loop)
                except DecompileError:
                    self.restore(snap)
                    del self.out[out_len:]
                    self.active_loops.discard(M)
                    rejected.add(M)
                    self.active_loops.add(M)
                    self.rejected_heads.setdefault(pc, set()).add(M)
            for M in rejected:
                self.active_loops.discard(M)
        start_pc = pc
        while True:
            if pc >= end:
                if len(self.stack) > self.nloc:
                    self.declare(len(self.stack), pc)
                return pc
            if pc != start_pc and len(self.stack) > self.nloc and self.loop_sources(pc, end):
                self.declare(len(self.stack), pc)
                return pc
            self.declare_debug(pc)
            ins = self.code[pc]
            op = ins.op
            if op in EXPR_OPS:
                pc = self.step(pc)
                continue
            if op == "CALL":
                if ins.b == 0:
                    nxt = self.code[pc + 1] if pc + 1 < len(self.code) else None
                    if nxt is not None and nxt.op in STORE_OPS:
                        self.pending_extras.append(self.build_call(ins.a, 0))
                        pc += 1
                        continue
                    self.declare(ins.a, pc)
                    call = self.build_call(ins.a, 0)
                    self.emit(CallStmt(call), call.line)
                    return pc + 1
                pc = self.exec_call(pc)
                continue
            if op in STORE_OPS:
                return self.assignment(pc, loop)
            if op == "POP":
                nxt = self.code[pc + 1] if pc + 1 < len(self.code) else None
                if nxt is not None and nxt.op in STORE_OPS:
                    n = ins.u
                    if len(self.stack) - n < self.nloc:
                        raise DecompileError("extra values below locals")
                    self.pending_extras = self.stack[len(self.stack) - n:] + self.pending_extras
                    del self.stack[len(self.stack) - n:]
                    pc += 1
                    continue
                if nxt is not None and nxt.op == "JMP" and loop is not None and self.same_dest(nxt.tgt, loop.exit):
                    if len(self.stack) > self.nloc:
                        self.declare(len(self.stack), pc)
                    return self.break_stmt(pc + 1, ins.u)
                if len(self.stack) > self.nloc:
                    self.declare(len(self.stack), pc)
                n = ins.u
                if n > self.nloc:
                    raise DecompileError("POP below function locals")
                vars_ = [self.stack[i].var for i in range(self.nloc - n, self.nloc)]
                se = ScopeEnd(vars_)
                se.pc = pc
                se.at_end = (pc + 1 == end) or (stop_at is not None and pc + 1 == stop_at)
                self.out.append(se)
                del self.stack[self.nloc - n:]
                self.nloc -= n
                return pc + 1
            if op == "JMP":
                if loop is not None and self.same_dest(ins.tgt, loop.exit):
                    if len(self.stack) > self.nloc:
                        self.declare(len(self.stack), pc)
                    return self.break_stmt(pc, 0)
                if len(self.stack) > self.nloc:
                    self.declare(len(self.stack), pc)
                return self.cond_statement(pc, end, loop, stop_at)
            if op == "RETURN":
                u = ins.u
                if u > len(self.stack) or u < self.nloc:
                    raise DecompileError("bad RETURN")
                self.declare(u, pc)
                exprs = self.stack[u:]
                del self.stack[u:]
                st = Return(list(exprs))
                self.emit(st, self.first_line(exprs) if exprs else ins.line)
                return pc + 1
            if op == "TAILCALL":
                base = ins.b
                if base < self.nloc or base > ins.a:
                    raise DecompileError("bad TAILCALL")
                self.declare(base, pc)
                call = self.build_call(ins.a, MULT_RET)
                exprs = self.stack[base:] + [call]
                del self.stack[base:]
                self.emit(Return(exprs), self.first_line(exprs))
                return pc + 1
            if op in COND_JUMPS:
                if self.is_value_jump(pc):
                    pc = self.value_chain(pc)
                    continue
                return self.cond_statement(pc, end, loop, stop_at)
            if op == "FORPREP":
                return self.for_num(pc, loop)
            if op == "LFORPREP":
                return self.for_in(pc, loop)
            if op == "END":
                if len(self.stack) > self.nloc:
                    self.declare(len(self.stack), pc)
                return pc + 1
            raise DecompileError("unexpected %s at %d" % (op, pc))

    def break_stmt(self, jpc, popped):
        st = Break()
        st.line = self.code[jpc].line
        self.emit(st)
        pc = jpc + 1
        if popped and pc < len(self.code) and self.code[pc].op == "PUSHNIL" and self.code[pc].u == popped:
            pc += 1
        return pc

    def assignment(self, pc, loop=None):
        targets = []
        values = []
        scope_pop = 0
        while pc < len(self.code) and self.code[pc].op in STORE_OPS:
            ins = self.code[pc]
            if ins.op == "SETLOCAL":
                v = self.pop()
                values.append(v)
                if ins.u >= self.nloc:
                    self.declare(ins.u + 1, pc)
                ref = self.stack[ins.u]
                if not isinstance(ref, LocalRef):
                    raise DecompileError("SETLOCAL on non-local")
                targets.append(LocalRef(ref.var))
            elif ins.op == "SETGLOBAL":
                v = self.pop()
                values.append(v)
                targets.append(Global(self.kstr(ins.u)))
            else:
                a, b = ins.a, ins.b
                n = len(self.stack)
                if a < 3 or n - a < self.nloc:
                    raise DecompileError("bad SETTABLE")
                v = self.stack[n - 1]
                t = self.stack[n - a]
                k = self.stack[n - a + 1]
                if isinstance(t, Garbage) or isinstance(k, Garbage):
                    raise DecompileError("SETTABLE on consumed target")
                values.append(v)
                idx = Index(t, k)
                idx.line = t.line
                targets.append(idx)
                self.stack[n - a] = Garbage()
                self.stack[n - a + 1] = Garbage()
                self.stack[n - 1] = Garbage()
                if n - b < self.nloc:
                    if any(not isinstance(x, Garbage) for x in self.stack[self.nloc:n]):
                        raise DecompileError("SETTABLE pops pending values")
                    scope_pop = self.nloc - (n - b)
                    del self.stack[self.nloc:]
                else:
                    del self.stack[n - b:]
            pc += 1
        g = 0
        while len(self.stack) - g > self.nloc and isinstance(self.stack[len(self.stack) - 1 - g], Garbage):
            g += 1
        if g:
            if pc < len(self.code) and self.code[pc].op == "POP" and self.code[pc].u == g:
                del self.stack[len(self.stack) - g:]
                pc += 1
            else:
                raise DecompileError("assignment garbage without POP")
        for e in self.stack[self.nloc:]:
            if isinstance(e, Garbage):
                raise DecompileError("garbage left after assignment")
        targets.reverse()
        values.reverse()
        exprs = values + self.pending_extras
        self.pending_extras = []
        self.declare(len(self.stack), pc)
        st = Assign(targets, exprs)
        lines = [x.line for x in targets + exprs if x.line is not None]
        st.line = min(lines) if lines else None
        self.emit(st)
        if scope_pop:
            nxt = self.code[pc] if pc < len(self.code) else None
            if nxt is not None and nxt.op == "JMP" and loop is not None and self.same_dest(nxt.tgt, loop.exit):
                return self.break_stmt(pc, scope_pop)
            self.scope_end(scope_pop, pc - 1)
        return pc

    def scope_end(self, n, pc):
        if n > self.nloc:
            raise DecompileError("scope end below function locals")
        vars_ = [self.stack[i].var for i in range(self.nloc - n, self.nloc)]
        se = ScopeEnd(vars_)
        se.pc = pc
        self.out.append(se)
        del self.stack[self.nloc - n:]
        self.nloc -= n

    def for_num(self, pc, loop):
        ins = self.code[pc]
        n = len(self.stack)
        if n - 3 < self.nloc:
            raise DecompileError("bad FORPREP")
        self.declare(n - 3, pc)
        start, limit, step = self.stack[n - 3:]
        loop_pc = self.find_loop_end(pc, "FORLOOP")
        exit_pc = loop_pc + 1
        if not self.same_dest(ins.tgt, exit_pc):
            raise DecompileError("bad numeric for")
        var = self.make_var(n - 3, pc + 1)
        vlim = LocalVar("(limit)", n - 2)
        vstep = LocalVar("(step)", n - 1)
        self.stack[n - 3] = LocalRef(var)
        self.stack[n - 2] = LocalRef(vlim)
        self.stack[n - 1] = LocalRef(vstep)
        self.nloc = n
        body, _, _ = self.block(pc + 1, loop_pc, Loop(exit_pc, self.nloc))
        del self.stack[n - 3:]
        self.nloc = n - 3
        st = ForNum(var, start, limit, None if (isinstance(step, Num) and step.v == 1) else step, body)
        st.line = start.line
        self.emit(st)
        return exit_pc

    def for_in(self, pc, loop):
        ins = self.code[pc]
        n = len(self.stack)
        if n - 1 < self.nloc:
            raise DecompileError("bad LFORPREP")
        self.declare(n - 1, pc)
        table = self.stack[n - 1]
        loop_pc = self.find_loop_end(pc, "LFORLOOP")
        exit_pc = loop_pc + 1
        if not self.same_dest(ins.tgt, exit_pc):
            raise DecompileError("bad generic for")
        vt = LocalVar("(table)", n - 1)
        kv = self.make_var(n, pc + 1)
        vv = self.make_var(n + 1, pc + 1)
        if self.debug_locals:
            kn = self.debug_name(n, pc + 1)
            vn = self.debug_name(n + 1, pc + 1)
            if kn:
                kv.name = kn
            if vn:
                vv.name = vn
        self.stack[n - 1] = LocalRef(vt)
        self.stack.append(LocalRef(kv))
        self.stack.append(LocalRef(vv))
        self.nloc = n + 2
        body, _, _ = self.block(pc + 1, loop_pc, Loop(exit_pc, self.nloc))
        del self.stack[n - 1:]
        self.nloc = n - 1
        st = ForIn(kv, vv, table, body)
        st.line = table.line
        self.emit(st)
        return exit_pc

    # ------------------------------------------------------------ conditions

    def snapshot(self):
        return (list(self.stack), self.nloc, len(self.vars))

    def restore(self, snap):
        self.stack = list(snap[0])
        self.nloc = snap[1]
        del self.vars[snap[2]:]

    def parse_atom(self, pc, h0, limit, head_ok=False):
        snap = self.snapshot()
        self.sandbox += 1
        start = pc
        try:
            while True:
                if pc >= limit:
                    raise Abort()
                if pc in self.back_jumps and not (head_ok and pc == start):
                    raise Abort()
                ins = self.code[pc]
                if ins.op in COND_JUMPS:
                    n = 2 if ins.op in CMP_JUMPS else 1
                    if len(self.stack) - n == h0:
                        if ins.op in ("JMPONT", "JMPONF"):
                            raise Abort()
                        e, jif = self.atom_from_jump(ins)
                        return (e, jif, ins, start), pc + 1
                    if len(self.stack) - n < h0:
                        raise Abort()
                    if not self.is_value_jump(pc):
                        raise Abort()
                    pc = self.value_chain(pc)
                    continue
                if ins.op == "CALL":
                    if ins.b == 0:
                        raise Abort()
                    pc = self.exec_call(pc)
                    continue
                if ins.op in EXPR_OPS:
                    pc = self.step(pc)
                    continue
                raise Abort()
        except (Abort, DecompileError):
            self.restore(snap)
            return None, pc
        finally:
            self.sandbox -= 1

    def collect_chain(self, jpc, end):
        ins = self.code[jpc]
        if ins.op == "JMP":
            h0 = len(self.stack)
            e = Nil()
            e.line = ins.line
            jif = False
        else:
            n = 2 if ins.op in CMP_JUMPS else 1
            h0 = len(self.stack) - n
            if h0 < self.nloc:
                raise DecompileError("condition below locals")
            self.declare(h0, jpc)
            e, jif = self.atom_from_jump(ins)
        atoms = [(e, jif, ins, None)]
        snaps = [self.snapshot()]
        pc = jpc + 1
        while pc < end:
            r, npc = self.parse_atom(pc, h0, end)
            if r is None:
                break
            atoms.append(r)
            snaps.append(self.snapshot())
            pc = npc
        return atoms, snaps, h0

    def chain_expr(self, atoms, T, F):
        starts = [a[3] for a in atoms]
        items = [("atom", a[0], a[1], a[2]) for a in atoms]
        labels = []
        dT, dF = self.dest(T), self.dest(F)
        for i, a in enumerate(atoms):
            t = a[2].tgt
            if t != T and t != F and t not in starts[1:]:
                if self.dest(t) == dT:
                    t = T
                elif self.dest(t) == dF:
                    t = F
            if t == T and t == F:
                if i == len(atoms) - 1:
                    labels.append("F")
                else:
                    labels.append("T" if a[1] else "F")
            elif t == T:
                labels.append("T")
            elif t == F:
                labels.append("F")
            elif t in starts[1:]:
                labels.append(("S", starts.index(t)))
            else:
                labels.append(("X", t))
        return self.build(items, labels, 0, len(items), "T", "F")

    def cond_statement(self, jpc, end, loop, stop_at):
        atoms, snaps, h0 = self.collect_chain(jpc, end)
        for n in range(len(atoms), 0, -1):
            sub = atoms[:n]
            last = sub[-1][2]
            T = last.pc + 1
            F = last.tgt
            if stop_at is not None and last.pc == stop_at - 1 and F < T:
                cond = self.chain_expr(sub, T, F)
                if cond is not None:
                    self.restore(snaps[n - 1])
                    cond = self.chain_expr(sub, T, F)
                    return (T, cond)
                continue
            if F <= last.pc and not (F < T):
                continue
            cond = self.chain_expr(sub, T, F)
            if cond is None:
                continue
            self.restore(snaps[n - 1])
            return self.if_stmt(cond, T, F, end, loop, stop_at)
        raise DecompileError("cannot rebuild condition at %d" % jpc)

    def if_stmt(self, cond, T, F, end, loop, stop_at):
        line = cond.line
        if F < T or F > end:
            then_end = end
            else_start = None
        else:
            then_end = F
            else_start = None
            if F > T:
                j = self.code[F - 1]
                is_break = j.op == "JMP" and loop is not None and self.same_dest(j.tgt, loop.exit)
                if j.op == "JMP" and not is_break and (j.tgt >= F or j.tgt < T):
                    then_end = F - 1
                    else_start = F
        then, _, _ = self.block(T, then_end, loop)
        orelse = None
        next_pc = then_end
        if else_start is not None:
            j = self.code[then_end]
            X = j.tgt
            if X < else_start or X > end:
                X = end
            orelse, _, _ = self.block(else_start, X, loop)
            next_pc = X
        elif then_end == end and (F < T or F > end):
            next_pc = end
        else:
            next_pc = F
        st = If(cond, then, orelse)
        st.line = line
        self.emit(st)
        return next_pc

    # ------------------------------------------------------------ loops

    def check_loop_body(self, pc, M):
        exit_pc = M + 1
        dexit = self.dest(exit_pc)
        for q in range(pc, M):
            ins = self.code[q]
            if ins.tgt is None:
                continue
            t = ins.tgt
            if pc <= t <= exit_pc:
                continue
            if self.dest(t) == dexit:
                continue
            raise DecompileError("jump out of loop body at %d" % q)

    def loop_stmt(self, pc, M, end, loop):
        self.check_loop_body(pc, M)
        j = self.code[M]
        if j.op == "JMP":
            exit_pc = M + 1
            lp = Loop(exit_pc, self.nloc)
            cond = None
            body_start = pc
            r = self.try_while_cond(pc, M, exit_pc)
            if r is not None:
                cond, body_start = r
            for q in range(body_start, M):
                ins = self.code[q]
                if ins.op in COND_JUMPS and ins.tgt == exit_pc:
                    raise DecompileError("conditional exit inside while body")
            self.active_loops.add(M)
            body, _, _ = self.block(body_start, M, lp)
            self.active_loops.discard(M)
            st = While(cond, body)
            st.line = cond.line if cond is not None else (self.code[pc].line)
            self.emit(st)
            return exit_pc
        exit_pc = M + 1
        lp = Loop(exit_pc, self.nloc)
        self.active_loops.add(M)
        body, npc, cond = self.block(pc, M + 1, lp, stop_at=M + 1)
        self.active_loops.discard(M)
        if cond is None:
            raise DecompileError("repeat without until")
        st = Repeat(body, cond)
        st.line = self.code[pc].line
        self.emit(st)
        return exit_pc

    def try_while_cond(self, pc, M, exit_pc):
        h0 = len(self.stack)
        r, npc = self.parse_atom(pc, h0, M, head_ok=True)
        if r is None:
            return None
        atoms = [r]
        snaps = [self.snapshot()]
        q = npc
        while q < M:
            r2, nq = self.parse_atom(q, h0, M)
            if r2 is None:
                break
            atoms.append(r2)
            snaps.append(self.snapshot())
            q = nq
        for n in range(len(atoms), 0, -1):
            sub = atoms[:n]
            last = sub[-1][2]
            if not self.same_dest(last.tgt, exit_pc):
                continue
            T = last.pc + 1
            cond = self.chain_expr(sub, T, last.tgt)
            if cond is None:
                continue
            self.restore(snaps[n - 1])
            return cond, T
        return None

    # ------------------------------------------------------------ entry

    def run(self):
        f = self.f
        params = []
        nparams = f.num_params
        for i in range(nparams):
            name = None
            if self.debug_locals and i < len(self.debug_locals):
                name = self.debug_locals[i][0]
            if name is None:
                name = self.ctx.fresh("p")
            v = LocalVar(name, i)
            self.vars.append(v)
            params.append(v)
            self.stack.append(LocalRef(v))
        if f.is_vararg:
            v = LocalVar("arg", nparams)
            self.vars.append(v)
            self.stack.append(LocalRef(v))
        self.nloc = len(self.stack)
        body, pc, _ = self.block(0, len(self.code), None)
        func = Func(f, params, bool(f.is_vararg), body, self.upnames)
        return func


class Context:
    def __init__(self, proto):
        self.used_names = set()
        self.counter = 0
        self.collect(proto)

    def fresh(self, prefix):
        while True:
            self.counter += 1
            name = "%s%d" % (prefix, self.counter)
            if name not in self.used_names:
                return name

    def collect(self, p):
        for s in p.kstr:
            if s is not None:
                self.used_names.add(s)
        for q in p.kproto:
            self.collect(q)

    def decompile(self, proto, upnames):
        d = FuncDecompiler(proto, upnames, self)
        return d.run()


def decompile_chunk(proto):
    ctx = Context(proto)
    return ctx.decompile(proto, [])
