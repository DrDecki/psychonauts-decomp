// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

#include "classes.h"

// Reads the waypoints from the table at idx and makes a path of them (or
// changes the path ref). Returns the reference of the path.
unsigned int BuildSplinePath(lua_State *L, int idx, bool closed, bool smooth, unsigned int ref);
void ScriptError(const char *fmt, ...);

extern float *g_gravity;

static const float kMinTime = 0.00001f;


LUA_FUNCTION(CreateSplinePath)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    int type = (int)GetNumberArg(L, 1, false);
    bool smooth = nN >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    unsigned int iRef = BuildSplinePath(L, 2, type == 1, smooth, 0);
    lua_pushrawnumber(L, iRef);
    return 1;
}

LUA_FUNCTION(AdjustSplinePath)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    unsigned int ref = (unsigned int)lua_torawvalue(L, 1);
    bool closed = GetNumberArg(L, 2, false) != 0;
    if (!ref)
        return 0;
    ESplinePath *path = g_game->m_pathManager->getPath(ref);
    if (!path)
        return 0;
    bool smooth = path->m_numNormals > 0;
    BuildSplinePath(L, 3, closed, smooth, ref);
    return 0;
}

LUA_FUNCTION(CreateGravitySpline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 8, 11))
        return 0;
    int iCount = (int)GetNumberArg(L, 1, false);
    Vector3 pStartVal(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    Vector3 end(GetNumberArg(L, 5, false), GetNumberArg(L, 6, false), GetNumberArg(L, 7, false));
    float time = GetNumberArg(L, 8, false);
    Vector3 up = n >= 9 ? Vector3(GetNumberArg(L, 9, true), GetNumberArg(L, 10, true), GetNumberArg(L, 11, true)) : Vector3(0, 1, 0);
    if (time < kMinTime) {
        ScriptError("Zero time not allowed in CreateGravitySpline");
        return 0;
    }
    unsigned int myRef = g_game->GetPathManager()->getNewPath();
    if (myRef) {
        ESplinePath *path = g_game->GetPathManager()->getPathByReference(myRef);
        path->createGravityPath(iCount, pStartVal, end, time, *g_gravity, up);
    }
    lua_pushrawnumber(L, myRef);
    return 1;
}

LUA_FUNCTION(CreateRelativePath)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float fSecs = GetNumberArg(L, 2, false);
    int aModeVal = (int)GetNumberArg(L, 3, false);
    if (fSecs == 0) {
        LuaCallSite(L);
        lua_pushnumber(L, 0);
        return 1;
    }
    unsigned int ref = BuildSplinePath(L, 4, aModeVal == 1, false, 0);

    ESplineController *ctrl = theEnt->getSplineController();
    ref = g_game->GetPathManager()->getNewPath();
    ESplinePath *path = g_game->GetPathManager()->getPathByReference(ref);
    if (path) {
        ctrl->setPath(path);
        ctrl->init(aModeVal, fSecs);
    }

    ctrl->setSplineOrigin(theEnt->GetPosition());
    ctrl->setRelativeSplineFlag(true);
    lua_pushrawnumber(L, ref);
    return 1;
}

LUA_FUNCTION(CreateChasePath)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 8, 8))
        return 0;
    EEntity *newEntVal = GetEntityArg(L, 1, false, 0);
    if (!newEntVal)
        return 0;
    int myTop = lua_gettop(L);
    float fSecs = lua_tonumber(L, 3);
    int nModeVal = (int)lua_tonumber(L, 4);
    float myDistPtr = lua_tonumber(L, 6);
    float fMinVel = lua_tonumber(L, 7);
    float maxVel = lua_tonumber(L, 8);
    if (fSecs == 0) {
        LuaCallSite(L);
        lua_pushnumber(L, 0);
        return 1;
    }
    unsigned int myRef = BuildSplinePath(L, 5, nModeVal == 1, false, 0);
    if (!myRef) {
        lua_pushnumber(L, 0);
        return 1;
    }
    ESplineController *ctrl = newEntVal->getSplineController();
    ESplinePath *path = g_game->GetPathManager()->getPathByReference(myRef);
    if (path) {
        ctrl->setPath(path);
        ctrl->init(nModeVal, fSecs);
    }

    EEntity *target = GetEntityArg(L, 2, false, 0);
    ctrl->setSplineEntity(target);
    ctrl->setSplineOrigin(target->GetPosition());
    ctrl->setSplineChaseDistance(myDistPtr);
    ctrl->setSplineChaseVelocity(fMinVel, maxVel);
    ctrl->setRelativeSplineFlag(true);
    lua_pushrawnumber(L, myRef);
    return 1;
}

