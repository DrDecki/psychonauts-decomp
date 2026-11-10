// Script functions, file name made up.

#include <stdio.h>

#include "engine.h"
#include "luafunc.h"

#include "classes.h"
#include "diagnostic.h"

// per-file copies of the angle conversion factors, set up at startup
extern float g_peDegToRad;

// names for emitters the scripts create without one
static int s_emitterCount;
static char s_emitterName[0x32];
extern float g_peRadToDeg;

LUA_FUNCTION(CreateParticleEmitter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    int newCountVal = (int)GetNumberArg(L, 2, false);
    int prespawnVal = n >= 3 ? (int)GetNumberArg(L, 3, true) : 0;
    if (!name || !strlen(name)) {
        LuaCallSite(L);
        s_emitterCount++;
        _snprintf(s_emitterName, 0x31, "ParticleEmitter%d", s_emitterCount);
        s_emitterName[0x31] = 0;
        while (GetScriptManager()->FindByName(s_emitterName)) {
            s_emitterCount++;
            _snprintf(s_emitterName + 15, 0x22, "%d", s_emitterCount);
        }
        name = s_emitterName;
    }
    EParticleEmitter *myPe = 0;
    EDomain *domain = 0;
    EScriptObject *mySoVal = GetThreadScript(L);
    if (mySoVal) {
        EEntity *ent = mySoVal->GetEntity();
        if (ent)
            domain = ent->m_domain;
    }
    if (!domain)
        domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain("Level");
    myPe = CreateEntity<EParticleEmitter>(domain, name);
    myPe->SetMaxParticles(newCountVal);
    if (prespawnVal)
        myPe->Prespawn(prespawnVal);
    lua_pushrawnumber(L, (int)myPe);
    return 1;
}

LUA_FUNCTION(CreateLineParticleEmitter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    bool done = false;
    const char *name = GetStringArg(L, 1, false);
    int theCount = (int)GetNumberArg(L, 2, false);
    bool soft = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    if (!name || !strlen(name)) {
        LuaCallSite(L);
        s_emitterCount++;
        _snprintf(s_emitterName, 0x31, "ParticleEmitter%d", s_emitterCount);
        s_emitterName[0x31] = 0;
        while (GetScriptManager()->FindByName(s_emitterName)) {
            s_emitterCount++;
            _snprintf(s_emitterName + 15, 0x22, "%d", s_emitterCount);
        }
        name = s_emitterName;
    }
    EParticleEmitter *thePe = 0;
    EDomain *myDomain = 0;
    EScriptObject *soVal = GetThreadScript(L);
    if (soVal) {
        EEntity *ent = soVal->GetEntity();
        if (ent)
            myDomain = ent->m_domain;
    }
    if (!myDomain)
        myDomain = g_game->GetLevelManager()->GetDomainManager()->FindDomain("Level");
    if (soft) {
        thePe = (EParticleEmitter *)CreateEntity<ESoftLineParticleEmitter>(myDomain, name);
        thePe->SetMaxParticles(theCount);
    } else {
        thePe = (EParticleEmitter *)CreateEntity<ELineParticleEmitter>(myDomain, name);
        thePe->SetMaxParticles(theCount);
    }
    lua_pushrawnumber(L, (int)thePe);
    return 1;
}

LUA_FUNCTION(CreateMeshEmitter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    const char *newName = GetStringArg(L, 1, false);
    const char *curMesh = GetStringArg(L, 2, false);
    int theCount = (int)GetNumberArg(L, 3, false);
    int prespawn = n >= 4 ? (int)GetNumberArg(L, 4, true) : 0;
    if (!newName || !strlen(newName)) {
        LuaCallSite(L);
        s_emitterCount++;
        _snprintf(s_emitterName, 0x31, "ParticleEmitter%d", s_emitterCount);
        s_emitterName[0x31] = 0;
        while (GetScriptManager()->FindByName(s_emitterName)) {
            s_emitterCount++;
            _snprintf(s_emitterName + 15, 0x22, "%d", s_emitterCount);
        }
        newName = s_emitterName;
    }
    if (!curMesh || !strlen(curMesh)) {
        GetDiagnosticSink()->Printf("ScriptVM: CreateMeshEmitter error: invalid newName - '%s'", curMesh ? curMesh : "NULL");
        DumpLuaStack(L, 0, 0);
        return 0;
    }
    EDomain *myDomain = 0;
    EScriptObject *so = GetThreadScript(L);
    if (so) {
        EEntity *ent = so->GetEntity();
        if (ent)
            myDomain = ent->m_domain;
    }
    if (!myDomain)
        myDomain = g_game->m_mainDomain;
    bool loaded = GetMeshManager()->PreloadMesh(curMesh, 4);
    if (!loaded) {
        LuaCallSite(L);
        return 0;
    }
    EMeshEmitter *peVal = 0;
    peVal = CreateEntity<EMeshEmitter>(myDomain, newName);
    strncpy(peVal->m_meshName, curMesh, 0x1ff);
    peVal->m_meshName[0x1ff] = 0;
    peVal->SetMaxParticles(theCount);
    peVal->Rebuild();
    if (prespawn)
        peVal->Prespawn(prespawn);
    lua_pushrawnumber(L, (int)peVal);
    return 1;
}

LUA_FUNCTION(GetMeshEmitterMeshEntity)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!ent)
        return 0;
    if (!ent->IsA(&EParticleEmitter::s_class)) {
        ScriptError("ScriptVM: GetMeshEmitterMeshEntity called on non ParticleEmitter entity");
        return 0;
    }
    EParticleEmitter *newPe = (EParticleEmitter *)ent;
    if (!newPe->IsMeshEmitter()) {
        ScriptError("ScriptVM: GetMeshEmitterMeshEntity called on non MeshEmitter entity");
        return 0;
    }
    EMeshEmitter *mesh = (EMeshEmitter *)newPe;
    lua_getref(L, mesh->GetMeshEntity()->GetScriptObject()->m_ref);
    return 1;
}

