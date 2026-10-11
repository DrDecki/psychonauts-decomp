// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"


LUA_FUNCTION(GetSaveString)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    TArray<char> str(0x1a, false);
    g_game->m_save->GetSaveString(str);
    lua_pushstring(L, str.GetData());
    return 1;
}

LUA_FUNCTION(SetSteamRichPresenceStatus)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *szStatus = GetStringArg(L, 1, false);
    if (g_game)
        g_game->SetSteamRichPresence("status", szStatus);
    return 0;
}

LUA_FUNCTION(UnlockAchievement)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    if (g_game)
        g_game->UnlockAchievement(name);
    return 0;
}

LUA_FUNCTION(ResetSteamUserStats)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    lua_pushnil(L);
    return 1;
}
