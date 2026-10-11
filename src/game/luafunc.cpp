// Registration of the script functions and the argument helpers they share.
// Function names are made up.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "luafunc.h"

ELuaFunction *ELuaFunction::s_first;
int ELuaFunction::s_count;

ELuaFunction::ELuaFunction(lua_CFunction func, const char *name)
{
    m_func = func;
    m_name = name;
    m_next = s_first;
    s_first = this;
    s_count++;
}

// "file.lua(line) function object" of the calling script, the error messages
// that used it are compiled out of the release build
const char *LuaCallSite(lua_State *L)
{
    static char buf[100];
    lua_Debug ar[2];
    unsigned int depth;
    const char *name;
    int lineNo;
    const char *ownerName;
    char *src;

    for (depth = 0; depth < 2; depth++) {
        lua_getstack(L, depth, &ar[depth]);
        lua_getinfo(L, "lnS", &ar[depth]);
    }
    name = ar[0].name;
    lineNo = ar[1].currentline;
    ownerName = LuaOwnerName(L);
    src = ar[1].short_src;
    if (strncmp(src, "string ", 7) == 0) {
        src = "[dostring] ";
    } else {
        while (1) {
            src = strchr(src, '\\');
            if (src == NULL) {
                src = ar[1].short_src;
                break;
            } else {
                src++;
            }
            if (_strnicmp(src, "scripts\\", 8) == 0) {
                src += 8;
                break;
            }
        }
    }
    _snprintf(buf, 100, "%s%d) %s %s", src, lineNo, name, ownerName);
    buf[strlen(src) - 1] = '(';
    buf[99] = 0;
    return buf;
}

bool CheckArgs(lua_State *L, int min, int max)
{
    int n = lua_gettop(L);
    if (n < min || n > max) {
        if (min == max)
            LuaCallSite(L);
        else
            LuaCallSite(L);
        return false;
    }
    return true;
}

const char *LuaOwnerName(lua_State *L)
{
    if (LUA_OWNER(L))
        return LUA_OWNER(L)->GetName();
    else
        return 0;
}

void ELuaFunction::RegisterAll(lua_State *L)
{
    ELuaFunction *f = s_first;
    int cnt;
    bool swapped = true;

    while (swapped) {
        ELuaFunction *prev;
        swapped = false;
        prev = 0;
        f = s_first;
        while (f->m_next) {
            if (_stricmp(f->m_name, f->m_next->m_name) > 0) {
                if (prev == 0) {
                    prev = s_first;
                    s_first = f->m_next;
                    prev->m_next = s_first->m_next;
                    s_first->m_next = prev;
                    prev = s_first;
                } else {
                    prev->m_next = f->m_next;
                    f->m_next = prev->m_next->m_next;
                    prev->m_next->m_next = f;
                    prev = prev->m_next;
                }
                swapped = true;
            } else {
                prev = f;
                f = f->m_next;
            }
        }
    }
    cnt = 0;
    f = s_first;
    while (f) {
        lua_pushcclosure(L, f->m_func, 0);
        lua_setglobal(L, f->m_name);
        f = f->m_next;
        cnt++;
    }
}

EScriptObject *GetScriptObjectById(lua_State *L, int id)
{
    if (LUA_OWNER(L) && LUA_OWNER(L)->GetId() == id)
        return LUA_OWNER(L);
    return GetScriptManager()->FindById(id);
}

#define IABS(x) (((x) & 0x80000000) ? -(x) : (x))

void DumpLuaStack(lua_State *L, int a, int b);

