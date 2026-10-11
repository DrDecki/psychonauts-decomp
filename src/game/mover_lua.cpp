// Script functions, file name made up.

#include <malloc.h>
#include "engine.h"
#include "luafunc.h"

// rigid body affectors, RTTI names
class ERBAffector {
public:
    void *operator new(size_t size) { return _aligned_malloc(size, 16); }
    ERBAffector() {}
    virtual void Vf00();
};

class ERBConstantForce : public ERBAffector {
public:
    ERBConstantForce() : m_force(g_zeroVector) {}
    virtual void Vf00();

    Vector3 m_force;                // 0x04
};

class ERBSpring : public ERBAffector {
public:
    char pad4[0x18 - 4];
    Vector3 m_worldPos;             // 0x18
};

class ERBTorqueSpring : public ERBAffector {
public:
    char pad4[0xc - 4];
    EQuat m_restRot;                // 0x0c
};

class ERBSystem {
public:
    void Reset();
    int Add(ERBAffector *affector);

    char pad0[0x98];
    ERBAffector **m_affectors;      // 0x98
};

class EMoverPhysics {
public:
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void SetMass(float mass);   // 0x14
    void Reset();
    void CalculateMassDistribution(int mode);

    char pad4[0x18c - 4];
    ERBSystem m_rb;                 // 0x18c
    bool m_debug;                   // 0x228, right after m_rb
};

class EMoverEntity {
public:
    EMoverPhysics *GetPhysics() { return m_physics; }

    char pad0[0x10];
    EMoverPhysics *m_physics;       // 0x10

    static EClassInfo s_class;
};


LUA_FUNCTION(MoverReset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    if (!mover)
        return 0;
    mover->GetPhysics()->Reset();
    ERBSystem *pSys = &mover->GetPhysics()->m_rb;
    pSys->Reset();
    return 0;
}

LUA_FUNCTION(MoverSetMass)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    float m = GetNumberArg(L, 2, false);
    if (!mover)
        return 0;
    mover->GetPhysics()->SetMass(m);
    return 0;
}

LUA_FUNCTION(MoverCalculateMassDistribution)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    int k = (int)GetNumberArg(L, 2, false);
    if (!mover)
        return 0;
    mover->GetPhysics()->CalculateMassDistribution(k);
    return 0;
}

LUA_FUNCTION_TODO(MoverAddSpring)

LUA_FUNCTION(MoverSetSpringWorldPos)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    if (!mover)
        return 0;
    ERBSystem *pSys = &mover->GetPhysics()->m_rb;
    int k = (int)GetNumberArg(L, 2, false);
    Vector3 goalPos(GetNumberArg(L, 3, false), GetNumberArg(L, 4, false), GetNumberArg(L, 5, false));
    ERBSpring *pSpring = (ERBSpring *)pSys->m_affectors[k];
    pSpring->m_worldPos = goalPos;
    return 0;
}

LUA_FUNCTION_TODO(MoverAddTorqueSpring)

LUA_FUNCTION(MoverSetTorqueSpringRestOrientation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    if (!mover)
        return 0;
    ERBSystem *pSys = &mover->GetPhysics()->m_rb;
    int k = (int)GetNumberArg(L, 2, false);
    Vector3 rotAngles(GetNumberArg(L, 3, false), GetNumberArg(L, 4, false), GetNumberArg(L, 5, false));
    ERBTorqueSpring *pSpring = (ERBTorqueSpring *)pSys->m_affectors[k];
    EQuat restRot(0.0f, 0.0f, 0.0f, 1.0f);
    restRot.SetEuler(rotAngles);
    pSpring->m_restRot = restRot;
    return 0;
}

LUA_FUNCTION(MoverAddForceAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    if (!mover)
        return 0;
    ERBSystem *pSys = &mover->GetPhysics()->m_rb;
    int nId = pSys->Add(new ERBConstantForce());
    lua_pushint(L, nId);
    return 1;
}

LUA_FUNCTION_TODO(MoverAddWaterSpring)

LUA_FUNCTION_TODO(RelativeRazPos)

LUA_FUNCTION(MoverSetDebug)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMoverEntity *mover = (EMoverEntity *)GetEntityArg(L, 1, false, &EMoverEntity::s_class);
    bool bDebug = GetNumberArg(L, 2, false) != 0;
    mover->GetPhysics()->m_debug = bDebug;
    return 0;
}