LUA_FUNCTION(CreateAbsolutePath)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float fSecs = GetNumberArg(L, 2, false);
    int aModeVal = (int)GetNumberArg(L, 3, false);
    if (fSecs == 0) {
        LuaCallSite(L);
        lua_pushnumber(L, 0);
        return 1;
    }
    unsigned int ref = BuildSplinePath(L, 4, aModeVal == 1, false, 0);

    if (!ref) {
        lua_pushnumber(L, 0);
        return 1;
    }
    ESplineController *ctrl = theEnt->getSplineController();
    ESplinePath *path = g_game->GetPathManager()->getPathByReference(ref);
    if (path) {
        ctrl->setPath(path);
        ctrl->init(aModeVal, fSecs);
    }

    ctrl->setRelativeSplineFlag(false);
    lua_pushrawnumber(L, ref);
    return 1;
}

LUA_FUNCTION(CreateLocatorPath)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float fSecs = GetNumberArg(L, 2, false);
    int aModeVal = (int)GetNumberArg(L, 3, false);
    if (fSecs == 0) {
        LuaCallSite(L);
        lua_pushnumber(L, 0);
        return 1;
    }
    unsigned int ref = BuildSplinePath(L, 4, aModeVal == 1, false, 0);

    if (!ref) {
        lua_pushnumber(L, 0);
        return 1;
    }
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl) {
        lua_pushnumber(L, 0);
        return 1;
    }
    ESplinePath *path = g_game->GetPathManager()->getPathByReference(ref);
    if (path) {
        ctrl->setPath(path);
        ctrl->init(aModeVal, fSecs);
    }

    ctrl->setRelativeSplineFlag(false);
    lua_pushrawnumber(L, ref);
    return 1;
}

LUA_FUNCTION(UsePath)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    unsigned int aRef = (unsigned int)lua_torawvalue(L, 1);
    EEntity *ent = GetEntityArg(L, 2, false, 0);
    if (!ent)
        return 0;
    float secsVal = GetNumberArg(L, 3, false);
    int newMode = (int)GetNumberArg(L, 4, false);
    if (secsVal == 0) {
        LuaCallSite(L);
        return 0;
    }
    ESplineController *ctrl = ent->getSplineController();
    if (!ctrl)
        return 0;
    if (!aRef) {
        LuaCallSite(L);
        return 0;
    }
    ESplinePath *path = g_game->m_pathManager->getPath(aRef);
    if (path) {
        ctrl->setPath(path);
        ctrl->init(newMode, secsVal);
        ctrl->setRelativeSplineFlag(false);
    } else
        LuaCallSite(L);
    return 0;
}

LUA_FUNCTION(AttachToSpline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    int myTop = lua_gettop(L);
    EEntity *leaderVal = GetEntityArg(L, 1, false, 0);
    if (!leaderVal)
        return 0;
    EEntity *theEntVal = GetEntityArg(L, 2, false, 0);
    if (!theEntVal)
        return 0;
    float secs = GetNumberArg(L, 3, false);
    int newMode = (int)GetNumberArg(L, 4, false);
    ESplineController *pFrom = leaderVal->getSplineController();
    ESplineController *ctrl = theEntVal->getSplineController();
    if (!pFrom)
        return 0;
    if (!ctrl)
        return 0;
    ESplinePath *path = pFrom->getSplinePath();
    if (path) {
        ctrl->setPath(path);
        ctrl->init(newMode, secs);
    }
    return 0;
}

