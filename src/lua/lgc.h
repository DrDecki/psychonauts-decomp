/*
** $Id: lgc.h,v 1.8 2000/10/02 14:47:43 roberto Exp $
** Changed by Double Fine for Psychonauts, reconstructed from Psychonauts.exe
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef lgc_h
#define lgc_h


#include "lobject.h"


extern lua_GCMarkHook luaC_markhook;
extern lua_GCHook luaC_gchook;

void luaC_markstack (lua_State *L);
void luaC_collect (lua_State *L, int all);
void luaC_checkGC (lua_State *L);


#endif
