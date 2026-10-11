/*
** $Id: ltable.h,v 1.24 2000/08/31 14:08:27 roberto Exp $
** Changed by Double Fine for Psychonauts, reconstructed from Psychonauts.exe
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ifndef ltable_h
#define ltable_h

#include "lobject.h"


#define node(t,i)	(&(t)->node[i])
#define nextnode(t,n)	((n)->next ? &(t)->node[(n)->next-1] : NULL)

/* copy a key between a TObject and a Node */
#define setnodekey(n,o) \
	do { const TObject *o_ = (o); (n)->keytt = ttype(o_); (n)->key = o_->value; } while (0)
#define getnodekey(o,n) \
	do { TObject *o_ = (o); ttype(o_) = (n)->keytt; o_->value = (n)->key; } while (0)

/* copy a value between a TObject and a Node */
#define setnodeval(n,o) \
	do { const TObject *o_ = (o); (n)->valtt = ttype(o_); (n)->val = o_->value; } while (0)
#define getnodeval(o,n) \
	do { TObject *o_ = (o); ttype(o_) = (n)->valtt; o_->value = (n)->val; } while (0)

Hash *luaH_new (lua_State *L, int nhash);
void luaH_free (lua_State *L, Hash *t);
const Node *luaH_get (lua_State *L, const Hash *t, const TObject *key);
const Node *luaH_getnum (const Hash *t, Number key);
const Node *luaH_getstr (const Hash *t, TString *key);
void luaH_remove (Hash *t, Node *key);
Node *luaH_set (lua_State *L, Hash *t, const TObject *key);
Node * luaH_next (lua_State *L, const Hash *t, const TObject *r);
Node *luaH_setint (lua_State *L, Hash *t, int key);
void luaH_setstrnum (lua_State *L, Hash *t, TString *key, Number val);
unsigned long luaH_hash (lua_State *L, const TObject *key);
const Node *luaH_getglobal (lua_State *L, const char *name);
int luaH_mainindex (const Hash *t, int tt, const Value *v);

/* exported only for debugging */
Node *luaH_mainposition (const Hash *t, const TObject *key);


#endif
