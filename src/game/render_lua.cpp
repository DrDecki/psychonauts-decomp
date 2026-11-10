// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

#include "classes.h"

static const float kMs = 1000.0f;
static const float kFar = 1000000000.0f;
static const float kMaxDistortion = 1000.0f;
static const float kMaxBlurScale = 1000.0f;
static const float kMinBlurScale = 0.001f;
static const float kMaxGlow = 2.0f;


LUA_FUNCTION(SetViewportNearZ)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float nearZ = lua_tonumber(L, 1);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_nearZ = Clamp(nearZ, 0.0f, pLevel->m_farZ - 0.1);
    return 0;
}

LUA_FUNCTION(SetViewportFarZ)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float farZ = lua_tonumber(L, 1);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_farZ = Clamp(farZ, pLevel->m_nearZ + 0.1, kFar);
    return 0;
}

LUA_FUNCTION(SetDistanceFogEnable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    if (g_game->m_renderer->m_fogSupported) {
        ELevel *level = g_game->GetLevel();
        level->m_distanceFog = on;
    }
    return 0;
}

LUA_FUNCTION(SetDistanceFogColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    if (g_game->m_renderer->m_fogSupported) {
        float r = Clamp(lua_tonumber(L, 1), 0.0f, 1.0f);
        float g = Clamp(lua_tonumber(L, 2), 0.0f, 1.0f);
        float b = Clamp(lua_tonumber(L, 3), 0.0f, 1.0f);
        ELevel *pLevel = g_game->GetLevel();
        pLevel->m_fogColor.Set(r, g, b);
    }
    return 0;
}

LUA_FUNCTION(SetDistanceFogRange)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    if (g_game->m_renderer->m_fogSupported) {
        float start = Clamp(lua_tonumber(L, 1), 0.0f, kFar);
        float end = Clamp(lua_tonumber(L, 2), start, kFar);
        ELevel *pLevel = g_game->GetLevel();
        pLevel->m_fogStart = start;
        pLevel->m_fogEnd = end;
    }
    return 0;
}

LUA_FUNCTION(SetHeightFogEnable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    if (g_game->m_renderer->m_fogSupported) {
        ELevel *level = g_game->GetLevel();
        level->m_heightFog = on;
    }
    return 0;
}

LUA_FUNCTION(SetHeightFogColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    if (g_game->m_renderer->m_fogSupported) {
        float r = Clamp(lua_tonumber(L, 1), 0.0f, 1.0f);
        float g = Clamp(lua_tonumber(L, 2), 0.0f, 1.0f);
        float b = Clamp(lua_tonumber(L, 3), 0.0f, 1.0f);
        ELevel *pLevel = g_game->GetLevel();
        pLevel->m_fogColor.Set(r, g, b);
    }
    return 0;
}

LUA_FUNCTION(SetHeightFogRange)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    if (g_game->m_renderer->m_fogSupported) {
        float start = lua_tonumber(L, 1);
        float end = lua_tonumber(L, 2);
        ELevel *pLevel = g_game->GetLevel();
        pLevel->m_heightFogRange.x = start;
        pLevel->m_heightFogRange.y = end;
    }
    return 0;
}

LUA_FUNCTION_TODO(SetVolumeFogColor)

LUA_FUNCTION(SetDetailCullingRange)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    float fStart = GetNumberArg(L, 1, false);
    float end = GetNumberArg(L, 2, false);
    float theMinRange = 0.01f;
    fStart = Clamp(fStart, theMinRange, 1.0f);
    end = Clamp(end, 0.0f, fStart - theMinRange);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_detailCullStart = fStart;
    pLevel->m_detailCullEnd = end;
    return 0;
}

LUA_FUNCTION(SetDistortionScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = Clamp(lua_tonumber(L, 1), 0.0f, 1000.0f);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_distortionScale = fScale;
    return 0;
}

LUA_FUNCTION(SetDistortionColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float r = Clamp(lua_tonumber(L, 1), 0.0f, 1.0f);
    float g = Clamp(lua_tonumber(L, 2), 0.0f, 1.0f);
    float b = Clamp(lua_tonumber(L, 3), 0.0f, 1.0f);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_distortionColor.Set(r, g, b);
    return 0;
}

LUA_FUNCTION(SetDistortionFilter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int iFilter = (unsigned int)GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_distortionFilter = iFilter;
    return 0;
}

LUA_FUNCTION(SetDistortionTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *texture = lua_torawvalue(L, 1);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_distortionTexture = texture;
    return 0;
}

