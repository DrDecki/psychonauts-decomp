// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

extern bool g_rumbleEnabled;


LUA_FUNCTION(SetDeadZone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float zone = GetNumberArg(L, 1, false);
    return 0;
}

LUA_FUNCTION(GetDeadZone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    float zone = 0.0f;
    lua_pushnumber(L, zone);
    return 1;
}

LUA_FUNCTION(EnableRumble)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool enable = GetNumberArg(L, 1, false) != 0;
    if (enable)
        g_rumbleEnabled = true;
    else
        g_rumbleEnabled = false;
    return 0;
}

LUA_FUNCTION_TODO(RumbleSpeed)

LUA_FUNCTION_TODO(RumbleLeft)

LUA_FUNCTION_TODO(RumbleRight)

LUA_FUNCTION_TODO(RumbleLeftBezier)

LUA_FUNCTION_TODO(RumbleRightBezier)
