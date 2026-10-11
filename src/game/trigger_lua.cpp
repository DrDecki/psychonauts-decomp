// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

enum {
    TRIGGER_OBB = 1,
    TRIGGER_SPHERE = 2,
    TRIGGER_MESH = 4
};

class ETriggerVolume {
public:
    virtual void Delete(int flags);                     // 0x00
    virtual bool AddListener(EScriptObject *listener);  // 0x04
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void GetAABB(EAABB *box);                   // 0x14
    virtual void Vf18();
    virtual void Vf1c();
    virtual void Vf20();
    virtual void Vf24();
    virtual void Vf28();
    virtual void Vf2c();
    virtual const char *GetName();                      // 0x30

    bool ContainsEntity(EEntity *ent);
    bool RemoveListener(EScriptObject *listener);
    int GetType() { return m_type; }
    int GetHandle() { return m_handle; }

    char pad4[0x1c - 4];
    int m_type;                     // 0x1c
    int m_handle;                   // 0x20
};

struct ESphere {
    ESphere() {}
    ESphere(const Vector3 &c, float r) : center(c) { radius = r; }
    Vector3 center;
    float radius;
};

class ETriggerSphere : public ETriggerVolume {
public:
    void SetSphere(const ESphere &sphere);
    float GetRadius() { return m_radius; }

    char pad24[0x34 - 0x24];
    Vector3 m_center;               // 0x34
    char pad40[0x4c - 0x40];
    float m_radius;                 // 0x4c
};

struct EOBBShape {
    char pad0[4];
    EAABB box;                      // 0x04
};

class ETriggerOBB : public ETriggerVolume {
public:
    char pad24[0x38 - 0x24];
    EOBBShape *m_shape;             // 0x38
    char pad3c[0x80 - 0x3c];
    unsigned int m_pointCheck : 1;  // 0x80
};

class ETriggerA8 {
public:
    ETriggerOBB *FindOBB(const char *name);
};

class EOBBManager {
public:
    ETriggerOBB *FindOBB(const char *name);
};

ETriggerVolume *LookupTriggerVolume(void *handle);
ETriggerSphere *CreateTriggerSphere(const ESphere &sphere, EEntity *owner);
void DeleteTriggerSphere(ETriggerSphere *sphere);
void DumpTriggerOBBList();

static ETriggerVolume *GetTriggerVolumeArg(lua_State *L, int idx, bool optional)
{
    void *handle = lua_torawvalue(L, idx);
    ETriggerVolume *t = LookupTriggerVolume(handle);
    ETriggerVolume *result = 0;
    if (t) {
        int type = t->m_type;
        if (type == TRIGGER_OBB || type == TRIGGER_SPHERE || type == TRIGGER_MESH)
            result = t;
    }
    if (!result && !optional)
        LuaCallSite(L);
    return result;
}

static void PushTriggerSphere(lua_State *L, ETriggerSphere *sphere)
{
    lua_pushrawnumber(L, (int)sphere);
}

static ETriggerSphere *GetTriggerSphereArg(lua_State *L, int idx, bool optional)
{
    ETriggerVolume *t = GetTriggerVolumeArg(L, idx, optional);
    if (t && t->GetType() == TRIGGER_SPHERE) {
        return (ETriggerSphere *)t;
    } else {
        if (!optional)
            LuaCallSite(L);
        return 0;
    }
}

static void PushTriggerOBB(lua_State *L, ETriggerOBB *obb)
{
    lua_pushrawnumber(L, obb->GetHandle());
}

static ETriggerOBB *GetTriggerOBBArg(lua_State *L, int idx, bool optional)
{
    ETriggerVolume *t = GetTriggerVolumeArg(L, idx, optional);
    if (t && t->GetType() == TRIGGER_OBB) {
        return (ETriggerOBB *)t;
    } else {
        if (!optional)
            LuaCallSite(L);
        return 0;
    }
}


LUA_FUNCTION(Trigger_ContainsEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerVolume *t = GetTriggerVolumeArg(L, 1, false);
    if (!t)
        return 0;
    EEntity *ent = GetEntityArg(L, 2, false, 0);
    if (!ent)
        return 0;
    bool inside = t->ContainsEntity(ent);
    lua_pushnumber(L, inside);
    return 1;
}

LUA_FUNCTION(Trigger_RegisterListener)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerVolume *t = GetTriggerVolumeArg(L, 1, false);
    if (!t)
        return 0;
    EScriptObject *obj = GetScriptObjectArg(L, 2, false, false);
    if (!obj)
        return 0;
    bool quiet = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    if (!t->AddListener(obj)) {
        if (!quiet) {
            LuaCallSite(L);
            DebugPrintf("trigger %x: could not add %s", t ? t->GetHandle() : 0xdead00, obj ? obj->GetName() : "a NULL object");
        }
        return 0;
    }
    return 0;
}

