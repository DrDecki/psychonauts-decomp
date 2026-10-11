// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

static const float kDampingScale = 2.6f;
static const float kStiffnessScale = 80.0f;

class ETightropeEntity {
public:
    char pad0[0x110];
    bool m_bouncy;                  // 0x110
    char pad111[0x114 - 0x111];
    float m_mass;                   // 0x114
    float m_damping;                // 0x118
    float m_stiffness;              // 0x11c
    float m_120;
    float m_velX;                   // 0x124
    float m_128;
    float m_velY;                   // 0x12c

    static EClassInfo s_class;
};

// control handler 7
class ETightropeControlHandler : public EControlHandler {
public:
    bool IsFacingForward() { return m_facingForward; }

    char pad4[0x230 - 4];
    ETightropeEntity *m_rope;       // 0x230
    char pad234[0x240 - 0x234];
    bool m_facingForward;           // 0x240
};


LUA_FUNCTION(TightropeSetBouncy)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETightropeEntity *rope = (ETightropeEntity *)GetEntityArg(L, 1, false, &ETightropeEntity::s_class);
    bool bouncy = GetNumberArg(L, 2, false) != 0;
    rope->m_bouncy = bouncy;
    rope->m_120 = 0.0f;
    rope->m_128 = 0.0f;
    rope->m_velX = 0.0f;
    rope->m_velY = 0.0f;
    return 0;
}

LUA_FUNCTION(TightropeAddVel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    float f1 = GetNumberArg(L, 1, false);
    float f2 = GetNumberArg(L, 2, false);
    ETightropeControlHandler *pHandler = (ETightropeControlHandler *)g_game->GetControlHandler(7);
    ETightropeEntity *rope = pHandler->m_rope;
    if (!rope)
        return 0;
    if (!pHandler->IsFacingForward())
        f1 *= -1.0f;
    if (f1 == 0)
        rope->m_velX = 0.0f;
    else
        rope->m_velX += f1;
    if (f2 == 0)
        rope->m_velY = 0.0f;
    else
        rope->m_velY += f2;
    return 0;
}

LUA_FUNCTION(TightropeGetRope)
{
    ETightropeControlHandler *pHandler = (ETightropeControlHandler *)g_game->GetControlHandler(7);
    ETightropeEntity *rope = pHandler->m_rope;
    if (!rope)
        return 0;
    PushEntity(L, (EEntity *)rope, false);
    return 1;
}

LUA_FUNCTION(TightropeSetMass)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETightropeEntity *rope = (ETightropeEntity *)GetEntityArg(L, 1, false, &ETightropeEntity::s_class);
    if (!rope)
        return 0;
    float m = GetNumberArg(L, 2, false);
    if (m != 0)
        rope->m_mass = m;
    return 0;
}

LUA_FUNCTION(TightropeSetStiffness)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    ETightropeEntity *rope = (ETightropeEntity *)GetEntityArg(L, 1, false, &ETightropeEntity::s_class);
    if (!rope)
        return 0;
    float k = GetNumberArg(L, 2, false);
    float damp = GetNumberArg(L, 3, false);
    rope->m_stiffness = k * kStiffnessScale;
    rope->m_damping = damp * Sqrt(k) * kDampingScale;
    return 0;
}
