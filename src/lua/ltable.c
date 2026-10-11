/*
** $Id: ltable.c,v 1.58 2000/10/26 12:47:05 roberto Exp $
** Changed by Double Fine for Psychonauts, reconstructed from Psychonauts.exe
** Lua tables (hash)
** See Copyright Notice in lua.h
*/


/*
** Implementation of tables (aka arrays, objects, or hash tables);
** uses a mix of chained scatter table with Brent's variation.
** A main invariant of these tables is that, if an element is not
** in its main position (i.e. the `original' position that its hash gives
** to it), then the colliding element is in its own main position.
** In other words, there are collisions only when two elements have the
** same main position (i.e. the same hash values for that table size).
** Because of that, the load factor of these tables can be 100% without
** performance penalties.
*/


#include "lua.h"

#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"


#define gcsize(L, n)	(sizeof(Hash)+(n)*sizeof(Node))



#define TagDefault LUA_TTABLE



/*
** returns the `main' position of an element in a table (that is, the index
** of its hash value), counting from 1; 0 for an invalid key
*/
int luaH_mainindex (const Hash *t, int tt, const Value *v) {
  unsigned long h;
  switch (tt) {
    case LUA_TNUMBER:
      h = (unsigned long)(long)v->n;
      break;
    case LUA_TSTRING:
      h = v->ts->u.s.hash;
      break;
    case LUA_TUSERDATA:
      h = IntPoint(v->ts);
      break;
    case LUA_TTABLE:
      h = IntPoint(v->a);
      break;
    case LUA_TFUNCTION:
      h = IntPoint(v->cl);
      break;
    default:
      return 0;  /* invalid key */
  }
  LUA_ASSERT(h%(unsigned int)t->size == (h&((unsigned int)t->size-1)),
            "a&(x-1) == a%x, for x power of 2");
  return (h&(t->size-1)) + 1;
}


Node *luaH_mainposition (const Hash *t, const TObject *key) {
  int i = luaH_mainindex(t, ttype(key), &key->value);
  return i ? node(t, i-1) : NULL;
}


static const Node *luaH_getany (lua_State *L, const Hash *t,
                                const TObject *key) {
  Node *n = luaH_mainposition(t, key);
  if (!n)
    lua_error(L, "table index is nil");
  else do {
    if (luaO_equalValue(key, n->keytt, &n->key))
      return n;
    n = nextnode(t, n);
  } while (n);
  return &luaO_nilnode;  /* key not found */
}


/* specialized version for numbers */
const Node *luaH_getnum (const Hash *t, Number key) {
  Node *n = &t->node[(unsigned long)(long)key&(t->size-1)];
  do {
    if (n->keytt == LUA_TNUMBER && n->key.n == key)
      return n;
    n = nextnode(t, n);
  } while (n);
  return &luaO_nilnode;  /* key not found */
}


/* specialized version for strings */
const Node *luaH_getstr (const Hash *t, TString *key) {
  Node *n = &t->node[key->u.s.hash&(t->size-1)];
  do {
    if (n->keytt == LUA_TSTRING && n->key.ts == key)
      return n;
    n = nextnode(t, n);
  } while (n);
  return &luaO_nilnode;  /* key not found */
}


const Node *luaH_get (lua_State *L, const Hash *t, const TObject *key) {
  switch (ttype(key)) {
    case LUA_TNUMBER: return luaH_getnum(t, nvalue(key));
    case LUA_TSTRING: return luaH_getstr(t, tsvalue(key));
    default:         return luaH_getany(L, t, key);
  }
}


Node *luaH_next (lua_State *L, const Hash *t, const TObject *key) {
  int i;
  if (ttype(key) == LUA_TNIL)
    i = 0;  /* first iteration */
  else {
    const Node *v = luaH_get(L, t, key);
    if (v == &luaO_nilnode)
      lua_error(L, "invalid key for `next'");
    i = (int)(((const char *)v -
               (const char *)(&t->node[0])) / sizeof(Node)) + 1;
  }
  for (; i<t->size; i++) {
    Node *n = node(t, i);
    if (n->valtt != LUA_TNIL)
      return n;
  }
  return NULL;  /* no more elements */
}


/*
** try to remove a key without value from a table. To avoid problems with
** hash, change `key' for a number with the same hash.
*/
void luaH_remove (Hash *t, Node *key) {
  if (key->keytt == LUA_TNUMBER ||
       (key->keytt == LUA_TSTRING && key->key.ts->len <= 30))
  return;  /* do not remove numbers nor small strings */
  else {
    /* try to find a number `n' with the same hash as `key' */
    int mp = luaH_mainindex(t, key->keytt, &key->key);
    int n = mp - 1;
    /* make sure `n' is not in `t' */
    while (luaH_getnum(t, n) != &luaO_nilnode) {
      if (n >= MAX_SHORT - t->size)
        return;  /* give up; (to avoid overflow) */
      n += t->size;
    }
    key->keytt = LUA_TNUMBER;
    key->key.n = n;
  }
}


