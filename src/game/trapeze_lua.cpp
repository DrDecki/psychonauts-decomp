// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class ETrapezePhysics {
public:
    float GetSwingSpeed();
    void SetSwingSpeed(float speed);

    char pad0[0x1c4];
    float m_maxEnergyPct;           // 0x1c4
};

class ETrapezeEntity {
public:
    ETrapezePhysics *GetPhysics() { return m_physics; }
    void Reset();

    char pad0[0x10];
    ETrapezePhysics *m_physics;     // 0x10

    static EClassInfo s_class;
};


LUA_FUNCTION(TrapezeGetSwingSpeed)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETrapezeEntity *trapeze = (ETrapezeEntity *)GetEntityArg(L, 1, false, &ETrapezeEntity::s_class);
    if (!trapeze)
        return 0;
    lua_pushnumber(L, trapeze->GetPhysics()->GetSwingSpeed());
    return 1;
}

LUA_FUNCTION(TrapezeSetSwingSpeed)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETrapezeEntity *trapeze = (ETrapezeEntity *)GetEntityArg(L, 1, false, &ETrapezeEntity::s_class);
    float rate = GetNumberArg(L, 2, false);
    if (!trapeze)
        return 0;
    trapeze->GetPhysics()->SetSwingSpeed(rate);
    return 0;
}

LUA_FUNCTION(TrapezeSetMaxEnergyPct)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETrapezeEntity *trapeze = (ETrapezeEntity *)GetEntityArg(L, 1, false, &ETrapezeEntity::s_class);
    float max = GetNumberArg(L, 2, false);
    if (!trapeze)
        return 0;
    max = Clamp(max, 0.1f, 2.0f);
    trapeze->GetPhysics()->m_maxEnergyPct = max;
    return 0;
}

LUA_FUNCTION(TrapezeReset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETrapezeEntity *trapeze = (ETrapezeEntity *)GetEntityArg(L, 1, false, &ETrapezeEntity::s_class);
    if (!trapeze)
        return 0;
    trapeze->Reset();
    return 0;
}
