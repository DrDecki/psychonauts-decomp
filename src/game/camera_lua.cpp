// Script functions, file name made up.
// Default FOV body reconstructed; other bodies remain missing.

#include "engine.h"
#include "luafunc.h"

#include "classes.h"

extern bool g_cameraShakeEnabled;
float NormalizeAngle(float degrees);
void SetPVSCamera(ECamera *cam);
void ScriptError(const char *fmt, ...);

// position of the script object with this id
Vector3 GetScriptObjectPosition(lua_State *L, int id);

static const float kNoLimit = 1000000000.0f;

// chase camera defaults
static const float kDefaultFOV = 104.0f;
static const float kDefaultRadius = 735.0f;
static const float kDefaultAltitude = 14.0f;
static const float kDefaultTargetHeight = 150.0f;


LUA_FUNCTION(SetCamera)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    unsigned int nType = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    bool force = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!force && !g_game->CamerasEnabled())
        return 0;
    g_game->SetCameraType(g_game->GetCamera(), nType);
    return 0;
}

LUA_FUNCTION(GetCamera)
{
    int type = g_game->GetCamera()->GetType();
    lua_pushnumber(L, type);
    return 1;
}

LUA_FUNCTION(SetCurrentCamera)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    if (!g_game->CamerasEnabled())
        return 0;
    g_game->SetCurrentCamera(index, 0);
    return 0;
}

LUA_FUNCTION(GetCurrentCamera)
{
    unsigned int index = g_game->GetCamera() - g_game->GetCameraByIndex(0);
    lua_pushnumber(L, index);
    return 1;
}

LUA_FUNCTION(SetCameraPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 pos(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->SetPosition(Vector3(pos));
    cam->SetMoved();
    return 0;
}

LUA_FUNCTION(GetCameraPosition)
{
    Vector3 pos = g_game->GetCamera()->GetPosition();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION(SetCameraOrientation)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 euler(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    if (!g_game->CamerasEnabled())
        return 0;
    EQuat rot;
    rot.SetEuler(euler);
    g_game->GetCamera()->SetOrientation(rot);
    g_game->GetCamera()->SetMoved();
    return 0;
}

LUA_FUNCTION(GetCameraOrientation)
{
    EQuat rot = g_game->GetCamera()->GetOrientation();
    Vector3 euler = rot.ToEuler();
    lua_pushnumber(L, euler.x);
    lua_pushnumber(L, euler.y);
    lua_pushnumber(L, euler.z);
    return 3;
}

LUA_FUNCTION(SetCameraFieldOfView)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 4))
        return 0;
    float fov = GetNumberArg(L, 1, false);
    float secs = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    float myA = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    float b = n >= 4 ? GetNumberArg(L, 4, true) : 0.0f;
    if (!g_game->CamerasEnabled())
        return 0;
    g_game->GetCamera()->SetFieldOfView(fov, secs, myA, b);
    g_game->GetCamera()->SetMoved();
    return 0;
}

LUA_FUNCTION(GetCameraFieldOfView)
{
    float fov = g_game->GetCamera()->GetFieldOfView();
    lua_pushnumber(L, fov);
    return 1;
}

LUA_FUNCTION(GetCameraDefaultFOV)
{
    float fov = 104.0f;
    lua_pushnumber(L, fov);
    return 1;
}

LUA_FUNCTION(GetCameraViewVector)
{
    Vector3 view = g_game->GetCamera()->GetView();
    lua_pushnumber(L, view.x);
    lua_pushnumber(L, view.y);
    lua_pushnumber(L, view.z);
    return 3;
}

LUA_FUNCTION(GetCameraRightVector)
{
    Vector3 right = g_game->GetCamera()->GetRight();
    lua_pushnumber(L, right.x);
    lua_pushnumber(L, right.y);
    lua_pushnumber(L, right.z);
    return 3;
}

LUA_FUNCTION(SetCameraPrimaryTarget)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool snap = aN >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *theCam = g_game->GetCamera();
    if (lua_type(L, 1) == LUA_TNIL) {
        theCam->SetPrimaryTarget(0, snap);
        return 0;
    }
    EEntity *theEnt = GetEntityArg(L, 1, false, 0);
    if (!theEnt || !((EEntityVirtuals *)theEnt)->HasMesh()) {
        int id = (int)lua_topointer(L, 1);
        Vector3 pos = GetScriptObjectPosition(L, id);
        theCam->SetFixedTarget(pos, false);
        return 0;
    }
    EMeshEntity *mesh = (EMeshEntity *)theEnt;
    theCam->SetPrimaryTarget(mesh, snap);
    return 0;
}