LUA_FUNCTION(SetDistortionFullScreen)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = lua_tonumber(L, 1) != 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_distortionFullScreen = on;
    return 0;
}

LUA_FUNCTION_TODO(SetShadowColor)

LUA_FUNCTION(SetShadowBlendMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int iRaw = (int)lua_tonumber(L, 1);
    int nMode = iRaw > 1 ? iRaw : 1;
    ELevel *myLevel = g_game->GetLevel();
    myLevel->m_shadowBlendMode = nMode;
    return 0;
}

LUA_FUNCTION(SetShadowFixedDirection)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_shadowFixedDirection = on;
    return 0;
}

LUA_FUNCTION(SetShadowFixedLightSource)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *theTexture = lua_torawvalue(L, 1);
    ELevel *levelPtr = g_game->GetLevel();
    levelPtr->m_shadowLightSource = theTexture;
    return 0;
}

LUA_FUNCTION(SetShadowCullingBias)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float theValue = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_shadowCullingBias = theValue;
    return 0;
}

LUA_FUNCTION(SetStaticLightIntensity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fValueVal = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_staticLightIntensity = fValueVal;
    g_game->UpdateDomainSleep();
    return 0;
}

LUA_FUNCTION(SetDynamicLightIntensity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fValueVal = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_dynamicLightIntensity = fValueVal;
    g_game->UpdateDomainSleep();
    return 0;
}

LUA_FUNCTION(SetLightRigIntensity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fValueVal = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_lightRigIntensity = fValueVal;
    g_game->UpdateDomainSleep();
    return 0;
}

LUA_FUNCTION_TODO(SetLightRigBoost)

LUA_FUNCTION(SetPsiLightColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float r = GetNumberArg(L, 1, false);
    float g = GetNumberArg(L, 2, false);
    float b = GetNumberArg(L, 3, false);
    g_game->m_lights->SetPsiLightColor(Vector3(Clamp(r, 0.0f, 1.0f), Clamp(g, 0.0f, 1.0f), Clamp(b, 0.0f, 1.0f)));
    return 0;
}

LUA_FUNCTION(SetNumWaterBlurs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int count = (unsigned int)GetNumberArg(L, 1, false);
    if (count < 0)
        return 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_numWaterBlurs = count;
    return 0;
}

LUA_FUNCTION(SetNumGlowBlurs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int count = (unsigned int)GetNumberArg(L, 1, false);
    if (count < 1)
        return 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_numGlowBlurs = count;
    return 0;
}

LUA_FUNCTION_TODO(SetGlareColor)

LUA_FUNCTION(SetMotionBlurEnable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    float mySecs = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    mySecs = Max(mySecs, 0.0f);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_motionBlur = on;
    pLevel->m_motionBlurTime = mySecs * kMs;
    return 0;
}

LUA_FUNCTION_TODO(SetMotionBlurInput)

LUA_FUNCTION(SetMotionBlurOutput)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 4))
        return 0;
    float output = GetNumberArg(L, 1, false);
    float secs = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    float output2 = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    float fSecs = n >= 4 ? GetNumberArg(L, 4, true) : 0.0f;
    output = Clamp(output, 0.0f, 1.0f);
    secs = Max(secs, 0.0f);
    ELevel *pLevelVal = g_game->GetLevel();
    pLevelVal->m_motionBlurOutput = output;
    pLevelVal->m_motionBlurTime = secs * kMs;
    pLevelVal->m_motionBlurOutput2 = output2;
    pLevelVal->m_motionBlurOutputTime2 = fSecs * kMs;
    return 0;
}

LUA_FUNCTION(SetMotionBlurScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = GetNumberArg(L, 1, false);
    fScale = Clamp(fScale, kMinBlurScale, kMaxBlurScale);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_motionBlurScale = fScale;
    return 0;
}

LUA_FUNCTION(SetMotionBlurFilter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int iFilter = (unsigned int)GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_motionBlurFilter = iFilter;
    return 0;
}

LUA_FUNCTION(SetMotionBlurMaskTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *texture = lua_torawvalue(L, 1);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_motionBlurMask = texture;
    return 0;
}

LUA_FUNCTION(SetMotionBlurState)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    unsigned int myState = (unsigned int)GetNumberArg(L, 1, false);
    float secs = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    float myMs = Max(secs * kMs, 0.0f);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->SetMotionBlurState(myState, myMs);
    return 0;
}