LUA_FUNCTION(FollowOnSpline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *leaderVal = GetEntityArg(L, 1, false, 0);
    if (!leaderVal)
        return 0;
    EEntity *curEnt = GetEntityArg(L, 2, false, 0);
    if (!curEnt)
        return 0;
    float fOffsetObj = GetNumberArg(L, 3, false);
    ESplineController *pFrom = leaderVal->getSplineController();
    ESplineController *ctrl = curEnt->getSplineController();
    if (!pFrom)
        return 0;
    if (!ctrl)
        return 0;
    ESplinePath *path = pFrom->getSplinePath();
    if (path) {
        ctrl->setPath(path);
        ctrl->init(pFrom->splineLoopMode(), pFrom->splineTraversalPeriod());
        ctrl->setOffset(fOffsetObj);
        // Some inline call that leaves no code was here, the stack layout shows it. Which one is a guess.
        ctrl->setSync(false);
        ctrl->setFormationFlag(false);
        ctrl->setFollowSplineFlag(true);
        ctrl->setActive(true);
        ctrl->setExternalController(leaderVal);
    }
    return 0;
}

LUA_FUNCTION(FormationOnSpline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    int myTop = lua_gettop(L);
    EEntity *leaderVal = GetEntityArg(L, 1, false, 0);
    if (!leaderVal)
        return 0;
    EEntity *theEnt = GetEntityArg(L, 2, false, 0);
    if (!theEnt)
        return 0;
    float fOffsetObj = GetNumberArg(L, 3, false);
    float x = GetNumberArg(L, 4, false);
    float newY = GetNumberArg(L, 5, false);
    float z = GetNumberArg(L, 6, false);
    ESplineController *pFrom = leaderVal->getSplineController();
    ESplineController *aCtrlVal = theEnt->getSplineController();
    if (!pFrom)
        return 0;
    if (!aCtrlVal)
        return 0;
    ESplinePath *path = pFrom->getSplinePath();
    if (path) {
        aCtrlVal->setPath(path);
        aCtrlVal->init(pFrom->splineLoopMode(), pFrom->splineTraversalPeriod());
        aCtrlVal->setOffset(fOffsetObj);
        aCtrlVal->setFormationPosition(x, newY, z);
        aCtrlVal->setFormationFlag(true);
        aCtrlVal->setFollowSplineFlag(true);
        aCtrlVal->setActive(true);
        aCtrlVal->setExternalController(leaderVal);
    }
    return 0;
}

LUA_FUNCTION(AnimateOnSpline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int top = lua_gettop(L);
    EEntity *leader = GetEntityArg(L, 1, false, 0);
    if (!leader)
        return 0;
    EEntity *myEnt = GetEntityArg(L, 2, false, 0);
    if (!myEnt)
        return 0;
    ESplineController *pFrom = leader->getSplineController();
    ESplineController *aCtrlVal = myEnt->getSplineController();
    if (!pFrom)
        return 0;
    if (!aCtrlVal)
        return 0;
    ESplinePath *path = pFrom->getSplinePath();
    if (path) {
        aCtrlVal->setAnimationFlag(true);
        aCtrlVal->setFollowSplineFlag(false);
        aCtrlVal->setExternalController(0);
    }
    return 0;
}

LUA_FUNCTION(DetachFromSpline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    ESplineController *aCtrl = ent->getSplineController();
    if (!aCtrl)
        return 0;
    ESplinePath *path = aCtrl->getSplinePath();
    if (path) {
        aCtrl->setActive(false);
        aCtrl->setFollowSplineFlag(false);
        aCtrl->setFormationFlag(false);
        aCtrl->setAnimationFlag(false);
        aCtrl->setExternalController(0);
        aCtrl->setPath((unsigned int)0);
        ent->GetPhysics()->ResetSplineDelta();
    }
    return 0;
}