LUA_FUNCTION(GetCameraPrimaryTarget)
{
    ECamera *cam = g_game->GetCamera();
    EEntity *target = cam->m_primaryTarget;
    if (target)
        lua_getref(L, target->GetScriptObject()->m_ref);
    else
        lua_pushnil(L);
    return 1;
}

LUA_FUNCTION(SetCameraSecondaryTarget)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 1, 3))
        return 0;
    unsigned int myMode = aN >= 2 ? (unsigned int)GetNumberArg(L, 2, true) : 1;
    bool snap = aN >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *theCam = g_game->GetCamera();
    if (lua_type(L, 1) == LUA_TNIL) {
        theCam->SetSecondaryTarget(0, myMode, snap);
        return 0;
    }
    EEntity *curEnt = GetEntityArg(L, 1, false, 0);
    if (!curEnt)
        return 0;
    if (!((EEntityVirtuals *)curEnt)->HasMesh())
        return 0;
    EMeshEntity *mesh = (EMeshEntity *)curEnt;
    theCam->SetSecondaryTarget(mesh, myMode, snap);
    return 0;
}

LUA_FUNCTION(GetCameraSecondaryTarget)
{
    ECamera *cam = g_game->GetCamera();
    EEntity *target = cam->m_secondaryTarget;
    if (target)
        lua_getref(L, target->GetScriptObject()->m_ref);
    else
        lua_pushnil(L);
    return 1;
}

LUA_FUNCTION(SetCameraTertiaryTarget)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *theCam = g_game->GetCamera();
    if (lua_type(L, 1) == LUA_TNIL) {
        theCam->SetTertiaryTarget(0);
        return 0;
    }
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    if (!((EEntityVirtuals *)ent)->HasMesh())
        return 0;
    EMeshEntity *mesh = (EMeshEntity *)ent;
    theCam->SetTertiaryTarget(mesh);
    return 0;
}

LUA_FUNCTION(GetCameraTertiaryTarget)
{
    ECamera *cam = g_game->GetCamera();
    EEntity *target = cam->m_tertiaryTarget;
    if (target)
        lua_getref(L, target->GetScriptObject()->m_ref);
    else
        lua_pushnil(L);
    return 1;
}

LUA_FUNCTION(SetCameraFixedTarget)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    float x = GetNumberArg(L, 1, false);
    float y = GetNumberArg(L, 2, false);
    float z = GetNumberArg(L, 3, false);
    bool snap = aN >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *theCamArg = g_game->GetCamera();
    theCamArg->SetFixedTarget(Vector3(x, y, z), snap);
    return 0;
}

LUA_FUNCTION(SetCameraUpwardTracking)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    bool only = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *curCamVal = g_game->GetCamera();
    curCamVal->m_upwardTracking = on;
    curCamVal->m_upwardOnly = only;
    return 0;
}

LUA_FUNCTION(SetCameraRotateToTarget)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool myOn = GetNumberArg(L, 1, false) != 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_rotateToTarget = myOn;
    return 0;
}

LUA_FUNCTION(SetCameraFloatingTarget)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float x = GetNumberArg(L, 1, false);
    float y = GetNumberArg(L, 2, false);
    float z = GetNumberArg(L, 3, false);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *pCamVal = g_game->GetCamera();
    pCamVal->SetFloatingTarget(Vector3(x, y, z));
    return 0;
}

LUA_FUNCTION(SetCameraTargetHeight)
{
    if (!g_game->CamerasEnabled())
        return 0;
    if (lua_gettop(L) != 1) {
        ScriptError("ScriptVM: Wrong number of args to SetCameraTargetHeight. Usage: SetCameraTargetHeight(target_height);");
        return 0;
    }
    ECamera *pCam = g_game->GetCamera();
    float height = lua_tonumber(L, 1);
    pCam->SetTargetHeight(height);
    return 0;
}

LUA_FUNCTION(GetCameraTargetHeight)
{
    float height = g_game->GetCamera()->GetTargetHeight();
    lua_pushnumber(L, height);
    return 1;
}