LUA_FUNCTION(SetRenderSplineTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *texture = lua_torawvalue(L, 1);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_splineTexture = texture;
    return 0;
}

LUA_FUNCTION(SetRenderSplineThickness)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float value = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_splineThickness = value;
    return 0;
}

LUA_FUNCTION(SetRenderSplineDensity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fValueVal = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_splineDensity = fValueVal;
    return 0;
}

LUA_FUNCTION_TODO(RenderSpline)

LUA_FUNCTION_TODO(RenderLightTrail)

LUA_FUNCTION(FreezeCoyoteProjections)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_freezeCoyoteProjections = on;
    return 0;
}

LUA_FUNCTION(SetPsychedelicAlpha)
{
    int curNArg = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    float alpha = GetNumberArg(L, 1, false);
    float secs = curNArg >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    alpha = Clamp(alpha, 0.0f, 1.0f);
    float ms = Max(secs * kMs, 0.0f);
    ELevel *pLevel = g_game->GetLevel();
    if (ms != 0) {
        pLevel->m_psychedelicAlpha = alpha;
        pLevel->m_psychedelicTime = ms;
    } else {
        pLevel->m_psychedelicTarget = alpha;
        pLevel->m_psychedelicTime = 0.0f;
    }
    return 0;
}

LUA_FUNCTION(SetShaderLOD)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_shaderLOD = on;
    return 0;
}

LUA_FUNCTION(SetMirrorScene)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_mirrorScene = on;
    return 0;
}

LUA_FUNCTION(SetBinaryAlphaThreshhold)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int nThreshold = (int)GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_binaryAlphaThreshold = Clamp(nThreshold, 0, 0xff);
    return 0;
}

LUA_FUNCTION_TODO(PlayVideo)

LUA_FUNCTION(SetVideoVolume)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int volume = (int)GetNumberArg(L, 1, false);
    if (volume < 0 || volume > 0xffff)
        return 0;
    g_game->SetVideoVolume(volume);
    return 0;
}

LUA_FUNCTION(CaptureScreenShot)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    bool bA = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    bool b = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    g_game->m_screenshot = true;
    g_game->m_screenshotA = bA;
    g_game->m_screenshotB = b;
    return 0;
}

LUA_FUNCTION(CaptureEnvironmentMap)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    unsigned int size = n >= 1 ? (unsigned int)GetNumberArg(L, 1, true) : 0x80;
    float time = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    g_game->m_captureEnvironmentMap = true;
    g_game->m_environmentMapSize = size;
    g_game->m_environmentMapTime = time;
    return 0;
}

LUA_FUNCTION(MovieCaptureMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    bool theFlag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    g_game->m_movieCapture = on;
    bool capture = on ? theFlag : true;
    g_game->m_movieCaptureFlag = capture;
    g_game->m_notMovieCaptureFlag = !capture;
    return 0;
}

LUA_FUNCTION(RefreshVertexShaders)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_refreshVertexShaders = true;
    return 0;
}

LUA_FUNCTION(RefreshPixelShaders)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_refreshPixelShaders = true;
    return 0;
}

LUA_FUNCTION(PalettizeLightmaps)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_lightmaps->Palettize();
    return 0;
}

LUA_FUNCTION(SetEntityVisible)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    int nVisible = (int)GetNumberArg(L, 2, false);
    ((EScriptEntity *)ent)->SetFlag(1, nVisible != 0);
    return 0;
}

LUA_FUNCTION_TODO(SetEntityColor)

LUA_FUNCTION(SetEntityAlpha)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float aAlphaVal = GetNumberArg(L, 2, false);
    float secs = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    aAlphaVal = Clamp(aAlphaVal, 0.0f, 1.0f);
    float myMs = Max(secs * kMs, 0.0f);
    ent->SetAlpha(aAlphaVal, myMs);
    return 0;
}

LUA_FUNCTION(GetEntityAlpha)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float theAlpha = ent->GetAlpha();
    lua_pushnumber(L, theAlpha);
    return 1;
}

LUA_FUNCTION(SetEntityAmbientLight)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float r = GetNumberArg(L, 2, false);
    float g = GetNumberArg(L, 3, false);
    float b = GetNumberArg(L, 4, false);
    Vector3 pColorArg(Clamp(r, 0.0f, 1.0f), Clamp(g, 0.0f, 1.0f), Clamp(b, 0.0f, 1.0f));
    ent->SetAmbientLight(pColorArg);
    return 0;
}

