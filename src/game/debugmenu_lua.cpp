// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

#include "win32.h"
#include <stdlib.h>

class ESoundManager {
public:
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void Vf14();
    virtual void Vf18();
    virtual void Vf1c();
    virtual void Vf20();
    virtual void Vf24();
    virtual void Vf28();
    virtual void Vf2c();
    virtual void Vf30();
    virtual void Vf34();
    virtual void Vf38();
    virtual void Vf3c();
    virtual void Vf40();
    virtual void Vf44();
    virtual void Vf48();
    virtual void Vf4c();
    virtual void Vf50();
    virtual void Vf54();
    virtual void Vf58();
    virtual void Vf5c();
    virtual void Vf60();
    virtual void Vf64();
    virtual void Vf68();
    virtual void Vf6c();
    virtual void Vf70();
    virtual void Vf74();
    virtual void Vf78();
    virtual void Vf7c();
    virtual void Vf80();
    virtual void Vf84();
    virtual void Vf88();
    virtual void Vf8c();
    virtual void Vf90();
    virtual void Vf94();
    virtual void Vf98();
    virtual void Vf9c();
    virtual void Vfa0();
    virtual void Vfa4();
    virtual void Vfa8();
    virtual void Vfac();
    virtual void Vfb0();
    virtual void Vfb4();
    virtual void Vfb8();
    virtual void Vfbc();
    virtual void Vfc0();
    virtual void Vfc4();
    virtual void Vfc8();
    virtual void Vfcc();
    virtual void Vfd0();
    virtual void Vfd4();
    virtual void Vfd8();
    virtual void Vfdc();
    virtual void Vfe0();
    virtual void Vfe4();
    virtual void Vfe8();
    virtual void Vfec();
    virtual void Vff0();
    virtual void Vff4();
    virtual void Vff8();
    virtual void Vffc();
    virtual void DumpSounds(bool all);  // 0x100
    virtual void DumpChannels();        // 0x104
};

class ELightManager {
public:
    void DumpInfo();
};

class EBlendShapeManager {
public:
    void DumpInfo();
    void DumpUsage();
};

class EMeshManager {
public:
    void DumpInfo();
};

class EAnimManager {
public:
    void SetCacheSize(unsigned int bytes);
};

class ECrashHandler {
public:
    void Enable(int flags);
};

struct ECamera {
    int GetType() { return m_type; }

    int m_type;
    char pad4[0x531 - 4];
    unsigned char m_531_0 : 1;
    unsigned char m_531_1 : 1;
    unsigned char m_flyCam : 1;
};

extern EAnimManager *g_animManager;
extern EMeshManager *g_meshManager;
extern ECrashHandler g_crashHandler;
void CreateMeshManager();
extern float g_globalTimerMs;
extern FILETIME g_globalTimerStart;
extern int g_fragDebugCount;
extern int g_fragDebugMin;
extern int g_fragDebugEntity;


LUA_FUNCTION(DbgMenuReset)
{
    EMenuPage *page = g_game->m_menus->GetPage("DebugPage");
    if (page)
        page->Reset();
    return 0;
}

LUA_FUNCTION(DbgMenuAppend)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    const char *pszCommand1 = GetStringArg(L, 2, false);
    const char *szArg2 = n >= 3 ? GetStringArg(L, 3, true) : 0;
    EMenuPage *page = g_game->m_menus->GetPage("DebugPage");
    if (page)
        page->Append(name, szArg2, pszCommand1);
    return 0;
}

LUA_FUNCTION(DBGMenuSetCheck)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool bCheck2 = GetNumberArg(L, 2, false) != 0;
    EMenuPage *page = g_game->m_menus->GetPage("DebugPage");
    if (page)
        page->SetCheck(name, bCheck2);
    return 0;
}

LUA_FUNCTION(DBGMenuSelect)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    EMenuPage *page = g_game->m_menus->GetPage("DebugPage");
    if (page)
        page->Select(name);
    return 0;
}

LUA_FUNCTION_TODO(DrawLine)

LUA_FUNCTION_TODO(DrawSphere)

LUA_FUNCTION_TODO(DrawText)

LUA_FUNCTION_TODO(DrawCircle)

LUA_FUNCTION_TODO(DrawCylinder)

LUA_FUNCTION_TODO(DebugDumpCollisionMem)