LUA_FUNCTION(Trigger_UnregisterListener)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerVolume *t = GetTriggerVolumeArg(L, 1, false);
    if (!t)
        return 0;
    EScriptObject *obj = GetScriptObjectArg(L, 2, false, false);
    if (!obj)
        return 0;
    bool quiet = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    if (!t->RemoveListener(obj)) {
        if (!quiet) {
            LuaCallSite(L);
            obj->GetName();
        }
        return 0;
    }
    return 0;
}

LUA_FUNCTION_TODO(Trigger_GetListeners)

LUA_FUNCTION(TriggerSphere_New)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 5))
        return 0;
    Vector3 pCenter2(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    float fRadius = GetNumberArg(L, 4, false);
    EEntity *owner = n >= 5 ? GetEntityArg(L, 5, true, 0) : 0;
    ESphere sphere(pCenter2, fRadius);
    ETriggerSphere *t = CreateTriggerSphere(sphere, owner);
    PushTriggerSphere(L, t);
    return 1;
}

LUA_FUNCTION(TriggerSphere_Delete)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerSphere *t = GetTriggerSphereArg(L, 1, false);
    if (!t)
        return 0;
    DeleteTriggerSphere(t);
    return 0;
}

LUA_FUNCTION(TriggerSphere_SetSphere)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    ETriggerSphere *t = GetTriggerSphereArg(L, 1, false);
    if (!t)
        return 0;
    Vector3 pCenter2(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    float fRadius = GetNumberArg(L, 5, false);
    ESphere sphere(pCenter2, fRadius);
    t->SetSphere(sphere);
    return 0;
}

LUA_FUNCTION(TriggerSphere_SetPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ETriggerSphere *t = GetTriggerSphereArg(L, 1, false);
    if (!t)
        return 0;
    Vector3 pCenter2(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ESphere sphere(pCenter2, t->GetRadius());
    t->SetSphere(sphere);
    return 0;
}

LUA_FUNCTION(TriggerSphere_SetRadius)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerSphere *t = GetTriggerSphereArg(L, 1, false);
    if (!t)
        return 0;
    float fRadius = GetNumberArg(L, 2, false);
    ESphere sphere(t->m_center, fRadius);
    t->SetSphere(sphere);
    return 0;
}

LUA_FUNCTION(TriggerSphere_GetSphere)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerSphere *t = GetTriggerSphereArg(L, 1, false);
    if (!t)
        return 0;
    PushVector(L, t->m_center);
    lua_pushnumber(L, t->GetRadius());
    return 4;
}

LUA_FUNCTION(TriggerOBB_Lookup)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *name = GetStringArg(L, 2, false);
    if (!name)
        return 0;
    ETriggerOBB *pObb1 = 0;
    if (lua_type(L, 1) == LUA_TNIL) {
        pObb1 = ((EOBBManager *)g_game->m_unk81b4)->FindOBB(name);
    } else {
        ETriggerA8 *owner = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
        if (!owner)
            return 0;
        pObb1 = owner->FindOBB(name);
    }
    if (!pObb1)
        return 0;
    PushTriggerOBB(L, pObb1);
    return 1;
}

LUA_FUNCTION(TriggerOBB_GetName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerOBB *pObb = GetTriggerOBBArg(L, 1, false);
    if (!pObb)
        return 0;
    if (pObb) {
        lua_pushstring(L, pObb->GetName());
        return 1;
    } else {
        return 0;
    }
}

LUA_FUNCTION(TriggerOBB_SetPointCheck)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerOBB *pObb = GetTriggerOBBArg(L, 1, false);
    if (!pObb)
        return 0;
    bool bCheck = GetNumberArg(L, 2, false) != 0;
    pObb->m_pointCheck = bCheck;
    return 0;
}

LUA_FUNCTION(TriggerOBB_GetAABB)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerOBB *obb = GetTriggerOBBArg(L, 1, false);
    if (!obb)
        return 0;
    EAABB box;
    obb->GetAABB(&box);
    Vector3 center = box.GetCenter();
    PushVector(L, center);
    Vector3 extent = box.GetExtent();
    PushVector(L, extent);
    return 6;
}

LUA_FUNCTION(TriggerOBB_GetExtent)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerOBB *pObb = GetTriggerOBBArg(L, 1, false);
    if (!pObb)
        return 0;
    EAABB *pBox2 = &pObb->m_shape->box;
    Vector3 pExtent = pBox2->GetExtent();
    PushVector(L, pExtent);
    return 3;
}

LUA_FUNCTION_TODO(TriggerOBB_SetExtent)

LUA_FUNCTION_TODO(TriggerMesh_New)

LUA_FUNCTION_TODO(TriggerMesh_Delete)

LUA_FUNCTION_TODO(GetTriggerVolumeBounds)

LUA_FUNCTION_TODO(RegisterTriggerVolume)

LUA_FUNCTION_TODO(UnregisterTriggerVolume)

LUA_FUNCTION(GetTriggerVolumeOrientation)
{
    LuaCallSite(L);
    Vector3 angles(0.0f, 0.0f, 0.0f);
    PushVector(L, angles);
    return 3;
}

LUA_FUNCTION(DumpTriggerOBBs)
{
    DumpTriggerOBBList();
    return 0;
}
