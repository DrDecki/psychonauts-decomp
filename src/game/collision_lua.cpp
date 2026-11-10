// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

#include "classes.h"

extern bool g_disableOOWKludge;


LUA_FUNCTION_TODO(CheckForCollision)

LUA_FUNCTION_TODO(MultiCheckCollision)

LUA_FUNCTION_TODO(SetSwitchableCollisionEnable)

LUA_FUNCTION_TODO(SnapEntityToGround)

LUA_FUNCTION_TODO(GetEntityGround)

LUA_FUNCTION(GetEntityTouching)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EPhysics *phys = ent->m_physics;
    EEntity *myTouchingVal = phys->GetTouching(0);
    PushEntity(L, myTouchingVal, false);
    return 1;
}

LUA_FUNCTION(GetWallMaterial)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EPlayerControl *pc = g_game->m_playerControl;
    lua_pushnumber(L, pc->m_wallMaterial);
    return 1;
}

LUA_FUNCTION(IsSurfaceFlagSet)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned int theFlagVal = (unsigned int)GetNumberArg(L, 2, false);
    lua_pushuint(L, (ent->GetPhysics()->GetSurfaceFlags() & theFlagVal) != 0);
    return 1;
}

LUA_FUNCTION(GetGroundMaterial)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EPhysics *phys = ent->m_physics;
    lua_pushuint(L, phys->GetSurface().m_material);
    return 1;
}

LUA_FUNCTION(GetTriggerSurfaceNormal)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    ETriggerBase *trig = GetTriggerArg(L, 1, false);
    if (!trig)
        return 0;
    bool smooth = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    EPhysics *pPhys = trig->m_physics;
    Vector3 *aNormalObj = smooth ? &pPhys->m_smoothSurfaceNormal : &pPhys->m_surfaceNormal;
    PushVector(L, *aNormalObj);
    return 3;
}

LUA_FUNCTION(GetTriggerSurfaceDistance)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *trig = GetTriggerArg(L, 1, false);
    if (!trig)
        return 0;
    lua_pushnumber(L, trig->GetPhysics()->m_surfaceDistance);
    return 1;
}

LUA_FUNCTION(GetTriggerSurfaceNumber)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    ESurfaceInfo pSurface = ent->GetPhysics()->m_surface;
    lua_pushuint(L, pSurface.m_number);
    return 1;
}

LUA_FUNCTION(GetCollisionMaterialNumber)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    ESurfaceInfo pSurface = ent->GetPhysics()->m_surface;
    lua_pushuint(L, pSurface.m_material);
    return 0;
}

LUA_FUNCTION(SetEntityStepHeight)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float height = GetNumberArg(L, 2, false);
    theEnt->GetPhysics()->m_stepHeight = height;
    return 0;
}

LUA_FUNCTION_TODO(SetEntityCollideCylinder)

LUA_FUNCTION_TODO(SetEntityCollideSphere)

LUA_FUNCTION(GetEntityCollideSpheres)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EPhysics *phys = ent->m_physics;
    int theCount = phys->m_numSpheres;
    for (int i = 0; i < theCount; i++) {
        ECollideSphere *sphere = &phys->m_spheres[i];
        lua_pushnumber(L, sphere->m_radius);
        PushVector(L, sphere->m_center);
    }
    return theCount * 4;
}

LUA_FUNCTION(SetEntityCollideGeometry)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *trig = GetTriggerArg(L, 1, false);
    if (!trig)
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    if (!pMesh->HasCollideGeometry()) {
        LuaCallSite(L);
        return 0;
    }
    trig->GetPhysics()->SetCollideMode(4);
    return 0;
}

LUA_FUNCTION(HasCollideGeometry)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    lua_pushint(L, pMesh->HasCollideGeometry() != 0);
    return 1;
}

LUA_FUNCTION_TODO(SetEntitySplineRadius)

LUA_FUNCTION(GetEntityCollideType)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushint(L, ent->GetPhysics()->m_collideType);
    return 1;
}

LUA_FUNCTION(SetEntityCollideIgnoreEntity)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EEntity *other = GetEntityArg(L, 2, false, 0);
    if (!other)
        return 0;
    float newTimeVal = GetNumberArg(L, 3, false);
    EPhysics *phys = ent->m_physics;
    phys->m_ignoreEntity = other;
    phys->m_ignoreTime = newTimeVal;
    return 0;
}

LUA_FUNCTION_TODO(SetPhysicsState)

LUA_FUNCTION_TODO(GetPhysicsState)

LUA_FUNCTION(SetPhysicsFlag)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned int flag = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    bool on = GetNumberArg(L, 3, false) != 0;
    ent->GetPhysics()->SetMode(flag, on);
    return 0;
}

LUA_FUNCTION(GetPhysicsFlag)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned int flag = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    lua_pushint(L, ent->GetPhysics()->TestPhysFlag(flag) != 0);
    return 1;
}

LUA_FUNCTION(SetCollideLayer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    int theLayer = (int)GetNumberArg(L, 2, false);
    bool bOnVal = GetNumberArg(L, 3, false) != 0;
    unsigned int mask = 1 << (theLayer + 0x1b);
    if (!(mask & 0xf0000000))
        LuaCallSite(L);
    else
        ent->GetPhysics()->SetMode(mask, bOnVal);
    return 0;
}

LUA_FUNCTION(IgnoreCollideLayer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    int iLayer = (int)GetNumberArg(L, 2, false);
    bool bOnEnt = GetNumberArg(L, 3, false) != 0;
    unsigned int mask = 1 << (iLayer + 0x17);
    if (!(mask & 0xf000000))
        LuaCallSite(L);
    else
        ent->GetPhysics()->SetMode(mask, bOnEnt);
    return 0;
}

LUA_FUNCTION(MI_DisableFallOffBall)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool off = GetNumberArg(L, 1, false) != 0;
    EPlayerControl *pc = g_game->m_playerControl;
    pc->m_noFallOffBall = off;
    return 0;
}

LUA_FUNCTION(MM_DisableOOWKludge)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool off = GetNumberArg(L, 1, false) != 0;
    EPhysics *phys = g_game->GetPlayerControl()->GetPhysics();
    g_disableOOWKludge = off;
    return 0;
}

LUA_FUNCTION_TODO(CubeGravityForAll)

LUA_FUNCTION_TODO(GetPhysicsConstant)

LUA_FUNCTION_TODO(SetMass)

LUA_FUNCTION(SetPushParams)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned char priority = (unsigned char)GetNumberArg(L, 2, false);
    unsigned short speed = (unsigned short)GetNumberArg(L, 3, false);
    ent->GetPhysics()->m_pushPriority = priority;
    ent->GetPhysics()->m_pushSpeed = speed;
    return 0;
}

LUA_FUNCTION(GetPushParams)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushnumber(L, ent->GetPhysics()->m_pushPriority);
    lua_pushnumber(L, ent->GetPhysics()->m_pushSpeed);
    return 2;
}

LUA_FUNCTION_TODO(Rail_ShowClosestPoints)

LUA_FUNCTION_TODO(PrecomputeTrajectory)