// a script object given as name, table, or id number (ids are 20 and up, smaller
// numbers are stack indices)
EScriptObject *GetScriptObjectArg(lua_State *L, int idx, bool optional, bool quiet)
{
    int i = idx;
    EScriptObject *o;

    if (IABS(i) < 20) {
        if (lua_type(L, i) == LUA_TSTRING) {
            const char *name = lua_tostring(L, i);
            EScriptObject *scriptObj = GetScriptManager()->FindByName(name);
            if (scriptObj == 0 && !quiet) {
                LuaCallSite(L);
                return 0;
            } else {
                return scriptObj;
            }
        } else {
            EScriptObject *obj;
            if (lua_type(L, i) == LUA_TNIL) {
                if (!optional && !quiet)
                    LuaCallSite(L);
                return 0;
            }
            if (lua_type(L, i) != LUA_TTABLE) {
                if (!quiet)
                    LuaCallSite(L);
                return 0;
            }
            lua_pushvalue(L, i);
            lua_pushstring(L, "m_pScriptObject");
            lua_gettable(L, -2);
            obj = 0;
            if (lua_type(L, -1) == LUA_TNUMBER) {
                obj = (EScriptObject *)lua_torawvalue(L, -1);
            } else {
                lua_settop(L, -3);
                return 0;
            }
            lua_settop(L, -3);
            if (obj == (EScriptObject *)1) {
                if (!quiet) {
                    const char *name;
                    lua_pushvalue(L, i);
                    lua_pushstring(L, "Name");
                    lua_gettable(L, -2);
                    name = lua_tostring(L, -1);
                    lua_settop(L, -3);
                }
                obj = 0;
            } else if (obj == 0 && !quiet) {
                LuaCallSite(L);
                DumpLuaStack(L, 0, 0);
            }
            return obj;
        }
    }
    if (idx == 0) {
        if (!optional && !quiet)
            LuaCallSite(L);
        return 0;
    }
    o = GetScriptObjectById(L, idx);
    if (o == 0 && !quiet)
        LuaCallSite(L);
    return o;
}

Vector3 GetVectorArg(lua_State *L, int idx)
{
    if (lua_type(L, idx) != LUA_TTABLE) {
        LuaCallSite(L);
        return Vector3(0.0f, 0.0f, 0.0f);
    }
    if (idx < 0)
        idx = idx + lua_gettop(L) + 1;
    lua_pushstring(L, "x");
    lua_rawget(L, idx);
    lua_pushstring(L, "y");
    lua_rawget(L, idx);
    lua_pushstring(L, "z");
    lua_rawget(L, idx);
    Vector3 v(lua_tonumber(L, -3), lua_tonumber(L, -2), lua_tonumber(L, -1));
    lua_settop(L, -4);
    return v;
}

bool GetObjectPosition(lua_State *L, int idx, Vector3 *out)
{
    if (CallLuaMethod(L, idx, "getPosition", 0, 0, 3, (float *)out)) {
        return true;
    } else {
        if (GetTableField(L, "xpos", idx)) {
            out->x = lua_tonumber(L, -1);
            lua_settop(L, -2);
            if (GetTableField(L, "ypos", idx)) {
                out->y = lua_tonumber(L, -1);
                lua_settop(L, -2);
                if (GetTableField(L, "zpos", idx)) {
                    out->z = lua_tonumber(L, -1);
                    lua_settop(L, -2);
                    return true;
                }
            }
        }
        lua_settop(L, -2);
    }
    *out = Vector3(0.0f, 0.0f, 0.0f);
    return false;
}

bool GetObjectOrientation(lua_State *L, int idx, Vector3 *out)
{
    if (CallLuaMethod(L, idx, "getOrientation", 0, 0, 3, (float *)out))
        return true;
    *out = Vector3(0.0f, 0.0f, 0.0f);
    return false;
}

bool CallLuaMethod(lua_State *L, int idx, const char *method, int nargs, const float *args, int nresults, float *results)
{
    int n = lua_gettop(L);
    float *dest;
    int iArg;
    int iRet;

    if (lua_type(L, idx) == LUA_TNIL)
        goto fail;
    dest = results;
    lua_pushvalue(L, idx);
    lua_pushstring(L, method);
    lua_gettable(L, -2);
    if (lua_type(L, -1) != LUA_TFUNCTION)
        goto fail;
    lua_pushvalue(L, -2);
    for (iArg = 0; iArg < nargs; iArg++)
        lua_pushnumber(L, args[iArg]);
    if (lua_call(L, nargs + 1, nresults) != 0)
        goto fail;
    for (iRet = 0; iRet < nresults; iRet++)
        dest[iRet] = lua_tonumber(L, -nresults + iRet);
    lua_settop(L, n);
    return true;
fail:
    lua_settop(L, n);
    return false;
}

EEntity *GetEntityArg(lua_State *L, int idx, bool optional, EClassInfo *cls)
{
    EEntity *ent = 0;
    int i = idx;
    if (IABS(i) < 20 && lua_type(L, i) == LUA_TNUMBER) {
        EEntity *e = (EEntity *)lua_torawvalue(L, i);
        if (e->IsDeleted())
            return 0;
        else
            return e;
    }
    EScriptObject *so = GetScriptObjectArg(L, idx, optional, false);
    if (so) {
        if (so->m_entity == 0) {
            LuaCallSite(L);
            so->GetName();
            return 0;
        }
        ent = so->m_entity;
    } else {
        int j = idx;
        if (IABS(j) < 20 && lua_isstring(L, j)) {
            const char *name = lua_tostring(L, j);
            ent = g_game->GetLevelManager()->GetLevel()->GetRoot()->FindEntity(name);
        } else {
            return 0;
        }
    }
    if (cls && ent) {
        if (!ent->IsA(cls)) {
            LuaCallSite(L);
            return 0;
        }
    }
    return ent;
}