LUA_FUNCTION(PEDestroyEmitter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    g_game->DestroyEmitter(pPe);
    return 0;
}

LUA_FUNCTION(EntityIsPE)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    int nIs = ent->IsType(2);
    lua_pushint(L, nIs);
    return 1;
}

LUA_FUNCTION(PESetMaxParticles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int iCount = (int)GetNumberArg(L, 2, false);
    if (iCount < 0) {
        ScriptError("ScriptVM: PESetMaxParticles called with non-positive particle iCount");
        return 0;
    }
    pPe->SetMaxParticles(iCount);
    return 0;
}

LUA_FUNCTION(PEGetMaxParticles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int nValue = pPe->m_maxParticles;
    lua_pushint(L, nValue);
    return 1;
}

LUA_FUNCTION(PEGetNumActiveParticles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int nValue = pPe->m_numActive;
    lua_pushint(L, nValue);
    return 1;
}

LUA_FUNCTION(PEInjectParticles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int count = (int)GetNumberArg(L, 2, false);
    if (count <= 0) {
        ScriptError("ScriptVM: PEInjectParticles called with non-positive particle count");
        return 0;
    }
    pPe->InjectParticles(count);
    return 0;
}

LUA_FUNCTION(PETickEmitter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float secs = GetNumberArg(L, 2, false);
    float t;
    if (secs > 15.0) {
        ScriptError("ScriptVM: PETickEmitter called with large tick time");
    }
    for (t = 0.0f; t < secs; t += 0.1) {
        pPe->SetPaused(true);
        pPe->Tick(0.1f);
    }
    return 0;
}

LUA_FUNCTION(PEReset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    bool ifDone = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!pPe)
        return 0;
    if (!ifDone || !pPe->IsTimed())
        pPe->Reset();
    else {
        ETimedEmitter *timed = (ETimedEmitter *)pPe;
        if (timed->IsDone())
            timed->Reset();
    }
    return 0;
}

LUA_FUNCTION(PESetTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    EResource *pTex = (EResource *)GetResourceArg(L, 2);
    bool flag = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    if (!pTex)
        return 0;
    if (flag)
        pTex->m_flags |= 1;
    pPe->m_texture = pTex->GetHandle();
    return 0;
}

LUA_FUNCTION(PEGetTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    void *theTexture = pPe->m_texture;
    if (!theTexture)
        lua_pushstring(L, "");
    else {
        EResource *res = g_game->m_resources->Get(theTexture);
        lua_pushstring(L, res->m_name);
    }
    return 1;
}

LUA_FUNCTION(PESetDepthTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    void *theTexture = g_game->m_resources->Load(name, 1, 0);
    if (!theTexture) {
        char msg[256];
        _snprintf(msg, 255, "ScriptVM: PESetDepthTexture unable to load theTexture <%s>", name);
        msg[255] = 0;
        ScriptError(msg);
        return 0;
    }
    pPe->m_depthTexture = theTexture;
    return 0;
}

LUA_FUNCTION(PEGetDepthTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    void *theTexture = pPe->m_depthTexture;
    if (!theTexture)
        lua_pushstring(L, "");
    else {
        EResource *res = g_game->m_resources->Get(theTexture);
        lua_pushstring(L, res->m_name);
    }
    return 1;
}

LUA_FUNCTION(PESetConeAngle)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fAVal = GetNumberArg(L, 2, false);
    float b = GetNumberArg(L, 3, false);
    pPe->SetConeAngle(g_peDegToRad * fAVal, g_peDegToRad * b);
    return 0;
}

LUA_FUNCTION(PEGetConeAngle)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float myA = pPe->m_coneAngle1;
    lua_pushnumber(L, myA * g_peRadToDeg);
    float b = pPe->m_coneAngle2;
    lua_pushnumber(L, b * g_peRadToDeg);
    return 2;
}

LUA_FUNCTION(PESetParticleRotation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float aLoArg = GetNumberArg(L, 2, false);
    float hi = GetNumberArg(L, 3, false);
    bool curRandom = n >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    Vector3 aMaxRot(0.0f, 0.0f, hi);
    Vector3 newMinRot(0.0f, 0.0f, aLoArg);
    pPe->SetRotation(newMinRot, aMaxRot, curRandom);
    {
        char unused[0x28];
    }
    return 0;
}

LUA_FUNCTION(PEGetParticleRotation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLo = pPe->m_rotation1;
    lua_pushnumber(L, fLo);
    float hi = pPe->m_rotation2;
    lua_pushnumber(L, hi);
    return 2;
}

LUA_FUNCTION(PESetParticle3DRotation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 7, 8))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float minX = GetNumberArg(L, 2, false);
    float maxX = GetNumberArg(L, 3, false);
    float minY = GetNumberArg(L, 4, false);
    float theMaxY = GetNumberArg(L, 5, false);
    float myMinZ = GetNumberArg(L, 6, false);
    float myMaxZ = GetNumberArg(L, 7, false);
    bool myRandom = n >= 8 ? GetNumberArg(L, 8, true) != 0 : false;
    Vector3 pMaxRot(maxX, theMaxY, myMaxZ);
    Vector3 newMinRot(minX, minY, myMinZ);
    pPe->SetRotation(newMinRot, pMaxRot, myRandom);
    {
        char unused[0x28];
    }
    return 0;
}