LUA_FUNCTION(MoveCameraToIdeal)
{
    ECamera *cam = g_game->GetCamera();
    cam->MoveToIdeal();
    cam->m_idealBlend = 0.0f;
    return 0;
}

LUA_FUNCTION(ResetCameraMode)
{
    ECamera *cam = g_game->GetCamera();
    cam->ResetMode();
    return 0;
}

LUA_FUNCTION(SetCameraShakeEnable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_cameraShakeEnabled = on;
    return 0;
}

LUA_FUNCTION(GetCameraShakeEnable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    bool enabled = g_cameraShakeEnabled;
    lua_pushnumber(L, enabled);
    return 1;
}

LUA_FUNCTION(CameraStartShakeRandom)
{
    int curNArg = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    float secs = lua_tonumber(L, 1);
    float period = lua_tonumber(L, 2);
    float myPhase = lua_tonumber(L, 3);
    float fXVal = lua_tonumber(L, 4);
    float y = lua_tonumber(L, 5);
    float fZVal = lua_tonumber(L, 6);
    g_game->GetCamera()->StartShake(Vector3(fXVal, y, fZVal), secs, period, myPhase);
    return 0;
}

LUA_FUNCTION(CameraStartShakePeriodic)
{
    int curNArg = lua_gettop(L);
    if (!CheckArgs(L, 7, 7))
        return 0;
    float secs = lua_tonumber(L, 1);
    float fA = lua_tonumber(L, 2);
    float b = lua_tonumber(L, 3);
    float newX = lua_tonumber(L, 4);
    float y = lua_tonumber(L, 5);
    float fZVal = lua_tonumber(L, 6);
    float myPeriodArg = lua_tonumber(L, 7);
    g_game->GetCamera()->StartShakePeriodic(Vector3(newX, y, fZVal), myPeriodArg, secs, fA, b);
    return 0;
}

LUA_FUNCTION(CameraStartShakeDriven)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float amount = lua_tonumber(L, 1);
    g_game->GetCamera()->StartShakeDriven(amount);
    return 0;
}

LUA_FUNCTION(CameraStopShake)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->GetCamera()->StopShake();
    return 0;
}

LUA_FUNCTION(SetCameraShakeRandom)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float amount = lua_tonumber(L, 1);
    if (amount != 0) {
        Vector3 shake(amount, amount, amount);
        g_game->GetCamera()->StartShake(shake, 100000.0f, 0, 0);
    } else
        g_game->GetCamera()->StopShake();
    return 0;
}

LUA_FUNCTION(SetCameraShakePeriodic)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float amount = lua_tonumber(L, 1);
    if (amount != 0) {
        Vector3 shake(amount, amount, amount);
        g_game->GetCamera()->StartShakePeriodic(shake, 3.0f, 100000.0f, 0, 0);
    } else
        g_game->GetCamera()->StopShake();
    return 0;
}

LUA_FUNCTION(AttachCameraToEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 8))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, true);
    Vector3 offset = n >= 2 ? Vector3(GetNumberArg(L, 2, true), GetNumberArg(L, 3, true), GetNumberArg(L, 4, true)) : g_zeroVector;
    Vector3 theEuler = n >= 5 ? Vector3(GetNumberArg(L, 5, true), GetNumberArg(L, 6, true), GetNumberArg(L, 7, true)) : g_zeroVector;
    bool snap = n >= 8 ? GetNumberArg(L, 8, true) != 0 : true;
    ECamera *camValue = g_game->GetCamera();
    if (lua_type(L, 1) == LUA_TNIL) {
        camValue->AttachTo(0, g_zeroVector, g_identityQuat, true);
        return 0;
    }
    camValue->AttachTo(ent, offset, EQuat(theEuler), snap);
    return 0;
}

LUA_FUNCTION(AssignCameraToChannel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int nIndex = (unsigned int)lua_tonumber(L, 1);
    unsigned int channel = (unsigned int)lua_tonumber(L, 2);
    g_game->SetCurrentCamera(nIndex, channel);
    return 0;
}

LUA_FUNCTION(SetCameraChannelFilter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int nFilterArg = (unsigned int)GetNumberArg(L, 2, false);
    ERenderChannel *channel = g_game->GetChannel(index);
    channel->m_filter = nFilterArg;
    return 0;
}

