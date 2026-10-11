// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class EBallTrailEntity {
public:
    void SetTexture(void *tex);
    static EBallTrailEntity *Cast(EEntity *e) { return (EBallTrailEntity *)e; }

    static EClassInfo s_class;
};

class EBubbleEntity {
public:
    void SetTrail(EBallTrailEntity *trail);
    float GetAverageSpeed() { return m_averageSpeed; }

    char pad0[0x150];
    float m_averageSpeed;           // 0x150

    static EClassInfo s_class;
};


LUA_FUNCTION(BubbleSetSquish)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float amount = GetNumberArg(L, 1, false);
    g_game->GetPlayerControl()->GetBubble()->m_squish = amount;
    return 0;
}

LUA_FUNCTION(BubbleGetAverageSpeed_ForSpikyBall)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EBubbleEntity *bubble = (EBubbleEntity *)GetEntityArg(L, 1, false, &EBubbleEntity::s_class);
    lua_pushnumber(L, bubble->GetAverageSpeed());
    return 1;
}

LUA_FUNCTION(BubbleSetTrailEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EBubbleEntity *bubble = (EBubbleEntity *)GetEntityArg(L, 1, false, &EBubbleEntity::s_class);
    EBallTrailEntity *pTrail = 0;
    if (lua_type(L, 2) != LUA_TNIL)
        pTrail = EBallTrailEntity::Cast(GetEntityArg(L, 2, false, &EBallTrailEntity::s_class));
    bubble->SetTrail(pTrail);
    return 0;
}

LUA_FUNCTION(BubbleSetTrailTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EBallTrailEntity *pTrail = (EBallTrailEntity *)GetEntityArg(L, 1, false, &EBallTrailEntity::s_class);
    void **tex = (void **)GetResourceArg(L, 2);
    if (!pTrail || !tex)
        return 0;
    pTrail->SetTexture(*tex);
    return 0;
}