static void setnodevector (lua_State *L, Hash *t, lint32 size) {
  int i;
  if (size > MAX_SHORT)
    lua_error(L, "table overflow");
  t->node = luaM_newvector(L, size, Node);
  for (i=0; i<(int)size; i++) {
    t->node[i].keytt = t->node[i].valtt = LUA_TNIL;
    t->node[i].next = 0;
  }
  L->nblocks += gcsize(L, size) - gcsize(L, t->size);
  t->size = size;
  t->firstfree = &t->node[size-1];  /* first free position to be used */
}


Hash *luaH_new (lua_State *L, int size) {
  Hash *t = luaM_new(L, Hash);
  t->htag = TagDefault;
  t->next = L->roottable;
  L->roottable = t;
  t->mark = t;
  t->size = 0;
  L->nblocks += gcsize(L, 0);
  t->node = NULL;
  setnodevector(L, t, luaO_power2(size));
  return t;
}


void luaH_free (lua_State *L, Hash *t) {
  L->nblocks -= gcsize(L, t->size);
  luaM_freevector(L, t->node, t->size, Node);
  luaM_freesize(L, t, sizeof(Hash));
}


static int numuse (const Hash *t) {
  Node *v = t->node;
  int size = t->size;
  int realuse = 0;
  int i;
  for (i=0; i<size; i++) {
    if (v[i].valtt != LUA_TNIL)
      realuse++;
  }
  return realuse;
}


static void rehash (lua_State *L, Hash *t) {
  int oldsize = t->size;
  Node *nold = t->node;
  int nelems = numuse(t);
  int i;
  LUA_ASSERT(nelems<=oldsize, "wrong count");
  if (nelems >= oldsize-oldsize/4)  /* using more than 3/4? */
    setnodevector(L, t, (lint32)oldsize*2);
  else if (nelems <= oldsize/4 &&  /* less than 1/4? */
           oldsize > MINPOWER2)
    setnodevector(L, t, oldsize/2);
  else
    setnodevector(L, t, oldsize);
  for (i=0; i<oldsize; i++) {
    Node *old = nold+i;
    if (old->valtt != LUA_TNIL) {
      TObject key;
      Node *slot;
      getnodekey(&key, old);
      slot = luaH_set(L, t, &key);
      slot->valtt = old->valtt;
      slot->val = old->val;
    }
  }
  luaM_freevector(L, nold, oldsize, Node);  /* free old array */
}


/*
** inserts a key into a hash table; first, check whether key is
** already present; if not, check whether key's main position is free;
** if not, check whether colliding node is in its main position or not;
** if it is not, move colliding node to an empty place and put new key
** in its main position; otherwise (colliding node is in its main position),
** new key goes to an empty position.
*/
Node *luaH_set (lua_State *L, Hash *t, const TObject *key) {
  int mpi = luaH_mainindex(t, ttype(key), &key->value);
  int pos = mpi;
  Node *mp;
  if (!mpi)
    lua_error(L, "table index is nil");
  do {  /* check whether `key' is somewhere in the chain */
    if (luaO_equalValue(key, t->node[pos-1].keytt, &t->node[pos-1].key))
      return &t->node[pos-1];  /* that's all */
    else pos = t->node[pos-1].next;
  } while (pos);
  /* `key' not found; must insert it */
  mp = &t->node[mpi-1];
  if (mp->keytt != LUA_TNIL) {  /* main position is not free? */
    int othern;  /* main position of colliding node */
    pos = t->firstfree - t->node + 1;  /* get a free place */
    /* is colliding node out of its main position? (can only happens if
       its position is after "firstfree") */
    if (mpi > pos && (othern=luaH_mainindex(t, mp->keytt, &mp->key)) != mpi) {
      /* yes; move colliding node into free position */
      while (t->node[othern-1].next != mpi)
        othern = t->node[othern-1].next;  /* find previous */
      t->node[othern-1].next = pos;  /* redo the chain with `pos' in place of `mp' */
      t->node[pos-1] = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      mp->next = 0;  /* now `mp' is free */
    }
    else {  /* colliding node is in its own main position */
      /* new node will go into free position */
      t->node[pos-1].next = mp->next;  /* chain new position */
      mp->next = pos;
      mp = &t->node[pos-1];
    }
  }
  setnodekey(mp, key);
  for (;;) {  /* correct `firstfree' */
    if (t->firstfree->keytt == LUA_TNIL)
      return mp;  /* OK; table still has a free place */
    else if (t->firstfree == t->node) break;  /* cannot decrement from here */
    else (t->firstfree)--;
  }
  rehash(L, t);  /* no more free places */
  return luaH_set(L, t, key);  /* `rehash' invalidates this insertion */
}


Node *luaH_setint (lua_State *L, Hash *t, int key) {
  TObject index;
  ttype(&index) = LUA_TNUMBER;
  nvalue(&index) = key;
  return luaH_set(L, t, &index);
}


void luaH_setstrnum (lua_State *L, Hash *t, TString *key, Number val) {
  TObject index;
  Node *slot;
  ttype(&index) = LUA_TSTRING;
  tsvalue(&index) = key;
  slot = luaH_set(L, t, &index);
  slot->valtt = LUA_TNUMBER;
  slot->val.n = val;
}


const Node *luaH_getglobal (lua_State *L, const char *name) {
  return luaH_getstr(L->gt, luaS_new(L, name));
}

