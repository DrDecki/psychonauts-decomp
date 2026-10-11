// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class ERopeEntity {
public:
    void SetPoint(Vector3 *dst, Vector3 *src);

    char pad0[0x13c];
    Vector3 m_end;                  // 0x13c
    Vector3 m_start;                // 0x148
    float m_k;                      // 0x154
    float m_m;                      // 0x158
    float m_damp;                   // 0x15c

    static EClassInfo s_class;
};


LUA_FUNCTION(RopeSetStart)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ERopeEntity *rope = (ERopeEntity *)GetEntityArg(L, 1, false, &ERopeEntity::s_class);
    Vector3 vPos(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    if (rope)
        rope->SetPoint(&vPos, &rope->m_start);
    return 0;
}

LUA_FUNCTION(RopeSetEnd)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ERopeEntity *rope = (ERopeEntity *)GetEntityArg(L, 1, false, &ERopeEntity::s_class);
    Vector3 endPos(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    if (rope)
        rope->SetPoint(&rope->m_end, &endPos);
    return 0;
}

LUA_FUNCTION(RopeSetK)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ERopeEntity *rope = (ERopeEntity *)GetEntityArg(L, 1, false, &ERopeEntity::s_class);
    float k = GetNumberArg(L, 2, false);
    if (rope)
        rope->m_k = k;
    return 0;
}

LUA_FUNCTION(RopeSetM)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ERopeEntity *rope = (ERopeEntity *)GetEntityArg(L, 1, false, &ERopeEntity::s_class);
    float m = GetNumberArg(L, 2, false);
    if (rope)
        rope->m_m = m;
    return 0;
}

LUA_FUNCTION(RopeSetDamp)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ERopeEntity *rope = (ERopeEntity *)GetEntityArg(L, 1, false, &ERopeEntity::s_class);
    float damp = GetNumberArg(L, 2, false);
    if (rope)
        rope->m_damp = damp;
    return 0;
}