LUA_FUNCTION(ShowTimingInfo)
{
    int nModeInfo;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    nModeInfo = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (nModeInfo < 0)
        g_game->m_timingInfoAll = nModeInfo == -1;
    else
        g_game->m_timingInfo = nModeInfo;
    return 0;
}

LUA_FUNCTION(DebugSetGameSpeed)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float speed = GetNumberArg(L, 1, false);
    g_game->m_gameSpeed = speed;
    return 0;
}

LUA_FUNCTION(DebugCycleCollisionDisplay)
{
    g_game->m_debug.CycleCollisionDisplay();
    return 0;
}

LUA_FUNCTION(ShowDebugInfo)
{
    int nModeInfo;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    nModeInfo = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (nModeInfo == -1)
        g_game->m_debug.CycleDebugInfo();
    else if (g_game->m_debug.m_mode == nModeInfo)
        g_game->m_debug.m_mode = 0;
    else
        g_game->m_debug.m_mode = nModeInfo;
    return 0;
}

LUA_FUNCTION(SetAnimDisplayEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, true);
    g_game->m_animDisplayEntity = ent;
    return 0;
}

LUA_FUNCTION_TODO(TraceSetEnabled)

LUA_FUNCTION(DebugIs3dFlyCamera)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    ECamera *cam = g_game->GetCamera();
    bool bFly = cam->GetType() == 0 && cam->m_flyCam ? 1 : 0;
    lua_pushnumber(L, bFly);
    return 1;
}

LUA_FUNCTION(DebugToggleInGameMenu)
{
    g_game->m_menus->ToggleInGameMenu();
    return 0;
}

LUA_FUNCTION_TODO(DebugTogglePrimaryTarget)

LUA_FUNCTION(DebugCycleHighlightedSurface)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int step = n >= 1 ? (int)GetNumberArg(L, 1, true) : 1;
    g_game->m_debug.CycleHighlightedSurface(step);
    return 0;
}

LUA_FUNCTION(DebugCycleHighlightedSurface2)
{
    g_game->m_debug.CycleHighlightedSurface2();
    return 0;
}

LUA_FUNCTION_TODO(DebugResetRaz)

LUA_FUNCTION(DumpTextureInfo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_game->m_resources->DumpInfo(all);
    return 0;
}

LUA_FUNCTION(DumpLightInfo)
{
    g_game->m_lights->DumpInfo();
    return 0;
}

LUA_FUNCTION(DumpMeshManagerInfo)
{
    if (!g_meshManager)
        CreateMeshManager();
    g_meshManager->DumpInfo();
    return 0;
}

LUA_FUNCTION_TODO(DumpSkeletonInfo)

LUA_FUNCTION(DumpEntityInfo)
{
    g_game->DumpEntityInfo();
    return 0;
}

LUA_FUNCTION_TODO(DebugHookVars)

LUA_FUNCTION(DebugSetAnimCacheSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int size = (int)GetNumberArg(L, 1, false);
    if (size > 0)
        g_animManager->SetCacheSize(size);
    return 0;
}

LUA_FUNCTION_TODO(TestBitfields)

LUA_FUNCTION(StackTrace)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    int nFirst = n >= 1 ? (int)GetNumberArg(L, 1, true) : 0;
    int last = n >= 2 ? (int)GetNumberArg(L, 2, true) : 0;
    DumpLuaStack(L, nFirst, last);
    return 0;
}

LUA_FUNCTION_TODO(SetTriCache)

LUA_FUNCTION_TODO(ReportTriCache)

LUA_FUNCTION(DebugDumpSounds)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_game->m_sound->DumpSounds(all);
    return 0;
}

LUA_FUNCTION(dcs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_game->m_sound->DumpChannels();
    return 0;
}

LUA_FUNCTION(DebugDumpBlendShapes)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_blendShapes->DumpInfo();
    g_game->m_blendShapes->DumpUsage();
    return 0;
}

LUA_FUNCTION_TODO(DebugDumpMemory)

LUA_FUNCTION_TODO(DebugDumpParticles)

LUA_FUNCTION(DebugDumpBlendshape)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    void *model = ((void **)ent)[0xa8 / 4];
    return 0;
}

LUA_FUNCTION(DumpDoFileArray)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_game->m_scripts.DumpDoFileArray(all);
    return 0;
}

LUA_FUNCTION(DebugDumpBaseClassHash)
{
    g_game->m_scripts.DumpBaseClassHash();
    return 0;
}