LUA_FUNCTION(SetCameraChannelFixedTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    void *texture = lua_torawvalue(L, 2);
    ERenderChannel *channel = g_game->GetChannel(index);
    channel->m_fixedTexture = texture;
    return 0;
}

LUA_FUNCTION(SetCameraChannelMaskTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    void *texture = lua_torawvalue(L, 2);
    ERenderChannel *channel = g_game->GetChannel(index);
    channel->m_maskTexture = texture;
    return 0;
}

LUA_FUNCTION(SetCameraChannelMovieStatic)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    float newStartEnt = GetNumberArg(L, 2, false);
    float fEndObj = GetNumberArg(L, 3, false);
    float amount = GetNumberArg(L, 4, false);
    newStartEnt = Clamp(newStartEnt, 0.0f, 1.0f);
    fEndObj = Clamp(fEndObj, newStartEnt, 1.0f);
    amount = Max(amount, 1.0f);
    ERenderChannel *channel = g_game->GetChannel(index);
    channel->m_staticStart = newStartEnt;
    channel->m_staticEnd = fEndObj;
    channel->m_staticAmount = amount;
    ELevel *curLevel = g_game->GetLevel();
    if (!curLevel->m_movieStaticTexture)
        curLevel->m_movieStaticTexture = g_game->m_resources->Load("WorkResource\\Textures\\LevelTextures\\BB_BasicBraining\\BB_movie_static.dds", 0x50000, 0);
    return 0;
}

LUA_FUNCTION(SetCameraChannelDeeLiteTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 4))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    bool bOn = GetNumberArg(L, 2, false) != 0;
    void *texture = n >= 3 ? lua_torawvalue(L, 3) : 0;
    ETriggerBase *ent = n >= 4 ? GetTriggerArg(L, 4, true) : 0;
    ERenderChannel *channel = g_game->GetChannel(index);
    if (!texture)
        texture = g_game->m_deeLiteTexture;
    if (bOn) {
        channel->m_flags |= 2;
        channel->m_deeLiteTexture = texture;
        channel->m_deeLiteEntity = ent;
    } else {
        channel->m_flags &= ~2;
        channel->m_deeLiteTexture = 0;
        channel->m_deeLiteEntity = 0;
    }
    return 0;
}

LUA_FUNCTION(SetMonitorChannel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    unsigned int channel = (unsigned int)GetNumberArg(L, 2, false);
    pMesh->SetMonitorChannel(channel ? channel : -1, 0);
    return 0;
}

LUA_FUNCTION(BeginVisibilityQuery)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    ETriggerBase *ent = GetTriggerArg(L, 2, false);
    if (!ent)
        return 0;
    ERenderChannel *channel = g_game->GetChannel(index);
    channel->m_flags |= 8;
    channel->m_queryEntity = ent;
    channel->m_visibility = -1.0f;
    return 0;
}

LUA_FUNCTION(GetVisibilityQueryResult)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 1, false);
    ERenderChannel *channel = g_game->GetChannel(index);
    lua_pushnumber(L, channel->m_visibility);
    return 1;
}

LUA_FUNCTION(WorldCoordsToScreen)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    Vector3 pPos(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    bool bClip = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    Vector3 screen = g_game->GetRenderCamera()->WorldToScreen(pPos, bClip);
    lua_pushnumber(L, screen.x);
    lua_pushnumber(L, screen.y);
    return 2;
}

LUA_FUNCTION(SetCameraIR)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_ir = on;
    return 0;
}

LUA_FUNCTION_TODO(PointVisibleFromCamera)

LUA_FUNCTION_TODO(EntityVisibleFromCamera)

LUA_FUNCTION(TestSphereInScene)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    float x = GetNumberArg(L, 1, false);
    float y = GetNumberArg(L, 2, false);
    float z = GetNumberArg(L, 3, false);
    float fRadiusObj = aN >= 4 ? GetNumberArg(L, 4, true) : 0.0f;
    ECamera *pCamVal = g_game->GetCamera();
    unsigned int visible = pCamVal->TestSphere(Vector3(x, y, z), fRadiusObj);
    lua_pushuint(L, visible);
    return 1;
}

LUA_FUNCTION(CreateNewCamera)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool curAssign = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    ECamera *myCamVal = new ECamera;
    int channel = 12;
    if (curAssign) {
        for (channel = 8; channel < 12; channel++) {
            ERenderChannel *ch = g_game->GetChannel(channel);
            if (!ch->m_camera) {
                g_game->SetChannelCamera(myCamVal, channel);
                ch->m_flags |= 4;
                break;
            }
        }
    }
    lua_pushrawnumber(L, (int)myCamVal);
    if (channel < 12) {
        lua_pushint(L, channel);
        return 2;
    } else
        return 1;
}