LUA_FUNCTION(SetEntityGlow)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float theScale = Clamp(lua_tonumber(L, 2), 0.0f, 2.0f);
    float fMs = Max(lua_tonumber(L, 3) * kMs, 0.0f);
    if (!g_game->m_noPlayerGlow && (ETriggerBase *)g_game->GetPlayerControl() == ent) {
        Color color(1.0f, 0.8f, 0.0f, 1.0f);
        ent->SetSolidColor(color);
        ent->SetSolidColorIntensity(theScale * 0.25, fMs);
    } else
        ent->SetGlow(theScale, fMs);
    return 0;
}

LUA_FUNCTION_TODO(SetEntityAura)

LUA_FUNCTION(SetEntityAuraColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Color theColor = GetColorArg(L, 2, false);
    ent->SetAuraColor(theColor);
    return 0;
}

LUA_FUNCTION(EnableAuras)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ELevel *pLevel = g_game->GetLevel();
    pLevel->m_auras = on;
    return 0;
}

LUA_FUNCTION_TODO(SetEntitySolidColor)

LUA_FUNCTION(SetEntitySolidColorIntensity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float newIntensity = GetNumberArg(L, 2, false);
    float secs = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    secs = Max(secs, 0.0f);
    ent->SetSolidColorIntensity(newIntensity, secs);
    return 0;
}

LUA_FUNCTION(SetEntityTexCoordVel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float fU = GetNumberArg(L, 2, false);
    float v = GetNumberArg(L, 3, false);
    ent->SetTexCoordVel(Vector2(fU, v));
    return 0;
}

LUA_FUNCTION(GetEntityTexCoordVel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector2 vel = ent->GetTexCoordVel();
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    return 2;
}

LUA_FUNCTION_TODO(SetEntitySortingSphere)

LUA_FUNCTION(SetEntityCameraAlphaRadius)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float fRadius = GetNumberArg(L, 2, false);
    ent->SetCameraAlphaRadius(fRadius);
    return 0;
}

LUA_FUNCTION(SetEntityDistortionScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float fScale = GetNumberArg(L, 2, false);
    ent->SetDistortionScale(fScale);
    return 0;
}

LUA_FUNCTION(SetEntityDistortionColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float theX = GetNumberArg(L, 2, false);
    float fY = GetNumberArg(L, 3, false);
    float z = GetNumberArg(L, 4, false);
    ent->SetDistortionColor(Vector3(theX, fY, z));
    return 0;
}

LUA_FUNCTION(SetEntityLightRigScale)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float x = GetNumberArg(L, 2, false);
    float y = GetNumberArg(L, 3, false);
    float z = GetNumberArg(L, 4, false);
    ent->SetLightRigScale(Vector3(x, y, z));
    return 0;
}

LUA_FUNCTION(FindLight)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    void *pMesh = GetTriggerA8Arg(L, 1, true);
    const char *name = GetStringArg(L, 2, false);
    unsigned int theLight = g_game->m_lights->FindLight(name, pMesh);
    lua_pushrawnumber(L, theLight);
    return 1;
}

LUA_FUNCTION(SetLightPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    unsigned int handle = (unsigned int)lua_torawvalue(L, 1);
    Vector3 pos(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ELight *light = g_game->m_lights->Get(handle);
    if (!light)
        return 0;
    light->m_position = pos;
    light->Update();
    if (!(light->m_flags & 1))
        g_game->UpdateDomainSleep();
    return 0;
}

LUA_FUNCTION(GetLightPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int curHandle = (unsigned int)lua_torawvalue(L, 1);
    ELight *light = g_game->m_lights->Get(curHandle);
    if (!light)
        return 0;
    Vector3 pos = light->GetPosition(0, 0);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION_TODO(SetLightColor)

LUA_FUNCTION(GetLightColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (lua_type(L, 1) == LUA_TNIL) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        return 3;
    }
    unsigned int handle = (unsigned int)lua_torawvalue(L, 1);
    ELight *light = g_game->m_lights->Get(handle);
    if (!light) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        return 3;
    }
    lua_pushnumber(L, light->m_color.x);
    lua_pushnumber(L, light->m_color.y);
    lua_pushnumber(L, light->m_color.z);
    return 3;
}