LUA_FUNCTION(PESetParticleSpeed)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLoVal = GetNumberArg(L, 2, false);
    float hi = GetNumberArg(L, 3, false);
    pPe->m_speedMin = fLoVal;
    pPe->m_speedMax = hi;
    return 0;
}

LUA_FUNCTION(PEGetParticleSpeed)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLo = pPe->m_speedMin;
    lua_pushnumber(L, fLo);
    float hi = pPe->m_speedMax;
    lua_pushnumber(L, hi);
    return 2;
}

LUA_FUNCTION(PESetNumStages)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int iCount = (int)GetNumberArg(L, 2, false);
    pPe->SetNumStages(iCount);
    return 0;
}

LUA_FUNCTION(PESetActiveStage)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int stage = (int)GetNumberArg(L, 2, false);
    pPe->SetActiveStage(stage);
    return 0;
}

LUA_FUNCTION(PESetStageDuration)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float value = GetNumberArg(L, 2, false);
    pPe->SetStageDuration(value);
    return 0;
}

LUA_FUNCTION(PESetStartColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    Color theColor = GetColorArg(L, 2, false);
    pPe->SetStartColor(theColor.r, theColor.g, theColor.b, theColor.a);
    return 0;
}

LUA_FUNCTION(PEGetStartColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    Color theColor = UnpackColor(pPe->m_startColor);
    lua_pushnumber(L, theColor.r * 255.0);
    lua_pushnumber(L, theColor.g * 255.0);
    lua_pushnumber(L, theColor.b * 255.0);
    lua_pushnumber(L, theColor.a * 255.0);
    {
        char unused[0x14];
    }
    return 4;
}

LUA_FUNCTION(PESetEndColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    Color theColor = GetColorArg(L, 2, false);
    pPe->SetEndColor(theColor.r, theColor.g, theColor.b, theColor.a);
    return 0;
}

LUA_FUNCTION(PEGetEndColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    Color theColor = UnpackColor(pPe->m_endColor);
    lua_pushnumber(L, theColor.r * 255.0);
    lua_pushnumber(L, theColor.g * 255.0);
    lua_pushnumber(L, theColor.b * 255.0);
    lua_pushnumber(L, theColor.a * 255.0);
    {
        char unused[0x14];
    }
    return 4;
}

LUA_FUNCTION(PESetStartAlpha)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int iAlpha = (int)GetNumberArg(L, 2, false);
    pPe->SetStartAlpha(iAlpha / 255.0);
    return 0;
}

LUA_FUNCTION(PESetEndAlpha)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int iAlpha = (int)GetNumberArg(L, 2, false);
    pPe->SetEndAlpha(iAlpha / 255.0);
    return 0;
}

LUA_FUNCTION(PESetParticleSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float start = GetNumberArg(L, 2, false);
    float end = GetNumberArg(L, 3, false);
    pPe->SetParticleSize(start, end);
    return 0;
}

LUA_FUNCTION(PESetParticleStartEndSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fStart = GetNumberArg(L, 2, false);
    float end = GetNumberArg(L, 3, false);
    pPe->SetParticleStartEndSize(fStart, end);
    return 0;
}

LUA_FUNCTION(PEGetParticleSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    bool staged = pPe->TestFlag(0x400);
    if (staged) {
        float fStart = pPe->m_stageSizeStart;
        lua_pushnumber(L, fStart);
        float end = pPe->m_stageSizeEnd;
        lua_pushnumber(L, end);
        lua_pushnumber(L, 1.0f);
    } else {
        float fStart = pPe->m_sizeStart;
        lua_pushnumber(L, fStart);
        float end = pPe->m_sizeEnd;
        lua_pushnumber(L, end);
        lua_pushnumber(L, 1.0f);
    }
    return 3;
}

LUA_FUNCTION(PESetParticleStageSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float theValue = GetNumberArg(L, 2, false);
    pPe->SetStageSize(theValue);
    return 0;
}

LUA_FUNCTION(PESetMaxParticlesPerSec)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fPps = GetNumberArg(L, 2, false);
    pPe->m_ppsMax = fPps;
    if (pPe->m_ppsMax < pPe->m_ppsMin)
        pPe->m_ppsMin = pPe->m_ppsMax;
    return 0;
}

LUA_FUNCTION(PESetParticlesPerSec)
{
    if (lua_gettop(L) < 3 || lua_gettop(L) > 3)
        return 0;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLoVal = GetNumberArg(L, 2, false);
    float hi = GetNumberArg(L, 3, false);
    pPe->m_ppsMin = fLoVal;
    pPe->m_ppsMax = hi;
    if (pPe->m_ppsMax < pPe->m_ppsMin)
        pPe->m_ppsMin = pPe->m_ppsMax;
    return 0;
}

LUA_FUNCTION(PEGetParticlesPerSec)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLo = pPe->m_ppsMin;
    lua_pushnumber(L, fLo);
    float hi = pPe->m_ppsMax;
    lua_pushnumber(L, hi);
    return 2;
}

LUA_FUNCTION(PESetMaxStartOffset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float theValue = GetNumberArg(L, 2, false);
    pPe->SetMaxStartOffset(theValue);
    return 0;
}

LUA_FUNCTION(PEGetMaxStartOffset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float offset = pPe->m_maxStartOffset;
    lua_pushnumber(L, offset);
    return 1;
}

LUA_FUNCTION(PESetParticleLifetime)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLo = GetNumberArg(L, 2, false);
    float hi = n >= 3 ? GetNumberArg(L, 3, true) : fLo;
    pPe->m_lifetimeMin = fLo;
    pPe->m_lifetimeMax = hi;
    pPe->UpdateLifetime();
    return 0;
}

