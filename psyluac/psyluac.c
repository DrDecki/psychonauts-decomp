#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "ldo.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "lzio.h"

#define OP_EXTARG 49
#define OP_CALLX 51
#define OP_CLOSUREX 52
#define A16_MAX 31
#define S16_MAX 511

enum { M_NONE, M_U, M_S, M_J, M_AB };

static int argmode(int op) {
  switch (op) {
    case OP_RETURN: case OP_PUSHNIL: case OP_POP: case OP_PUSHSTRING:
    case OP_PUSHNUM: case OP_PUSHNEGNUM: case OP_PUSHUPVALUE: case OP_GETLOCAL:
    case OP_GETGLOBAL: case OP_GETDOTTED: case OP_GETINDEXED: case OP_PUSHSELF:
    case OP_CREATETABLE: case OP_SETLOCAL: case OP_SETGLOBAL: case OP_SETMAP:
    case OP_CONCAT:
      return M_U;
    case OP_PUSHINT: case OP_ADDI:
      return M_S;
    case OP_JMPNE: case OP_JMPEQ: case OP_JMPLT: case OP_JMPLE: case OP_JMPGT:
    case OP_JMPGE: case OP_JMPT: case OP_JMPF: case OP_JMPONT: case OP_JMPONF:
    case OP_JMP: case OP_FORPREP: case OP_FORLOOP: case OP_LFORPREP: case OP_LFORLOOP:
      return M_J;
    case OP_CALL: case OP_TAILCALL: case OP_SETTABLE: case OP_SETLIST: case OP_CLOSURE:
      return M_AB;
    default:
      return M_NONE;
  }
}

static int needs_ext(Instruction i) {
  OpCode o = GET_OPCODE(i);
  if (o == OP_CLOSURE) return GETARG_A(i) >= A16_MAX - 1;
  if (o == OP_CALL) return GETARG_A(i) > A16_MAX;
  return 0;
}

static FILE *D;
static int strip;
static int failed;

static void fail(const char *msg, int pc) {
  fprintf(stderr, "psyluac: %s at pc %d\n", msg, pc);
  failed = 1;
}

static void dump_byte(int b) { fputc(b, D); }
static void dump_int(int x) { fwrite(&x, 4, 1, D); }
static void dump_u32(unsigned int x) { fwrite(&x, 4, 1, D); }
static void dump_u16(unsigned short x) { fwrite(&x, 2, 1, D); }
static void dump_float(float x) { fwrite(&x, 4, 1, D); }

static void dump_string(const TString *s) {
  if (s == NULL) {
    dump_u32(0);
    return;
  }
  dump_u32((unsigned int)s->len + 1);
  fwrite(s->str, 1, s->len + 1, D);
}