LUA_FUNCTION(SplineInfo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    if (!theEnt)
        return 0;
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    int newMode = ctrl->m_loopMode;
    float dir = ctrl->m_direction;
    float newSpeedVal = ctrl->m_speed;
    float secs = 1.0f / ctrl->m_period;
    lua_pushint(L, newMode);
    lua_pushnumber(L, dir);
    lua_pushnumber(L, newSpeedVal);
    lua_pushnumber(L, secs);
    return 4;
}

LUA_FUNCTION(SplineCharacter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setCharacterSplineFlag(on);
    return 0;
}

LUA_FUNCTION(SplineDisplay)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setDisplaySplineFlag(on);
    return 0;
}

LUA_FUNCTION(SplineDisplayUp)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setDisplaySplineUpFlag(on);
    return 0;
}

LUA_FUNCTION(SplineDisplayAxis)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setDisplaySplineAxisFlag(on);
    return 0;
}

LUA_FUNCTION(SplineFwdOnlyFlag)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setFwdOrientateSplineFlag(on);
    return 0;
}

LUA_FUNCTION(SplineOrientationFlag)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setOrientateWithSplineFlag(on);
    return 0;
}

LUA_FUNCTION(SplinePeriodInSecs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    float secs = GetNumberArg(L, 2, false);
    if (secs == 0) {
        LuaCallSite(L);
        return 0;
    }
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setSplineTraversalPeriod(secs);
    return 0;
}

LUA_FUNCTION(SplineSpeedRel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float speedVal = GetNumberArg(L, 2, false);
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setSplineRelSpeed(speedVal);
    return 0;
}

LUA_FUNCTION(SplineSpeedAbs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float speedVal = lua_tonumber(L, 2) * 100.0;
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setSplineAbsSpeed(speedVal);
    return 0;
}

LUA_FUNCTION(SplineOrigin)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    Vector3 origin(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setSplineOrigin(origin);
    return 0;
}

LUA_FUNCTION(SplineChase)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EEntity *curTarget = GetEntityArg(L, 2, true, 0);
    float dist = GetNumberArg(L, 3, false);
    float curMinVel = GetNumberArg(L, 4, false);
    float theMaxVel = GetNumberArg(L, 5, false);
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setSplineEntity(curTarget);
    pCtrlVal->setSplineChaseDistance(dist);
    pCtrlVal->setSplineChaseVelocity(curMinVel, theMaxVel);
    return 0;
}

LUA_FUNCTION(SplinePositionParametric)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float t = GetNumberArg(L, 2, false);
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    if (!ctrl->m_path)
        return 0;
    ctrl->setSplineTraversalPosition(t);
    return 0;
}

LUA_FUNCTION(SplinePositionAtWaypoint)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 2, false);
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setSplineTraversalToWaypoint(index);
    return 0;
}

LUA_FUNCTION(SplinePositionInMeters)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    float meters = GetNumberArg(L, 2, false);
    ESplineController *pCtrlVal = ent->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setSplineTraversalPositionInMeters(meters);
    return 0;
}

LUA_FUNCTION(SplineDirection)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *entArg = GetEntityArg(L, 1, false, 0);
    if (!entArg)
        return 0;
    float dir = GetNumberArg(L, 2, false);
    if (dir < 0)
        dir = -1.0f;
    else
        dir = 1.0f;
    ESplineController *ctrl = entArg->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setSplineTraversalDirection(dir);
    return 0;
}

LUA_FUNCTION(SplineLoopMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    unsigned int newMode = (unsigned int)GetNumberArg(L, 2, false);
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setSplineLoopMode(newMode);
    return 0;
}

LUA_FUNCTION(SplineBanking)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float factor = GetNumberArg(L, 2, false);
    ESplineController *pCtrlVal = theEnt->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setSplineRollFactor(factor);
    return 0;
}

LUA_FUNCTION(SplineUnidirectional)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    float dir = GetNumberArg(L, 2, false);
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setSplineUniDirection(dir);
    return 0;
}