ETriggerBase *GetTriggerArg(lua_State *L, int idx, bool optional)
{
    ETriggerBase *t = (ETriggerBase *)GetEntityArg(L, idx, optional, &ETriggerBase::s_class);
    return t;
}

void *GetTriggerA8Arg(lua_State *L, int idx, bool optional)
{
    ETriggerBase *t = GetTriggerArg(L, idx, optional);
    void *p;
    if (t == 0)
        return 0;
    p = *(void **)((char *)t + 0xa8);
    if (p == 0)
        LuaCallSite(L);
    return p;
}

float GetNumberArg(lua_State *L, int idx, bool optional)
{
    int type = lua_type(L, idx);
    if (optional && type == LUA_TNIL)
        return lua_tonumber(L, idx);
    if (type != LUA_TNUMBER && type != LUA_TSTRING) {
        LuaCallSite(L);
        lua_typename(L, type);
    }
    return lua_tonumber(L, idx);
}

const char *GetStringArg(lua_State *L, int idx, bool optional)
{
    int type = lua_type(L, idx);
    if (optional && type == LUA_TNIL)
        return lua_tostring(L, idx);
    if (type != LUA_TSTRING && type != LUA_TNIL) {
        LuaCallSite(L);
        lua_typename(L, type);
    }
    return lua_tostring(L, idx);
}

Color GetColorArg(lua_State *L, int idx, bool optional)
{
    Color c(GetNumberArg(L, idx, optional) * (1.0f / 255.0f),
            GetNumberArg(L, idx + 1, optional) * (1.0f / 255.0f),
            GetNumberArg(L, idx + 2, optional) * (1.0f / 255.0f),
            GetNumberArg(L, idx + 3, optional) * (1.0f / 255.0f));
    c.r = Clamp(c.r, 0.0f, 1.0f);
    c.g = Clamp(c.g, 0.0f, 1.0f);
    c.b = Clamp(c.b, 0.0f, 1.0f);
    c.a = Clamp(c.a, 0.0f, 1.0f);
    return c;
}

void PushVector(lua_State *L, const Vector3 &v)
{
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
}

void SetGlobalVector(lua_State *L, const Vector3 &v, const char *name)
{
    lua_getglobal(L, name);
    lua_pushstring(L, "x");
    lua_pushnumber(L, v.x);
    lua_rawset(L, -3);
    lua_pushstring(L, "y");
    lua_pushnumber(L, v.y);
    lua_rawset(L, -3);
    lua_pushstring(L, "z");
    lua_pushnumber(L, v.z);
    lua_rawset(L, -3);
}

bool PushEntity(lua_State *L, EEntity *ent, bool raw)
{
    if (ent && ent->GetScriptObject()) {
        lua_getref(L, ent->GetScriptObject()->m_ref);
        return true;
    } else if (raw) {
        lua_pushrawnumber(L, (int)ent);
        return true;
    } else {
        lua_pushnil(L);
        return false;
    }
}

bool PushScriptObject(lua_State *L, EScriptObject *o)
{
    if (o) {
        lua_getref(L, o->m_ref);
        return true;
    } else {
        lua_pushnil(L);
        return false;
    }
}

void *GetResourceArg(lua_State *L, int idx)
{
    if (IABS(idx) < 20) {
        if (lua_isrealnumber(L, idx)) {
            void *handle = lua_torawvalue(L, idx);
            return handle ? g_game->m_resources->Get(handle) : 0;
        } else if (lua_isstring(L, idx)) {
            const char *name = lua_tostring(L, idx);
            void *res = g_game->m_resources->Find(name);
            if (res == 0) {
                void *h = g_game->m_resources->Load(name, 0x50000, 0);
                if (h)
                    return g_game->m_resources->Get(h);
                else
                    return 0;
            } else {
                return res;
            }
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

bool GetTableField(lua_State *L, const char *key, int idx)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL)
        return false;
    return true;
}

void PushColor(lua_State *L, const Color &c)
{
    lua_pushnumber(L, c.r);
    lua_pushnumber(L, c.g);
    lua_pushnumber(L, c.b);
    lua_pushnumber(L, c.a);
}