LUA_FUNCTION(FreezePVS)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool on = n >= 1 ? GetNumberArg(L, 1, true) != 0 : true;
    if (on) {
        ECamera *cam = g_game->GetCamera();
        ECamera *pvs = g_game->GetCameraByIndex(0);
        g_game->SetCameraType(pvs, 0);
        pvs->SetPosition(cam->GetPosition());
        pvs->SetOrientation(cam->GetOrientation());
        ECamera *view = g_game->GetCameraByIndex(1);
        g_game->SetCameraType(view, 0);
        view->SetPosition(cam->GetPosition());
        view->SetOrientation(cam->GetOrientation());
        SetPVSCamera(pvs);
        g_game->SetChannelCamera(view, 0);
    } else {
        SetPVSCamera(0);
        g_game->SetCurrentCamera(0, 0);
    }
    return 0;
}

LUA_FUNCTION(DebugCurrentCamera)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool on = n >= 1 ? GetNumberArg(L, 1, true) != 0 : true;
    ECamera *pCam = g_game->GetCameraByIndex(0);
    ECamera *pDebug = g_game->GetCameraByIndex(1);
    if (on) {
        g_game->SetCameraType(pDebug, 0);
        Vector3 view = pCam->GetView();
        Vector3 thePos = pCam->GetPosition();
        pDebug->SetPosition(thePos - view * 500.0f);
        pDebug->SetOrientation(pCam->GetOrientation());
        g_game->SetChannelCamera(pDebug, 0);
    } else
        g_game->SetChannelCamera(pCam, 0);
    return 0;
}

LUA_FUNCTION(DebugBossCamera)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    if (!g_game->CamerasEnabled())
        return 0;
    if (g_game->GetCamera()->GetType() != 2)
        return 0;
    if (on) {
        EEntity *target = g_game->GetPlayerControl()->GetLockTarget();
        if (!target || !((EEntityVirtuals *)target)->HasMesh())
            return 0;
        g_game->GetCamera()->SetSecondaryTarget(target, 1, true);
    } else
        g_game->GetCamera()->SetSecondaryTarget(0, 1, true);
    return 0;
}

LUA_FUNCTION(SetChaseCameraFixedPosition)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 3, 5))
        return 0;
    float x = GetNumberArg(L, 1, false);
    float y = GetNumberArg(L, 2, false);
    float z = GetNumberArg(L, 3, false);
    bool snap = aN >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    bool bKeepTarget = aN >= 5 ? GetNumberArg(L, 5, true) != 0 : true;
    ECamera *theCamArg = g_game->GetCamera();
    if (theCamArg->GetType() != 2)
        return 0;
    theCamArg->SetFixedPosition(Vector3(x, y, z), snap, bKeepTarget, 0);
    return 0;
}

LUA_FUNCTION(SetChaseCameraFreePosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool myOn = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    ECamera *cam = g_game->GetCamera();
    cam->SetFreePosition(myOn);
    return 0;
}

LUA_FUNCTION(SetChaseCameraPaused)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool paused = n >= 1 ? GetNumberArg(L, 1, true) != 0 : true;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *myCam = g_game->GetCamera();
    if (paused && myCam->GetType() != 1) {
        ECamera::Pause(myCam->GetType());
        return 0;
    }
    if (paused) {
        Vector3 pos = myCam->GetPosition();
        Vector3 target = myCam->GetTargetPosition();
        g_game->SetCameraType(myCam, 2);
        myCam->SetFixedPosition(pos, false, true, 1);
        myCam->SetFixedTarget(target, false);
        myCam->MoveToIdeal();
    } else if (myCam->m_free) {
        myCam->SetPrimaryTarget(g_game->GetPlayerControl(), true);
        myCam->SetFreePosition(true);
    }
    return 0;
}