LUA_FUNCTION(SplineUniformSubdivision)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    int iCount = (int)GetNumberArg(L, 2, false);
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->SplineUniformSubdivision(iCount);
    return 0;
}

LUA_FUNCTION(SplineAdaptiveSubdivision)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *myEnt = GetEntityArg(L, 1, false, 0);
    if (!myEnt)
        return 0;
    int iCount = (int)GetNumberArg(L, 2, false);
    float theTolerance = GetNumberArg(L, 3, false);
    ESplineController *ctrl = myEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->SplineUniformSubdivision(iCount);
    return 0;
}

LUA_FUNCTION(SplineConformJoints)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setConformJointsFlag(on);
    return 0;
}

LUA_FUNCTION(SplineForceMove)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setForceMoveFlag(on);
    return 0;
}

LUA_FUNCTION(SplineEaseInOut)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    EEntity *entArg = GetEntityArg(L, 1, false, 0);
    if (!entArg)
        return 0;
    float in = GetNumberArg(L, 2, false);
    float theOut = GetNumberArg(L, 3, false);
    bool smooth = n >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    ESplineController *pCtrlVal = entArg->getSplineController();
    if (!pCtrlVal)
        return 0;
    pCtrlVal->setSplineEaseInOut(in, theOut, smooth);
    return 0;
}

LUA_FUNCTION(SplineWorldOffset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    Vector3 offset(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setWorldOffset(offset);
    return 0;
}

LUA_FUNCTION(SplineEnd)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    ctrl->setActive(false);
    ctrl->setFormationFlag(false);
    ctrl->setAnimationFlag(false);
    ctrl->releasePath();
    return 0;
}

LUA_FUNCTION(DestroyPath)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int ref = (unsigned int)lua_torawvalue(L, 1);
    if (ref && !g_game->m_pathManager->deletePathByReference(ref))
        LuaCallSite(L);
    return 0;
}

LUA_FUNCTION(SplineRelative)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt)
        return 0;
    EEntity *origin = GetEntityArg(L, 2, true, 0);
    ESplineController *ctrl = theEnt->getSplineController();
    if (!ctrl)
        return 0;
    if (origin) {
        ctrl->setRelativeSplineFlag(true);
        ctrl->setSplineOrigin(origin->GetPosition());
    } else
        ctrl->setRelativeSplineFlag(false);
    return 0;
}

LUA_FUNCTION_TODO(SplineSetValues)

LUA_FUNCTION(SplineSetListener)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EScriptObject *listenerVal = GetScriptObjectArg(L, 2, true, false);
    ESplineController *ctrl = ent->getSplineController();
    ctrl->SetListener(listenerVal);
    return 0;
}

LUA_FUNCTION(SplineGetNearestPoint)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    unsigned int ref = (unsigned int)lua_torawvalue(L, 1);
    Vector3 myPoint(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    if (!ref)
        return 0;
    ESplinePath *path = g_game->m_pathManager->getPath(ref);
    if (!path)
        return 0;
    float fT = path->getNearestPoint(myPoint);
    lua_pushnumber(L, fT);
    return 1;
}

LUA_FUNCTION(SplineGetNumWaypoints)
{
    int nN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int ref = (unsigned int)lua_torawvalue(L, 1);
    if (!ref)
        return 0;
    ESplinePath *path = g_game->m_pathManager->getPath(ref);
    if (!path)
        return 0;
    int theCount = path->getLastWaypoint() + 1;
    lua_pushint(L, theCount);
    return 1;
}

LUA_FUNCTION(LocatorPath)
{
    return CreateLocatorPath_impl(L);
}

LUA_FUNCTION(SplinePath)
{
    return CreateAbsolutePath_impl(L);
}

LUA_FUNCTION(SplineRelativePath)
{
    return CreateRelativePath_impl(L);
}

LUA_FUNCTION(SplineEE)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ESplineController::setExplicitEvaluation(on);
    return 0;
}