static void dump_function(const Proto *f) {
  int n = f->ncode;
  int *map = malloc(sizeof(int) * (n + 1));
  int i, total = 0;
  for (i = 0; i < n; i++) {
    map[i] = total;
    total += needs_ext(f->code[i]) ? 2 : 1;
  }
  map[n] = total;

  dump_string(f->source);
  dump_int(f->lineDefined);
  dump_int(f->numparams);
  dump_byte(f->is_vararg);
  dump_int(f->maxstacksize);

  if (strip) {
    dump_int(0);
    dump_int(0);
  } else {
    dump_int(f->nlocvars);
    for (i = 0; i < f->nlocvars; i++) {
      dump_string(f->locvars[i].varname);
      dump_int(map[f->locvars[i].startpc]);
      dump_int(map[f->locvars[i].endpc]);
    }
    dump_int(f->nlineinfo);
    for (i = 0; i < f->nlineinfo; i++) {
      int v = f->lineinfo[i];
      dump_int(v >= 0 && v != MAX_INT ? map[v] : v);
    }
  }

  dump_int(f->nkstr);
  for (i = 0; i < f->nkstr; i++) dump_string(f->kstr[i]);
  dump_int(f->nknum);
  for (i = 0; i < f->nknum; i++) dump_float((float)f->knum[i]);
  dump_int(f->nkproto);
  for (i = 0; i < f->nkproto; i++) dump_function(f->kproto[i]);

  dump_int(total);
  for (i = 0; i < n; i++) {
    Instruction ins = f->code[i];
    int op = GET_OPCODE(ins);
    switch (argmode(op)) {
      case M_NONE:
        dump_u16((unsigned short)op);
        break;
      case M_U:
        dump_u16((unsigned short)(((GETARG_U(ins) & 1023) << 6) | op));
        break;
      case M_S: {
        int s = GETARG_S(ins);
        if (s < -S16_MAX || s > S16_MAX) fail("S argument out of range", i);
        dump_u16((unsigned short)(((s + S16_MAX) << 6) | op));
        break;
      }
      case M_J: {
        int target = i + 1 + GETARG_S(ins);
        int s;
        if (target < 0 || target > n) {
          fail("jump outside function", i);
          target = i + 1;
        }
        s = map[target] - (map[i] + 1);
        if (s < -S16_MAX || s > S16_MAX) fail("jump too long", i);
        dump_u16((unsigned short)(((s + S16_MAX) << 6) | op));
        break;
      }
      case M_AB: {
        int a = GETARG_A(ins), b = GETARG_B(ins);
        if (b > 31) fail("B argument out of range", i);
        if (needs_ext(ins)) {
          if (a > 1023) fail("extended A out of range", i);
          dump_u16((unsigned short)((a << 6) | OP_EXTARG));
          dump_u16((unsigned short)((b << 6) | (op == OP_CALL ? OP_CALLX : OP_CLOSUREX)));
        } else {
          if (a > A16_MAX) fail("A argument out of range", i);
          dump_u16((unsigned short)((a << 11) | (b << 6) | op));
        }
        break;
      }
    }
  }
  free(map);
}

struct parse_job {
  ZIO *z;
  Proto *f;
};

static void do_parse(lua_State *L, void *ud) {
  struct parse_job *job = (struct parse_job *)ud;
  job->f = luaY_parser(L, job->z);
}

static char *read_file(const char *path, size_t *size) {
  FILE *f = fopen(path, "rb");
  char *buf;
  long n;
  if (f == NULL) return NULL;
  fseek(f, 0, SEEK_END);
  n = ftell(f);
  fseek(f, 0, SEEK_SET);
  buf = malloc(n + 1);
  if (fread(buf, 1, n, f) != (size_t)n) {
    fclose(f);
    free(buf);
    return NULL;
  }
  fclose(f);
  buf[n] = 0;
  *size = (size_t)n;
  return buf;
}

int main(int argc, char **argv) {
  const char *chunkname = NULL, *out = "out.lub", *in = NULL;
  int i;
  size_t size;
  char *src;
  lua_State *L;
  ZIO z;
  struct parse_job job;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-s") == 0) strip = 1;
    else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) chunkname = argv[++i];
    else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) out = argv[++i];
    else in = argv[i];
  }
  if (in == NULL) {
    fprintf(stderr, "usage: psyluac [-s] [-n chunkname] [-o out.lub] in.lua\n");
    return 1;
  }
  src = read_file(in, &size);
  if (src == NULL) {
    fprintf(stderr, "psyluac: cannot read %s\n", in);
    return 1;
  }
  if (chunkname == NULL) chunkname = in;

  L = lua_open(0);
  luaZ_mopen(&z, src, size, chunkname);
  job.z = &z;
  job.f = NULL;
  if (luaD_runprotected(L, do_parse, &job) != 0 || job.f == NULL) {
    fprintf(stderr, "psyluac: cannot compile %s\n", in);
    return 1;
  }

  D = fopen(out, "wb");
  if (D == NULL) {
    fprintf(stderr, "psyluac: cannot write %s\n", out);
    return 1;
  }
  fwrite("\033Lua", 1, 4, D);
  dump_byte(0x40);
  dump_byte(1);
  dump_byte(4);
  dump_byte(4);
  dump_byte(2);
  dump_byte(16);
  dump_byte(6);
  dump_byte(5);
  dump_byte(4);
  dump_float(3.14159f);
  dump_function(job.f);
  fclose(D);
  return failed ? 2 : 0;
}