LUA_FUNCTION(PEGetParticleLifetime)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fLo = pPe->m_lifetimeMin;
    lua_pushnumber(L, fLo);
    float hi = pPe->m_lifetimeMax;
    lua_pushnumber(L, hi);
    return 2;
}

LUA_FUNCTION(PEAgeParticles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float theValue = GetNumberArg(L, 2, false);
    pPe->AgeParticles(theValue);
    return 0;
}

LUA_FUNCTION(PESetActive)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pPe->SetActive(on);
    return 0;
}

LUA_FUNCTION(PEGetActive)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->IsActive());
    return 1;
}

LUA_FUNCTION(PESetApplyGravity)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x2, on);
    return 0;
}

LUA_FUNCTION(PEGetApplyGravity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x2));
    return 1;
}

LUA_FUNCTION(PESetCollideWithGround)
{
    if (lua_gettop(L) < 2 || lua_gettop(L) > 3)
        return 0;
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    bool stop = theNVal >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    pe->SetFlag(4, on);
    pe->SetFlag(0x2000000, stop);
    return 0;
}

LUA_FUNCTION(PEGetCollideWithGround)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x4));
    return 1;
}

LUA_FUNCTION(PESetGroundStopRotation)
{
    if (lua_gettop(L) < 2 || lua_gettop(L) > 2)
        return 0;
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x1000000, on);
    return 0;
}

LUA_FUNCTION(PEGetGroundStopRotation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x1000000));
    return 1;
}

LUA_FUNCTION(PESetTexAnimInfo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 6))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int nA = (int)GetNumberArg(L, 2, false);
    int myBEnt = (int)GetNumberArg(L, 3, false);
    int c = (int)GetNumberArg(L, 4, false);
    float newRateObj = GetNumberArg(L, 5, false);
    bool flag = n >= 6 ? GetNumberArg(L, 6, true) != 0 : false;
    int nWasAnimatedVal = pPe->TestFlag(0x20);
    pPe->SetTexAnimInfo(nA, myBEnt, c, newRateObj, flag);
    if (nWasAnimatedVal)
        pPe->RestartTexAnim(nA);
    return 0;
}

LUA_FUNCTION(PEGetTexAnimInfo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int nA, b, myC;
    float d;
    pPe->GetTexAnimInfo(&nA, &b, &myC, &d);
    int aAnimatedVal = pPe->TestFlag(0x20);
    lua_pushint(L, aAnimatedVal);
    lua_pushint(L, nA);
    lua_pushint(L, b);
    lua_pushint(L, myC);
    lua_pushnumber(L, d);
    return 5;
}

LUA_FUNCTION(PESetTexAnimInfo2D)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int iA = (int)GetNumberArg(L, 2, false);
    int aB = (int)GetNumberArg(L, 3, false);
    int c = (int)GetNumberArg(L, 4, false);
    int d = (int)GetNumberArg(L, 5, false);
    float rate = GetNumberArg(L, 6, false);
    pPe->SetTexAnimInfo2D(iA, aB, c, d, rate, false);
    return 0;
}

LUA_FUNCTION(PESetParticleForce)
{
    if (lua_gettop(L) < 4 || lua_gettop(L) > 4)
        return 0;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float fXVal = GetNumberArg(L, 2, false);
    float newY = GetNumberArg(L, 3, false);
    float z = GetNumberArg(L, 4, false);
    pPe->SetParticleForce(fXVal, newY, z);
    {
        char unused[0x14];
    }
    return 0;
}

LUA_FUNCTION(PEGetParticleForce)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    Vector3 force = pPe->m_force;
    lua_pushnumber(L, force.x);
    lua_pushnumber(L, force.y);
    lua_pushnumber(L, force.z);
    return 3;
}

LUA_FUNCTION(PESetParticleBounceFactor)
{
    if (lua_gettop(L) < 2 || lua_gettop(L) > 2)
        return 0;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    float factor = GetNumberArg(L, 2, false);
    pPe->m_bounceFactor = factor;
    return 0;
}

LUA_FUNCTION(PESetParticleBlendMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    unsigned int mode = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    pPe->m_blendMode = mode;
    return 0;
}

LUA_FUNCTION(PEGetParticleBlendMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    int value = pPe->m_blendMode;
    lua_pushint(L, value);
    return 1;
}

LUA_FUNCTION(LPEDrawWaveyLine)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 13, 13))
        return 0;
    ELineParticleEmitter *lpe = (ELineParticleEmitter *)GetEntityArg(L, 1, false, &ELineParticleEmitter::s_class);
    if (!lpe)
        return 0;
    float x1 = GetNumberArg(L, 2, false);
    float y1 = GetNumberArg(L, 3, false);
    float z1 = GetNumberArg(L, 4, false);
    Vector3 start(x1, y1, z1);
    float x2 = GetNumberArg(L, 5, false);
    float y2 = GetNumberArg(L, 6, false);
    float z2 = GetNumberArg(L, 7, false);
    Vector3 end(x2, y2, z2);
    int segments = (int)GetNumberArg(L, 8, false);
    Color color = GetColorArg(L, 9, false);
    float width = GetNumberArg(L, 13, false);
    EMatrix mat = g_identityMatrix;
    lpe->DrawLine(&mat, start, end, segments, width, color);
    return 0;
}

LUA_FUNCTION_TODO(LPEDrawWaveyCircle)

LUA_FUNCTION_TODO(LPEDrawWaveyEllipse)

LUA_FUNCTION_TODO(LPEDrawWaveyHelix)

