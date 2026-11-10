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

LUA_FUNCTION(RumbleSpeed)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    float myLeftObj = GetNumberArg(L, 1, false);
    float theRight = GetNumberArg(L, 2, false);
    if (!g_rumbleEnabled) {
        myLeftObj = 0.0f;
        theRight = 0.0f;
    }
    myLeftObj = myLeftObj < 0.0 ? 0.0f : myLeftObj;
    myLeftObj = myLeftObj > 1.0 ? 1.0f : myLeftObj;
    theRight = theRight < 0.0 ? 0.0f : theRight;
    theRight = theRight > 1.0 ? 1.0f : theRight;
    float t0 = 0.0f;
    float t1 = 0.333f;
    float t2 = 0.666f;
    float t3 = 1.0f;
    g_game->m_rumbleLeftStart = (float)g_game->GetTime() + t0;
    g_game->m_rumbleLeftEnd = (float)g_game->GetTime() + t3;
    g_game->m_rumbleLeftInvLength = 1.0f;
    g_game->m_rumbleLeftKeys[0].Set(t0, myLeftObj);
    g_game->m_rumbleLeftKeys[1].Set(t1, myLeftObj);
    g_game->m_rumbleLeftKeys[2].Set(t2, myLeftObj);
    g_game->m_rumbleLeftKeys[3].Set(t3, myLeftObj);
    g_game->m_rumbleRightStart = (float)g_game->GetTime() + t0;
    g_game->m_rumbleRightEnd = (float)g_game->GetTime() + t3;
    g_game->m_rumbleRightInvLength = 1.0f;
    g_game->m_rumbleRightKeys[0].Set(t0, theRight);
    g_game->m_rumbleRightKeys[1].Set(t1, theRight);
    g_game->m_rumbleRightKeys[2].Set(t2, theRight);
    g_game->m_rumbleRightKeys[3].Set(t3, theRight);
    return 0;
}

LUA_FUNCTION(RumbleLeft)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float curTimeObj = GetNumberArg(L, 1, false);
    float myFromArg = GetNumberArg(L, 2, false);
    float curToArg = GetNumberArg(L, 3, false);
    if (!g_rumbleEnabled) {
        curTimeObj = 1.0f;
        myFromArg = 0.0f;
        curToArg = 0.0f;
    }
    float curT = 0.0f;
    float myTEnt = curTimeObj * 0.333;
    float tVal = curTimeObj * 0.666;
    float newT = curTimeObj;
    float v0 = myFromArg;
    float v1 = (curToArg - myFromArg) * 0.333 + myFromArg;
    float v2 = (curToArg - myFromArg) * 0.666 + myFromArg;
    float curV = curToArg;
    float newNow = (float)g_game->GetTime();
    float start = newNow + curT;
    float curEnd = newNow + newT;
    float fLengthObj = curEnd - start;
    g_game->m_rumbleLeftStart = start;
    g_game->m_rumbleLeftEnd = curEnd;
    g_game->m_rumbleLeftKeys[0].Set(curT, v0);
    g_game->m_rumbleLeftKeys[1].Set(myTEnt, v1);
    g_game->m_rumbleLeftKeys[2].Set(tVal, v2);
    g_game->m_rumbleLeftKeys[3].Set(newT, curV);
    if (fLengthObj != 0.0)
        g_game->m_rumbleLeftInvLength = 1.0f / fLengthObj;
    else
        g_game->m_rumbleLeftInvLength = 1.0f;
    return 0;
}