LUA_FUNCTION(SetChaseCameraStatic)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 6, 7))
        return 0;
    float x = GetNumberArg(L, 1, false);
    float y = GetNumberArg(L, 2, false);
    float theZ = GetNumberArg(L, 3, false);
    float targetX = GetNumberArg(L, 4, false);
    float newTargetYVal = GetNumberArg(L, 5, false);
    float targetZ = GetNumberArg(L, 6, false);
    bool snap = aN >= 7 ? GetNumberArg(L, 7, true) != 0 : false;
    ECamera *theCamArg = g_game->GetCamera();
    if (theCamArg->GetType() != 2)
        return 0;
    theCamArg->SetFixedTarget(Vector3(targetX, newTargetYVal, targetZ), true);
    theCamArg->SetFixedPosition(Vector3(x, y, theZ), snap, false, 0);
    return 0;
}

LUA_FUNCTION(SetChaseCameraFixedSpherical)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    float radius = GetNumberArg(L, 1, false);
    float theAzimuth = GetNumberArg(L, 2, false);
    float myAltitude = GetNumberArg(L, 3, false);
    bool bSnapVal = n >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    ECamera *cam = g_game->GetCamera();
    if (cam->GetType() != 2)
        return 0;
    cam->SetFixedSpherical(Vector3(radius, theAzimuth, myAltitude), bSnapVal);
    return 0;
}

LUA_FUNCTION(SetChaseCameraFree)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool myOn = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    ECamera *cam = g_game->GetCamera();
    cam->SetPrimaryTarget(g_game->GetPlayerControl(), myOn);
    cam->SetFreePosition(myOn);
    cam->SetFreeOrientation(myOn);
    return 0;
}

LUA_FUNCTION(SetChaseCameraRadius)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    float fRadius = lua_tonumber(L, 1);
    cam->SetRadius(fRadius);
    return 0;
}

LUA_FUNCTION(GetChaseCameraRadius)
{
    ECamera *cam = g_game->GetCamera();
    float radius = cam->m_radius;
    lua_pushnumber(L, radius);
    return 1;
}

LUA_FUNCTION_TODO(SetChaseCameraAzimuth)

LUA_FUNCTION_TODO(GetChaseCameraAzimuth)

LUA_FUNCTION(SetChaseCameraAltitude)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    float myAltitude = lua_tonumber(L, 1);
    cam->SetAltitude(myAltitude);
    return 0;
}

LUA_FUNCTION(GetChaseCameraAltitude)
{
    ECamera *cam = g_game->GetCamera();
    float altitude = cam->m_altitude;
    lua_pushnumber(L, altitude);
    return 1;
}

LUA_FUNCTION(GetChaseCameraIdealPosition)
{
    Vector3 pos = g_game->GetCamera()->GetIdealPosition();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION(GetChaseCameraIdealOrientation)
{
    EQuat rot = g_game->GetCamera()->GetIdealOrientation();
    Vector3 myEuler = rot.ToEuler();
    lua_pushnumber(L, myEuler.x);
    lua_pushnumber(L, myEuler.y);
    lua_pushnumber(L, myEuler.z);
    return 3;
}

LUA_FUNCTION(SetChaseCameraPhysicsRadius)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    unsigned int nIndex = (unsigned int)lua_tonumber(L, 1);
    float fRadius = lua_tonumber(L, 2);
    ECamera::SetPhysicsRadius(nIndex, fRadius);
    return 0;
}

LUA_FUNCTION(SetChaseCameraPhysicsAltitude)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ECamera *cam = g_game->GetCamera();
    unsigned int nIndex = (unsigned int)lua_tonumber(L, 1);
    float myAltitude = lua_tonumber(L, 2);
    ECamera::SetPhysicsAltitude(nIndex, myAltitude);
    return 0;
}

LUA_FUNCTION(SetChaseCameraHintAzimuth)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float myAzimuth = GetNumberArg(L, 1, false);
    if (!g_game->CamerasEnabled() || !g_game->m_cameraHints)
        return 0;
    ECamera *cam = g_game->GetCamera();
    if (cam->GetType() != 2)
        return 0;
    cam->SetHintAzimuth(NormalizeAngle(myAzimuth));
    return 0;
}

LUA_FUNCTION(SetChaseCameraLadderAzimuth)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float myAzimuth = GetNumberArg(L, 1, false);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->SetLadderAzimuth(NormalizeAngle(myAzimuth));
    return 0;
}

LUA_FUNCTION(SetChaseCameraAutoRotateRange)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    float lo = GetNumberArg(L, 1, false);
    float fHi = GetNumberArg(L, 2, false);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_autoRotateRange = Vector2(lo, fHi);
    return 0;
}