LUA_FUNCTION(LPESetJointNoDraw)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    bool bOnVal = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    EBone *bone = pMesh->FindBone(name);
    if (bone)
        bone->m_noDraw = bOnVal;
    else
        NativeDiagnostic("ScriptVM: LPESetJointNoDraw can't find joint '%s'", name);
    return 0;
}

LUA_FUNCTION(LPESetJointNoDrawChildren)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    bool bOnVal = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    EBone *bone = pMesh->FindBone(name);
    if (bone)
        bone->m_noDrawChildren = bOnVal;
    else
        NativeDiagnostic("ScriptVM: LPESetJointNoDrawChildren can't find joint '%s'", name);
    return 0;
}

LUA_FUNCTION(LPESetJointNoDrawSkip)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    bool bOnVal = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    EBone *bone = pMesh->FindBone(name);
    if (bone)
        bone->m_noDrawSkip = bOnVal;
    else
        NativeDiagnostic("ScriptVM: LPESetJointNoDrawSkip can't find joint '%s'", name);
    return 0;
}

LUA_FUNCTION(LPESetJointFlare)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    bool bOnVal = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    EBone *bone = pMesh->FindBone(name);
    if (bone)
        bone->m_flare = bOnVal;
    else
        NativeDiagnostic("ScriptVM: LPESetJointFlare can't find joint '%s'", name);
    return 0;
}

LUA_FUNCTION_TODO(LPEDrawWaveySkeleton)

LUA_FUNCTION_TODO(LPEDrawWaveyCurve)

LUA_FUNCTION(LPESetPeriod)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    ELineParticleEmitter *curLpe = (ELineParticleEmitter *)GetEntityArg(L, 1, false, &ELineParticleEmitter::s_class);
    if (!curLpe)
        return 0;
    float fLo = GetNumberArg(L, 2, false);
    float hi = GetNumberArg(L, 3, false);
    curLpe->m_periodMin = fLo;
    curLpe->m_periodMax = hi;
    return 0;
}

LUA_FUNCTION(LPESetSoftPeriod)
{
    return LPESetPeriod_impl(L);
}

LUA_FUNCTION(LPESetFrequency)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ELineParticleEmitter *lpe = (ELineParticleEmitter *)GetEntityArg(L, 1, false, &ELineParticleEmitter::s_class);
    if (!lpe)
        return 0;
    float fValueVal = GetNumberArg(L, 2, false);
    lpe->m_frequency = fValueVal;
    return 0;
}

LUA_FUNCTION(LPESetAmplitude)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ELineParticleEmitter *lpe = (ELineParticleEmitter *)GetEntityArg(L, 1, false, &ELineParticleEmitter::s_class);
    if (!lpe)
        return 0;
    float theValue = GetNumberArg(L, 2, false);
    lpe->m_amplitude = theValue;
    return 0;
}

LUA_FUNCTION(LPESetDensity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ELineParticleEmitter *lpe = (ELineParticleEmitter *)GetEntityArg(L, 1, false, &ELineParticleEmitter::s_class);
    if (!lpe)
        return 0;
    float fValueVal = GetNumberArg(L, 2, false);
    lpe->m_density = fValueVal;
    return 0;
}

LUA_FUNCTION_TODO(PESetDistribution)

LUA_FUNCTION(PESetDistributionFlag)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *curPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!curPe)
        return 0;
    unsigned int flag = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    bool on = iN >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    if (curPe->GetSource()) {
        if (on)
            curPe->GetSource()->AddFlags(flag);
        else
            curPe->GetSource()->RemoveFlags(flag);
    }
    return 0;
}

LUA_FUNCTION(PESetText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    const char *szText = GetStringArg(L, 2, false);
    EParticleSource *source = pPe->m_source;
    if (source->GetType() == 6) {
        source->SetText(szText);
        int count = 0;
        const char *p = szText;
        while (*p) {
            if (*p != '\n')
                count++;
            p++;
        }
        if (pPe->GetMaxParticles() < count)
            pPe->SetMaxParticles(count);
        pPe->Reset();
        pPe->Prespawn(count);
    }
    return 0;
}

LUA_FUNCTION(PESetRelative)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x100, on);
    return 0;
}

LUA_FUNCTION(PEGetRelative)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x100));
    return 1;
}

LUA_FUNCTION(PESetPointSprite)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x4000, on);
    return 0;
}

LUA_FUNCTION(PEGetPointSprite)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x4000));
    return 1;
}

LUA_FUNCTION(PESetOriented)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *curPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!curPe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    bool myFlat = iN >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    curPe->SetFlag(0x8000, on);
    curPe->SetFlag(0x800000, myFlat);
    if (!curPe->m_meshLoaded)
        curPe->SetMaxParticles(curPe->m_maxParticles);
    return 0;
}

LUA_FUNCTION(PEGetOriented)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x8000));
    return 1;
}

LUA_FUNCTION(PESetZSprite)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x10000, on);
    return 0;
}

LUA_FUNCTION(PEGetZSprite)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x10000));
    return 1;
}

LUA_FUNCTION(PESetNoSpread)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x80, on);
    return 0;
}

LUA_FUNCTION(PEGetNoSpread)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x80));
    return 1;
}

LUA_FUNCTION(PESetDepthSort)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x20000, on);
    return 0;
}

LUA_FUNCTION(PEGetDepthSort)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x20000));
    return 1;
}

LUA_FUNCTION_TODO(PESetGlare)

LUA_FUNCTION(PEGetGlare)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    bool theA = (pPe->m_flags & 0x80000) != 0;
    bool b = (pPe->m_flags & 0x100000) != 0;
    lua_pushnumber(L, theA);
    lua_pushnumber(L, b);
    return 2;
}

LUA_FUNCTION(PESetLit)
{
    int theNVal = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x8000000, on);
    return 0;
}

