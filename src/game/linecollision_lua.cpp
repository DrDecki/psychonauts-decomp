// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

#include "classes.h"

// the points the scripts collect between StartLineCollision and EndLineCollision
extern TArray<Vector3> g_linePoints;
extern TArray<Vector3> g_lineNormals;


LUA_FUNCTION(StartLineCollision)
{
    g_linePoints.Clear();
    g_lineNormals.Clear();
    return 0;
}

LUA_FUNCTION_TODO(AddLineCollision)

LUA_FUNCTION(EndLineCollision)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    float fRadius = GetNumberArg(L, 2, false);
    ent->m_splineCollision = new ESplineCollision(ent);
    ent->m_splineCollision->m_radius = fRadius;
    ent->m_splineCollision->createFromPoints(&g_linePoints, &g_lineNormals, 3, 0, 0);
    ((EEntityVirtuals *)ent)->UpdateCollision();
    return 0;
}

LUA_FUNCTION_TODO(CreateTightropeCollision)

LUA_FUNCTION_TODO(Spline_AddSkip)

LUA_FUNCTION(Spline_ClearSkips)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    ESplineCollision *spline = ent->m_splineCollision;
    if (!spline)
        return 0;
    spline->clearSkips();
    return 0;
}

LUA_FUNCTION(Spline_ParametricToPoint)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    float t = GetNumberArg(L, 2, false);
    ESplineCollision *spline = ent->m_splineCollision;
    if (!spline)
        return 0;
    Vector3 point = spline->parametricToPoint(t, 0);
    PushVector(L, point);
    return 3;
}

LUA_FUNCTION_TODO(Spline_GetRight)

LUA_FUNCTION_TODO(Spline_FindClosestTo)

LUA_FUNCTION_TODO(Spline_FindClosest)