LUA_FUNCTION(SetChaseCameraDistanceAdjustRange)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fRange = GetNumberArg(L, 1, false);
    fRange = Max(fRange, 0.0f);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_distanceAdjustRange = fRange;
    return 0;
}

LUA_FUNCTION(SetChaseCameraDistanceAdjustParam)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float param = GetNumberArg(L, 1, false);
    param = Clamp(param, 0.0f, 1.0f);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_distanceAdjustParam = param;
    return 0;
}

LUA_FUNCTION(SetChaseCameraFallingAltitude)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float myAltitude = GetNumberArg(L, 1, false);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_fallingAltitude = myAltitude;
    return 0;
}

LUA_FUNCTION(SetChaseCameraZLock)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool myOn = GetNumberArg(L, 1, false) != 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->m_zLock = myOn;
    return 0;
}

LUA_FUNCTION(AlternateChaseCameraSettings)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->SetRadius(620.0f);
    cam->SetAltitude(14.0f);
    cam->SetTargetHeight(180.0f);
    cam->SetFieldOfView(110.0f, 0, 0, 0);
    cam->MoveToIdeal();
    return 0;
}

LUA_FUNCTION(RestoreChaseCameraSettings)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->SetRadius(kDefaultRadius);
    cam->SetAltitude(kDefaultAltitude);
    cam->SetTargetHeight(kDefaultTargetHeight);
    cam->SetFieldOfView(kDefaultFOV, 0, 0, 0);
    cam->MoveToIdeal();
    return 0;
}

LUA_FUNCTION_TODO(CreateCameraPath)

LUA_FUNCTION_TODO(LoadCameraPath)

LUA_FUNCTION(DeleteCameraPath)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (lua_type(L, 1) == LUA_TNIL)
        return 0;
    unsigned int ref = (unsigned int)lua_torawvalue(L, 1);
    if (!ref)
        return 0;
    void *thePath = g_game->m_cameraPaths->Find(ref);
    if (thePath)
        g_game->m_cameraPaths->Delete(thePath);
    return 0;
}

LUA_FUNCTION_NOSTACK_TODO(RunCameraPath)

LUA_FUNCTION(ClearCameraPath)
{
    if (g_game->GetCamera()->GetType() != 1)
        return 0;
    ECamera *cam = g_game->GetCamera();
    cam->ClearPath();
    return 0;
}

LUA_FUNCTION(IsCameraPathRunning)
{
    if (g_game->GetCamera()->GetType() != 1)
        return 0;
    ECamera *cam = g_game->GetCamera();
    if (cam->IsPathRunning())
        lua_pushnumber(L, 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION(SetRailCameraPaths)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    if (!g_game->CamerasEnabled())
        return 0;
    void *pRadiusPath = lua_torawvalue(L, 1);
    void *theAltitudePathArg = lua_torawvalue(L, 2);
    ECamera *cam = g_game->GetCamera();
    cam->SetRailPaths(pRadiusPath, theAltitudePathArg);
    return 0;
}

LUA_FUNCTION(SetRailCameraRadius)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float value = GetNumberArg(L, 1, false);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    if (lua_type(L, 1) == LUA_TNIL)
        cam->SetRailRadius(kNoLimit);
    else
        cam->SetRailRadius(value);
    return 0;
}

LUA_FUNCTION(SetRailCameraAltitude)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float theValue = GetNumberArg(L, 1, false);
    if (!g_game->CamerasEnabled())
        return 0;
    ECamera *cam = g_game->GetCamera();
    if (lua_type(L, 1) == LUA_TNIL)
        cam->SetRailAltitude(kNoLimit);
    else
        cam->SetRailAltitude(theValue);
    return 0;
}

LUA_FUNCTION(SetFlyCamPos)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 pos(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    if (g_game->GetCamera()->GetType() != 0)
        return 0;
    g_game->GetCamera()->SetPosition(Vector3(pos));
    g_game->GetCamera()->SetMoved();
    return 0;
}

LUA_FUNCTION(SetFlyCamOri)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 euler(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    if (g_game->GetCamera()->GetType() != 0)
        return 0;
    EQuat rot;
    rot.SetEuler(euler);
    g_game->GetCamera()->SetOrientation(rot);
    g_game->GetCamera()->SetMoved();
    return 0;
}