LUA_FUNCTION(PEGetLit)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pPe)
        return 0;
    lua_pushnumber(L, pPe->TestFlag(0x8000000));
    return 1;
}

LUA_FUNCTION(LPEDeactivateSoftEmitter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ELineParticleEmitter *lpe = (ELineParticleEmitter *)GetEntityArg(L, 1, false, &ELineParticleEmitter::s_class);
    if (!lpe)
        return 0;
    lpe->Deactivate();
    return 0;
}

LUA_FUNCTION(PESetMeshName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMeshEmitter *pMesh = (EMeshEmitter *)GetEntityArg(L, 1, false, &EMeshEmitter::s_class);
    if (!pMesh)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    strncpy(pMesh->m_meshName, name, 0x1ff);
    pMesh->m_meshName[0x1ff] = 0;
    if (!pMesh->m_meshLoaded)
        pMesh->SetMaxParticles(pMesh->m_maxParticles);
    return 0;
}

LUA_FUNCTION(PEAttachChild)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *parent = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!parent)
        return 0;
    EParticleEmitter *curChild = (EParticleEmitter *)GetEntityArg(L, 2, false, &EParticleEmitter::s_class);
    if (!curChild)
        return 0;
    curChild->AttachChild(parent);
    return 0;
}

LUA_FUNCTION(PEDetachChild)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    pe->DetachChild();
    return 0;
}

LUA_FUNCTION(PESetSortBuddy)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    ETriggerBase *theBuddy = GetTriggerArg(L, 2, false);
    if (!theBuddy)
        return 0;
    bool flag = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    pe->m_sortBuddy = theBuddy;
    return 0;
}

LUA_FUNCTION(PESetScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    float x = GetNumberArg(L, 2, false);
    float fY = n >= 3 ? GetNumberArg(L, 3, true) : x;
    pe->SetScale(x, fY);
    return 0;
}

LUA_FUNCTION(PEGetScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    float x, newY;
    pe->GetScale(&x, &newY);
    lua_pushnumber(L, x);
    lua_pushnumber(L, newY);
    return 2;
}

LUA_FUNCTION(PESetMaterialColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    Color theColor = GetColorArg(L, 2, false);
    pe->m_materialColor = theColor;
    return 0;
}

LUA_FUNCTION(PEGetMaterialColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    Color theColor = pe->m_materialColor;
    lua_pushnumber(L, theColor.r * 255.0);
    lua_pushnumber(L, theColor.g * 255.0);
    lua_pushnumber(L, theColor.b * 255.0);
    lua_pushnumber(L, theColor.a * 255.0);
    return 4;
}

LUA_FUNCTION(PESetPostOnly)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x400000, on);
    return 0;
}

LUA_FUNCTION(PEGetPostOnly)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = (pe->m_flags & 0x400000) != 0;
    lua_pushnumber(L, on ? 1.0f : 0.0f);
    return 1;
}

LUA_FUNCTION(PESetNoZBuffer)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x40000, on);
    return 0;
}

LUA_FUNCTION(PEGetNoZBuffer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *myPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!myPe)
        return 0;
    bool on = (myPe->m_flags & 0x40000) != 0;
    lua_pushnumber(L, on ? 1.0f : 0.0f);
    return 1;
}

LUA_FUNCTION(PESetDistortion)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x4000000, on);
    return 0;
}

LUA_FUNCTION(PEGetDistortion)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *myPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!myPe)
        return 0;
    bool on = (myPe->m_flags & 0x4000000) != 0;
    lua_pushnumber(L, on ? 1.0f : 0.0f);
    return 1;
}

LUA_FUNCTION(PESetRelGravForce)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x10000000, on);
    return 0;
}

LUA_FUNCTION(PEGetRelGravForce)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *myPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!myPe)
        return 0;
    bool on = (myPe->m_flags & 0x10000000) != 0;
    lua_pushnumber(L, on ? 1.0f : 0.0f);
    return 1;
}

LUA_FUNCTION(PESetAlwaysDraw)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    pe->SetFlag(0x20000000, on);
    return 0;
}

LUA_FUNCTION(PEGetAlwaysDraw)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EParticleEmitter *myPe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!myPe)
        return 0;
    bool on = (myPe->m_flags & 0x20000000) != 0;
    lua_pushnumber(L, on ? 1.0f : 0.0f);
    return 1;
}

LUA_FUNCTION(CreateSphericalParticleDeflector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    const char *name = lua_tostring(L, 1);
    float radius = lua_tonumber(L, 2);
    float curFalloffVal = radius;
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);
    float newZCur = lua_tonumber(L, 5);
    if (!name)
        return 0;
    EParticleAffector *theAff = 0;
    EDomain *domain = 0;
    EScriptObject *soPtr = GetThreadScript(L);
    if (soPtr) {
        EEntity *ent = soPtr->GetEntity();
        if (ent)
            domain = ent->m_domain;
    }
    if (!domain)
        domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain("Level");
    theAff = CreateEntity<EParticleAffector>(domain, name);
    theAff->Setup(8, radius, curFalloffVal, x, y, newZCur);
    bool newOkVal = theAff->Load("ParticleEmitter", name, 0, 0, 0, true);
    if (newOkVal == true)
        lua_getref(L, theAff->GetScriptObject()->m_ref);
    else {
        delete (EEntityVirtuals *)theAff;
        lua_pushnil(L);
    }
    return 1;
}