LUA_FUNCTION(RumbleRight)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float curTimeObj = GetNumberArg(L, 1, false);
    float myFromArg = GetNumberArg(L, 2, false);
    float curToArg = GetNumberArg(L, 3, false);
    if (!g_rumbleEnabled) {
        curTimeObj = 1.0f;
        myFromArg = 0.0f;
        curToArg = 0.0f;
    }
    float curT = 0.0f;
    float myTEnt = curTimeObj * 0.333;
    float tVal = curTimeObj * 0.666;
    float newT = curTimeObj;
    float v0 = myFromArg;
    float v1 = (curToArg - myFromArg) * 0.333 + myFromArg;
    float v2 = (curToArg - myFromArg) * 0.666 + myFromArg;
    float curV = curToArg;
    float newNow = (float)g_game->GetTime();
    float start = newNow + curT;
    float curEnd = newNow + newT;
    float fLengthObj = curEnd - start;
    g_game->m_rumbleRightStart = start;
    g_game->m_rumbleRightEnd = curEnd;
    g_game->m_rumbleRightKeys[0].Set(curT, v0);
    g_game->m_rumbleRightKeys[1].Set(myTEnt, v1);
    g_game->m_rumbleRightKeys[2].Set(tVal, v2);
    g_game->m_rumbleRightKeys[3].Set(newT, curV);
    if (fLengthObj != 0.0)
        g_game->m_rumbleRightInvLength = 1.0f / fLengthObj;
    else
        g_game->m_rumbleRightInvLength = 1.0f;
    return 0;
}

LUA_FUNCTION(RumbleLeftBezier)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 8, 8))
        return 0;
    float curT = GetNumberArg(L, 1, false);
    float v0 = GetNumberArg(L, 2, false);
    float myTEnt = GetNumberArg(L, 3, false);
    float v1 = GetNumberArg(L, 4, false);
    float tVal = GetNumberArg(L, 5, false);
    float v2 = GetNumberArg(L, 6, false);
    float newT = GetNumberArg(L, 7, false);
    float curV = GetNumberArg(L, 8, false);
    if (!g_rumbleEnabled) {
        curT = 0.0f;
        v0 = 0.0f;
        myTEnt = 0.0f;
        v1 = 0.0f;
        tVal = 0.0f;
        v2 = 0.0f;
        newT = 0.0f;
        curV = 0.0f;
    }
    float newNow = (float)g_game->GetTime();
    float start = newNow + curT;
    float curEnd = newNow + newT;
    float theLength = curEnd - start;
    g_game->m_rumbleLeftStart = start;
    g_game->m_rumbleLeftEnd = curEnd;
    g_game->m_rumbleLeftKeys[0].Set(curT, v0);
    g_game->m_rumbleLeftKeys[1].Set(myTEnt, v1);
    g_game->m_rumbleLeftKeys[2].Set(tVal, v2);
    g_game->m_rumbleLeftKeys[3].Set(newT, curV);
    if (theLength != 0.0)
        g_game->m_rumbleLeftInvLength = 1.0f / theLength;
    else
        g_game->m_rumbleLeftInvLength = 1.0f;
    return 0;
}

LUA_FUNCTION(RumbleRightBezier)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 8, 8))
        return 0;
    float curT = GetNumberArg(L, 1, false);
    float v0 = GetNumberArg(L, 2, false);
    float myTEnt = GetNumberArg(L, 3, false);
    float v1 = GetNumberArg(L, 4, false);
    float tVal = GetNumberArg(L, 5, false);
    float v2 = GetNumberArg(L, 6, false);
    float newT = GetNumberArg(L, 7, false);
    float curV = GetNumberArg(L, 8, false);
    if (!g_rumbleEnabled) {
        curT = 0.0f;
        v0 = 0.0f;
        myTEnt = 0.0f;
        v1 = 0.0f;
        tVal = 0.0f;
        v2 = 0.0f;
        newT = 0.0f;
        curV = 0.0f;
    }
    float newNow = (float)g_game->GetTime();
    float start = newNow + curT;
    float curEnd = newNow + newT;
    float theLength = curEnd - start;
    g_game->m_rumbleRightStart = start;
    g_game->m_rumbleRightEnd = curEnd;
    g_game->m_rumbleRightKeys[0].Set(curT, v0);
    g_game->m_rumbleRightKeys[1].Set(myTEnt, v1);
    g_game->m_rumbleRightKeys[2].Set(tVal, v2);
    g_game->m_rumbleRightKeys[3].Set(newT, curV);
    if (theLength != 0.0)
        g_game->m_rumbleRightInvLength = 1.0f / theLength;
    else
        g_game->m_rumbleRightInvLength = 1.0f;
    return 0;
}
