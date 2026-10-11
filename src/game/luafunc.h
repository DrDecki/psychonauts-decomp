// Script functions: every C function the Lua scripts can call is declared
// with one of the macros below. They register the function under its own
// name, and most of them also run it on a separate stack.
//
// The class and the macros are reconstructed from the code they generate,
// their real names are lost.

#ifndef LUAFUNC_H
#define LUAFUNC_H

struct lua_State;
typedef int (*lua_CFunction)(lua_State *L);

class ELuaFunction
{
public:
    ELuaFunction(lua_CFunction func, const char *name);
    static void RegisterAll(lua_State *L);

    lua_CFunction m_func;
    const char *m_name;
    ELuaFunction *m_next;

    static ELuaFunction *s_first;
    static int s_count;
};

// stack the script functions run on, see LUA_FUNCTION
extern void *g_luaStack;
extern bool g_onLuaStack;

// A script function that switches to g_luaStack (unless it is already on
// it) and calls name_impl. The body that follows is name_impl.
#define LUA_FUNCTION_WRAPPER(name) \
    int name(lua_State *L) \
    { \
        void *savedEsp = 0; \
        int ret; \
        if (!g_onLuaStack && g_luaStack) { \
            __asm { mov savedEsp, esp } \
            __asm { mov esp, g_luaStack } \
            g_onLuaStack = true; \
        } \
        ret = name##_impl(L); \
        if (savedEsp) { \
            __asm { mov esp, savedEsp } \
            savedEsp = 0; \
            g_onLuaStack = false; \
        } \
        return ret; \
    } \
    static ELuaFunction name##_reg(name, #name);

#define LUA_FUNCTION(name) \
    static int name##_impl(lua_State *L); \
    LUA_FUNCTION_WRAPPER(name) \
    static int name##_impl(lua_State *L)

// Same, for functions whose body is not decompiled yet.
#define LUA_FUNCTION_TODO(name) \
    int name##_impl(lua_State *L); \
    LUA_FUNCTION_WRAPPER(name)

// A script function that runs on the normal stack, mostly ones that can yield.
#define LUA_FUNCTION_NOSTACK(name) \
    int name(lua_State *L); \
    static ELuaFunction name##_reg(name, #name); \
    int name(lua_State *L)

#define LUA_FUNCTION_NOSTACK_TODO(name) \
    int name(lua_State *L); \
    static ELuaFunction name##_reg(name, #name);

#endif

class EScriptObject;
const char *LuaCallSite(lua_State *L);
bool CheckArgs(lua_State *L, int min, int max);
const char *LuaOwnerName(lua_State *L);
EScriptObject *GetScriptObjectById(lua_State *L, int id);
EScriptObject *GetScriptObjectArg(lua_State *L, int idx, bool optional, bool quiet);
class EEntity;
class EClassInfo;
class ETriggerBase;
struct Vector3;
struct Color;
Vector3 GetVectorArg(lua_State *L, int idx);
bool CallLuaMethod(lua_State *L, int idx, const char *method, int nargs, const float *args, int nresults, float *results);
bool GetObjectPosition(lua_State *L, int idx, Vector3 *out);
bool GetObjectOrientation(lua_State *L, int idx, Vector3 *out);
EEntity *GetEntityArg(lua_State *L, int idx, bool optional, EClassInfo *cls);
ETriggerBase *GetTriggerArg(lua_State *L, int idx, bool optional);
void *GetTriggerA8Arg(lua_State *L, int idx, bool optional);
float GetNumberArg(lua_State *L, int idx, bool optional);
const char *GetStringArg(lua_State *L, int idx, bool optional);
Color GetColorArg(lua_State *L, int idx, bool optional);
void PushVector(lua_State *L, const Vector3 &v);
void SetGlobalVector(lua_State *L, const Vector3 &v, const char *name);
bool PushEntity(lua_State *L, EEntity *ent, bool raw);
bool PushScriptObject(lua_State *L, EScriptObject *o);
void *GetResourceArg(lua_State *L, int idx);
bool GetTableField(lua_State *L, const char *key, int idx);
void PushColor(lua_State *L, const Color &c);
