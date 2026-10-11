// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"


LUA_FUNCTION(AddFrozen)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *szName = GetStringArg(L, 1, false);
    char *arg2 = n >= 2 ? (char *)GetStringArg(L, 2, true) : 0;
    EPhysics *hero = g_game->GetPlayerControl()->m_player;
    EMatrix *pMatrix = &hero->m_matrix;
    if (arg2)
        arg2 = StrDup(arg2, 0x1a);
    EQuat qOrient(*pMatrix);
    Vector3 localPos(0.0f, 20.0f, 200.0f);
    Vector3 spawnPos = pMatrix->TransformPoint(localPos);
    g_game->m_freeze->AddFrozen(szName, &arg2, g_game->m_unk81b4, spawnPos, qOrient);
    return 0;
}

LUA_FUNCTION(GetFreezeThawDists)
{
    lua_pushnumber(L, Sqrt(g_game->m_freeze->m_freezeDistSq));
    lua_pushnumber(L, Sqrt(g_game->m_freeze->m_thawDistSq));
    return 2;
}

LUA_FUNCTION(SetFreezeThawDists)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    float freezeDist = GetNumberArg(L, 1, false);
    freezeDist = Max(1.0f, freezeDist);
    g_game->m_freeze->m_thawDistSq = Square(freezeDist);
    g_game->m_freeze->m_freezeDistSq = Square(freezeDist);
    return 0;
}