LUA_FUNCTION(CreateCylindricalParticleDeflector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    float radius = GetNumberArg(L, 2, false);
    float height = GetNumberArg(L, 3, false);
    if (!name)
        return 0;
    EParticleAffector *theAff = 0;
    EDomain *domain = 0;
    EScriptObject *soPtr = GetThreadScript(L);
    if (soPtr) {
        EEntity *ent = soPtr->GetEntity();
        if (ent)
            domain = ent->m_domain;
    }
    if (!domain)
        domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain("Level");
    theAff = CreateEntity<EParticleAffector>(domain, name);
    theAff->Setup(0x10, height, radius, 0.0f, 0.0f, 0.0f);
    bool newOkVal = theAff->Load("dummy", name, 0, 0, 0, true);
    if (newOkVal == true)
        lua_getref(L, theAff->GetScriptObject()->m_ref);
    else {
        delete (EEntityVirtuals *)theAff;
        lua_pushnil(L);
    }
    return 1;
}

LUA_FUNCTION(PASetInverse)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleAffector *aff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!aff)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    if (on)
        aff->m_type |= 0x20000;
    else
        aff->m_type &= ~0x20000;
    return 0;
}

LUA_FUNCTION(CreateSphericalParticleAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    const char *name = lua_tostring(L, 1);
    float radius = lua_tonumber(L, 2);
    float curFalloffVal = radius;
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);
    float newZCur = lua_tonumber(L, 5);
    if (!name)
        return 0;
    EParticleAffector *theAff = 0;
    EDomain *domain = 0;
    EScriptObject *soPtr = GetThreadScript(L);
    if (soPtr) {
        EEntity *ent = soPtr->GetEntity();
        if (ent)
            domain = ent->m_domain;
    }
    if (!domain)
        domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain("Level");
    theAff = CreateEntity<EParticleAffector>(domain, name);
    theAff->Setup(1, radius, curFalloffVal, x, y, newZCur);
    bool newOkVal = theAff->Load("dummy", name, 0, 0, 0, true);
    if (newOkVal == true)
        lua_getref(L, theAff->GetScriptObject()->m_ref);
    else {
        delete (EEntityVirtuals *)theAff;
        lua_pushnil(L);
    }
    return 1;
}

LUA_FUNCTION(CreateCylindricalParticleAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    const char *name = lua_tostring(L, 1);
    float radius = lua_tonumber(L, 2);
    float curFalloffVal = lua_tonumber(L, 3);
    float x = lua_tonumber(L, 4);
    float y = lua_tonumber(L, 5);
    float newZCur = lua_tonumber(L, 6);
    if (!name)
        return 0;
    EParticleAffector *theAff = 0;
    EDomain *domain = 0;
    EScriptObject *soPtr = GetThreadScript(L);
    if (soPtr) {
        EEntity *ent = soPtr->GetEntity();
        if (ent)
            domain = ent->m_domain;
    }
    if (!domain)
        domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain("Level");
    theAff = CreateEntity<EParticleAffector>(domain, name);
    theAff->Setup(2, radius, curFalloffVal, x, y, newZCur);
    bool newOkVal = theAff->Load("dummy", name, 0, 0, 0, true);
    if (newOkVal == true)
        lua_getref(L, theAff->GetScriptObject()->m_ref);
    else {
        delete (EEntityVirtuals *)theAff;
        lua_pushnil(L);
    }
    return 1;
}

LUA_FUNCTION(SetPrimaryAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    EParticleAffector *aff = (EParticleAffector *)GetEntityArg(L, 2, false, &EParticleAffector::s_class);
    if (!aff)
        return 0;
    pe->SetPrimaryAffector(aff);
    {
        char unused[0x28];
    }
    return 0;
}

LUA_FUNCTION(SetSecondaryAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    EParticleAffector *aff = (EParticleAffector *)GetEntityArg(L, 2, false, &EParticleAffector::s_class);
    if (!aff)
        return 0;
    pe->SetSecondaryAffector(aff);
    {
        char unused[0x28];
    }
    return 0;
}

LUA_FUNCTION(PESetAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    EParticleAffector *aAffVal = (EParticleAffector *)GetEntityArg(L, 2, false, &EParticleAffector::s_class);
    if (!aAffVal)
        return 0;
    int index = (int)GetNumberArg(L, 3, false);
    pe->SetAffector(index, aAffVal);
    {
        char unused[0x10];
    }
    return 0;
}

LUA_FUNCTION(PEAddAffector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleEmitter *thePeArg = (EParticleEmitter *)GetEntityArg(L, 1, false, &EParticleEmitter::s_class);
    if (!thePeArg)
        return 0;
    EParticleAffector *aAffVal = (EParticleAffector *)GetEntityArg(L, 2, false, &EParticleAffector::s_class);
    if (!aAffVal)
        return 0;
    int index = thePeArg->AddAffector(aAffVal);
    lua_pushint(L, index);
    {
        char unused[0x30];
    }
    return 1;
}

LUA_FUNCTION(PASetActive)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleAffector *aff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!aff)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    aff->m_active = on;
    return 0;
}

LUA_FUNCTION(PASetType)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EParticleAffector *aff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!aff)
        return 0;
    unsigned int type = (unsigned int)lua_tonumber(L, 2);
    aff->m_type &= 0xffff0000;
    aff->m_type |= type;
    return 0;
}

LUA_FUNCTION(PASetFlag)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EParticleAffector *aff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!aff)
        return 0;
    unsigned int flag = (unsigned int)lua_tonumber(L, 2);
    bool on = iN >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    aff->SetFlag(flag, on);
    return 0;
}