LUA_FUNCTION(DebugGetSharedBase)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *szName1 = GetStringArg(L, 1, false);
    int ref = GetScriptManager()->GetSharedBase(szName1, "<reload>", 1);
    if (ref) {
        lua_getref(L, ref);
        return 1;
    } else {
        return 0;
    }
}

LUA_FUNCTION_NOSTACK(DebugReloadClassConstructor)
{
    char buf[256];
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    if (!GetScriptManager()->ReloadClass(name, buf, 256, "<reload>", 1)) {
        return 0;
    } else {
        lua_pushstring(L, buf);
        return 1;
    }
}

LUA_FUNCTION(StartGlobalTimer)
{
    g_globalTimerMs = g_game->m_time * 1000.0;
    GetSystemTimeAsFileTime(&g_globalTimerStart);
    return 0;
}

LUA_FUNCTION(EndGlobalTimer)
{
    SYSTEMTIME st3;
    FILETIME now;
    float ms;
    FILETIME start;
    FILETIME end2;
    FILETIME diff3;
    ms = g_game->m_time * 1000.0;
    GetSystemTimeAsFileTime(&now);
    start.dwLowDateTime = 0;
    start.dwHighDateTime = 0;
    start.dwHighDateTime = g_globalTimerStart.dwHighDateTime;
    start.dwLowDateTime = g_globalTimerStart.dwLowDateTime;
    end2.dwHighDateTime = now.dwHighDateTime;
    end2.dwLowDateTime = now.dwLowDateTime;
    *(unsigned __int64 *)&diff3 = *(unsigned __int64 *)&end2 - *(unsigned __int64 *)&start;
    now.dwHighDateTime = diff3.dwHighDateTime;
    now.dwLowDateTime = diff3.dwLowDateTime;
    FileTimeToSystemTime(&now, &st3);
    return 0;
}

LUA_FUNCTION(TestSegFault)
{
    g_crashHandler.Enable(0);
    int *p = 0;
    *p = 5;
    return 0;
}

LUA_FUNCTION(TestAssert)
{
    int x = -1;
    return 0;
}

LUA_FUNCTION(TestAssertf)
{
    int x = -1;
    return 0;
}

LUA_FUNCTION(TestAssertWarn)
{
    int x = -1;
    return 0;
}

LUA_FUNCTION(TestAssertfWarn)
{
    int x = -1;
    return 0;
}

LUA_FUNCTION(Halt)
{
    return 0;
}

LUA_FUNCTION(GetXboxMem)
{
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    lua_pushnumber(L, (ms.dwTotalPhys - ms.dwAvailPhys) / 1024.0);
    return 1;
}

LUA_FUNCTION(GetLuaMem)
{
    lua_pushnumber(L, GetScriptManager()->GetMemoryUsage() / 1024.0);
    return 1;
}

LUA_FUNCTION(WriteAllTexturesToPackFile)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    bool all = GetNumberArg(L, 1, false) != 0;
    int flags = (int)GetNumberArg(L, 2, false);
    g_game->m_resources->WriteAllToPackFile(all, flags);
    return 0;
}

LUA_FUNCTION(DebugDumpLSODupes)
{
    g_game->m_scripts.DumpLSODupes();
    return 0;
}

LUA_FUNCTION(DebugClearGlobals)
{
    lua_newtable(L);
    lua_setglobals(L);
    return 0;
}

LUA_FUNCTION(DebugGetTableSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int size = lua_gettablesize(L, 1);
    if (size < 0)
        lua_pushint(L, size);
    else
        lua_pushuint(L, size * 8);
    return 1;
}

LUA_FUNCTION(DebugDumpNonGarbage)
{
    return 0;
}

LUA_FUNCTION(Debug_GetEntityName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushstring(L, ent->GetName());
    return 1;
}

LUA_FUNCTION(Debug_GetEntityType)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushstring(L, ent->GetClass()->m_name);
    return 1;
}

LUA_FUNCTION(FragDebug_Reset)
{
    g_fragDebugCount = 0;
    g_fragDebugMin = 0x7fffffff;
    g_fragDebugEntity = -1;
    return 0;
}

LUA_FUNCTION_TODO(FragDebug_CanSee)

LUA_FUNCTION(TestExit)
{
    exit(-1);
}

LUA_FUNCTION(Debug_SetProductionLoading)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    lua_pushnumber(L, g_game->m_productionLoading);
    g_game->m_productionLoading = on;
    return 1;
}

LUA_FUNCTION_TODO(SetPCDisplayOptions)