LUA_FUNCTION(SetLightBrightness)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    if (lua_type(L, 1) == LUA_TNIL)
        return 0;
    unsigned int nHandle = (unsigned int)lua_torawvalue(L, 1);
    float fBrightness = GetNumberArg(L, 2, false);
    float secs = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    fBrightness = Clamp(fBrightness, 0.0f, 1.0f);
    secs = Max(secs, 0.0f);
    ELight *theLight = g_game->m_lights->Get(nHandle);
    if (!theLight)
        return 0;
    if (secs == 0) {
        theLight->m_brightness = fBrightness;
        theLight->m_fadeTime = 0.0f;
    } else {
        theLight->m_targetBrightness = fBrightness;
        theLight->m_fadeTime = secs;
    }
    if (!(theLight->m_flags & 1))
        g_game->UpdateDomainSleep();
    return 0;
}

LUA_FUNCTION(GetLightBrightness)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (lua_type(L, 1) == LUA_TNIL) {
        lua_pushnumber(L, 0);
        return 1;
    }
    unsigned int handle = (unsigned int)lua_torawvalue(L, 1);
    ELight *light = g_game->m_lights->Get(handle);
    if (!light) {
        lua_pushnumber(L, 0);
        return 1;
    }
    lua_pushnumber(L, light->m_brightness);
    return 1;
}

LUA_FUNCTION(SetLightRadius)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int handle = (unsigned int)lua_torawvalue(L, 1);
    float fRadius = GetNumberArg(L, 2, false);
    ELight *light = g_game->m_lights->Get(handle);
    if (!light)
        return 0;
    if (light->m_type != 2 || !(light->m_flags & 1))
        return 0;
    light->m_radius = fRadius;
    return 0;
}

LUA_FUNCTION(SetLightAttenRange)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    unsigned int curHandle = (unsigned int)lua_torawvalue(L, 1);
    float startVal = GetNumberArg(L, 2, false);
    float end = GetNumberArg(L, 3, false);
    ELight *light = g_game->m_lights->Get(curHandle);
    if (!light)
        return 0;
    if (light->m_type != 5)
        return 0;
    light->m_shadow->m_attenStart = startVal;
    light->m_shadow->m_attenEnd = end;
    return 0;
}

LUA_FUNCTION(SetLightShadowEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int handle = (unsigned int)lua_torawvalue(L, 1);
    ETriggerBase *ent = GetTriggerArg(L, 2, true);
    ELight *light = g_game->m_lights->Get(handle);
    if (!light)
        return 0;
    if (light->m_type != 5)
        return 0;
    light->m_shadow->m_entity = ent;
    return 0;
}

LUA_FUNCTION(LoadTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool theFlag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    void *texture = g_game->m_resources->Load(name, 0x50000 + (theFlag != 0), 0);
    lua_pushrawnumber(L, (int)texture);
    return 1;
}

LUA_FUNCTION(GetTextureDimensions)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *theHandle = lua_torawvalue(L, 1);
    void *theTexture = g_game->m_resources->GetTexture(theHandle, 0);
    if (!theTexture)
        return 0;
    int width, height;
    g_game->m_renderer->GetTextureSize(theTexture, &width, &height);
    lua_pushint(L, width);
    lua_pushint(L, height);
    return 2;
}

LUA_FUNCTION(SetEnvironmentMap)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *theHandle = lua_torawvalue(L, 1);
    if (theHandle) {
        ELevel *level = g_game->GetLevel();
        void *texture = g_game->m_resources->GetTexture(theHandle, 0);
        level->m_environmentMap = texture;
    }
    return 0;
}

LUA_FUNCTION_TODO(SetBaseTexture)

LUA_FUNCTION_TODO(RestoreBaseTexture)

LUA_FUNCTION(SetTrapTextureString)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *str = GetStringArg(L, 1, false);
    if (str)
        g_game->m_resources->SetTrapTextureString(str);
    return 0;
}

LUA_FUNCTION_TODO(GetAnimatedTextureFrame)

LUA_FUNCTION_TODO(SetAnimatedTextureFrame)

LUA_FUNCTION_TODO(GetAnimatedTextureFramerate)

LUA_FUNCTION_TODO(SetAnimatedTextureFramerate)

LUA_FUNCTION_TODO(GetAnimatedTexturePlayMode)

LUA_FUNCTION_TODO(SetAnimatedTexturePlayMode)

LUA_FUNCTION(GetViewPortDimensions)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    unsigned int width, height;
    g_game->m_renderer->GetViewport(&width, &height);
    lua_pushnumber(L, width);
    lua_pushnumber(L, height);
    return 2;
}