LUA_FUNCTION(PASetDimensions)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EParticleAffector *theAff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!theAff)
        return 0;
    float theW = GetNumberArg(L, 2, false);
    float fH = GetNumberArg(L, 3, false);
    theAff->m_width = theW;
    theAff->m_height = fH;
    theAff->m_widthSq = theW * theW;
    theAff->m_heightSq = fH * fH;
    if (theW > 0.0)
        theAff->m_invWidthSq = 1.0f / theAff->m_widthSq;
    else
        theAff->m_invWidthSq = 1.0f;
    if (fH > 0.0)
        theAff->m_invHeightSq = 1.0f / theAff->m_heightSq;
    else
        theAff->m_invHeightSq = 1.0f;
    return 0;
}

LUA_FUNCTION(PASetForces)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EParticleAffector *curAff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!curAff)
        return 0;
    float myAVal = GetNumberArg(L, 2, false);
    float b = GetNumberArg(L, 3, false);
    float c = GetNumberArg(L, 4, false);
    curAff->m_force1 = myAVal;
    curAff->m_force2 = b;
    curAff->m_force3 = c;
    return 0;
}

LUA_FUNCTION(PASetTurbulence)
{
    int aN = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EParticleAffector *theAff = (EParticleAffector *)GetEntityArg(L, 1, false, &EParticleAffector::s_class);
    if (!theAff)
        return 0;
    float x = GetNumberArg(L, 2, false);
    float y = GetNumberArg(L, 3, false);
    float z = GetNumberArg(L, 4, false);
    theAff->m_turbulence.Set(x, y, z);
    return 0;
}

LUA_FUNCTION(TESetTranslationOffsets)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int iIndex = (unsigned int)GetNumberArg(L, 2, false);
    Vector3 offset(GetNumberArg(L, 3, false), GetNumberArg(L, 4, false), GetNumberArg(L, 5, false));
    EEmitterEntry &entry = te->GetEntry(iIndex);
    return 0;
}

LUA_FUNCTION(TEGetTranslationOffsets)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 2, false);
    if (index >= te->m_entries.m_count)
        return 0;
    EEmitterEntry &entryVal = te->GetValidEntry(index);
    Vector3 offset(0.0f, 0.0f, 0.0f);
    PushVector(L, offset);
    {
        char unused[0xc];
    }
    return 3;
}

LUA_FUNCTION(TESetRotationOffsets)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    int iIndex = (int)GetNumberArg(L, 2, false);
    Vector3 offset(GetNumberArg(L, 3, false), GetNumberArg(L, 4, false), GetNumberArg(L, 5, false));
    EEmitterEntry &entry = te->GetEntry(iIndex);
    return 0;
}

LUA_FUNCTION(TEGetRotationOffsets)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 2, false);
    if (index >= te->m_entries.m_count)
        return 0;
    EEmitterEntry &entryVal = te->GetValidEntry(index);
    Vector3 offset(0.0f, 0.0f, 0.0f);
    PushVector(L, offset);
    {
        char unused[0xc];
    }
    return 3;
}

LUA_FUNCTION(TESetPE)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 2, false);
    EParticleEmitter *pe = (EParticleEmitter *)GetEntityArg(L, 3, false, &EParticleEmitter::s_class);
    if (!pe)
        return 0;
    EEmitterEntry &entry = te->GetEntry(index);
    entry.m_emitter = pe;
    return 0;
}

LUA_FUNCTION(TEGetPE)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 2, false);
    if (index >= te->m_entries.m_count)
        return 0;
    EEmitterEntry &entry = te->GetEntry(index);
    if (!entry.m_emitter)
        return 0;
    lua_pushrawnumber(L, (int)entry.m_emitter);
    return 1;
}

LUA_FUNCTION(TESetPA)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int iIndex = (unsigned int)GetNumberArg(L, 2, false);
    unsigned int iWhich = (unsigned int)GetNumberArg(L, 3, false);
    EParticleAffector *pa = (EParticleAffector *)GetEntityArg(L, 4, false, &EParticleAffector::s_class);
    if (!pa)
        return 0;
    if (iIndex >= te->m_entries.m_count)
        return 0;
    EEmitterEntry &entry = te->GetEntry(iIndex);
    if (iWhich == 0)
        entry.m_affectors[0] = pa;
    else if (iWhich == 1)
        entry.m_affectors[1] = pa;
    return 0;
}

LUA_FUNCTION(TEGetPA)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int iIndex = (unsigned int)GetNumberArg(L, 2, false);
    unsigned int iWhich = (unsigned int)GetNumberArg(L, 3, false);
    if (iIndex >= te->m_entries.m_count)
        return 0;
    EEmitterEntry &entry = te->GetValidEntry(iIndex);
    EParticleAffector *pa = iWhich == 0 ? entry.m_affectors[0] : entry.m_affectors[1];
    if (!pa || pa->IsDeleted())
        return 0;
    lua_getref(L, pa->GetScriptObject()->m_ref);
    return 1;
}

LUA_FUNCTION(TEKillPE)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEmitterGroup *te = (EEmitterGroup *)GetEntityArg(L, 1, false, &EEmitterGroup::s_class);
    if (!te)
        return 0;
    unsigned int index = (unsigned int)GetNumberArg(L, 2, false);
    if (index >= te->m_entries.m_count)
        return 0;
    EEmitterEntry &entry = te->GetValidEntry(index);
    g_game->DestroyEmitter(entry.m_emitter);
    if (entry.m_affectors[0] && !entry.m_affectors[0]->IsDeleted())
        g_game->m_scripts.KillScript(entry.m_affectors[0]->GetScriptObject());
    if (entry.m_affectors[1] && !entry.m_affectors[1]->IsDeleted())
        g_game->m_scripts.KillScript(entry.m_affectors[1]->GetScriptObject());
    entry.m_emitter = 0;
    entry.m_affectors[1] = 0;
    entry.m_affectors[0] = 0;
    return 0;
}
