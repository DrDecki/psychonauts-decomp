// Script functions, file name made up.

#include <math.h>
#include "win32.h"
#include "engine.h"
#include "luafunc.h"

// virtual functions of the entities, slots are known only where the scripts call them
class EEntityVirtuals {
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
    virtual void GetLookOffsetPos(Vector3 *pos);     // 0x58
    virtual void GetLookOffsetPosLocal(Vector3 *pos);     // 0x5c
    virtual void Vf60();
    virtual void Vf64();
    virtual void Vf68();
    virtual void Vf6c();
    virtual void Vf70();
    virtual void Vf74();
    virtual void Vf78();
    virtual void Vf7c();
    virtual void LookAt(EEntity *target, int a, int b);     // 0x80
};

class EInventory {
public:
    void ItemRemoved(const char *name);
};

class ETriggerA8 {
public:
    float GetAnimSpeedScale() { return m_animSpeedScale; }
    Vector3 &GetScale() { return m_scale; }

    struct EBone *FindBone(const char *name);
    bool Contains(Vector3 *point, int flags);
    void StartMeshAnim(int anim, bool loop);
    void StopMeshAnim(bool now);
    void LockLODLevel(unsigned char level);

    char pad0[0x54 - 0x0];
    EArrayBase m_joints;                // 0x54
    char pad60[0x94 - 0x60];
    bool m_94_0 : 1;                // 0x94
    bool m_meshAnimPlaying : 1;
    char pad95[0x1b0 - 0x95];
    Vector3 m_scale;                // 0x1b0
    char pad1bc[0x1d8 - 0x1bc];
    float m_animSpeedScale;         // 0x1d8
};

struct EDomain {
    void SetSleeping(bool on);
    const char *GetName() { return m_name; }
    bool IsSleeping() { return m_sleeping; }

    char pad0[0x44];
    const char *m_name;             // 0x44
    bool m_sleeping;                // 0x48
};

class EScriptEntity {
public:
    EEntity *GetParent() { return m_parent; }
    EDomain *GetDomain() { return m_domain; }
    bool IsSimulationCulled();
    void SetSimulationCullDistance(float dist);

    void Detach();
    void SetFlag(int flag, bool on);
    void GetLookOffsetPos(Vector3 *pos);
    void SetDomain(EDomain *domain);

    char pad0[0x14];
    float m_cullDistSq;             // 0x14
    EEntity *m_parent;              // 0x18
    char pad1c[0x34 - 0x1c];
    EDomain *m_domain;              // 0x34
    char pad38[0x40 - 0x38];
    Vector3 m_lookOffset;           // 0x40
    char pad4c[0x58 - 0x4c];
    int m_interestLevel;            // 0x58
    char pad5c[0x60 - 0x5c];
    float m_maxViewDistance;        // 0x60
};

class ELookTrigger {
public:
    EMatrix *GetLookMatrix() { return m_lookMatrix; }

    bool SetHeadJoint(const char *joint);
    bool IsSkelAnimDone();
    void SetLastSkelAnimTranslateMode(int mode);
    float GetLastSkelAnimLenSecs();
    void ResetAction();
    const char *GetActionState(int index);
    void ResetIdleAnimationTimer();
    bool SetActionTable(const char *name, bool flag, int a);
    void ApplyRelativeVelocity(Vector3 *v);

    char pad0[0x6c];
    EMatrix *m_lookMatrix;          // 0x6c
};

class EPlayerControlVirtuals {
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
    virtual void IdleBeat(int a);    // 0x40
};

void MakeSharedTableWritableImpl(lua_State *L);

static const float kMinPeriod = 0.00001f;

struct EVelocity {
    void operator+=(const Vector3 &v);
};

struct EPhysicsVel {
    char pad0[0x68];
    EVelocity m_velocity;           // 0x68
};

class EMeshAnimManager {
public:
    ETriggerA8 *Find(const char *name);
};

class EScriptControlHandler : public EControlHandler {
public:
    void SetScript(EScriptObject *so);
};

class EActionItems {
public:
    void LockInputTransform(bool on);
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
    virtual EEntity *GetItem();             // 0x30

    char pad4[0x1d4 - 4];
    float m_blink1;                 // 0x1d4
    float m_blink2;                 // 0x1d8
    float m_blink3;                 // 0x1dc
};

struct EFadeInfo {
    char pad0[0x13c];
    float completion;               // 0x13c
};

class EFader {
public:
    void GetFadeInfo(EFadeInfo *info);
};

class EEventFileManager {
public:
    void MakeEventFile(const char *name, const char *src);
};

class EBlendAnims {
public:
    float GetCompletion(const char *name);
};

class EAnimState {
public:
    float GetBlendAnimCompletion(const char *name)
    {
        if (m_blends)
            return m_blends->GetCompletion(name);
        else
            return 1.0f;
    }

    char pad0[0x1d4];
    EBlendAnims *m_blends;          // 0x1d4
};

class EAnimEntity {
public:
    EAnimState *GetAnimState() { return m_anim; }

    char pad0[0xa8];
    EAnimState *m_anim;             // 0xa8
};

class EConsole {
public:
    void Print(const char *msg);
    void __cdecl Printf(int level, const char *fmt, ...);
};

extern EConsole g_console;
int __cdecl FormatString(char *buf, const char *fmt, ...);

class EMenuManager2 {
public:
    void RefreshLevels();
};

// the entities that have a mesh (field 0xa8)
class EMeshEntity {
public:
    ETriggerA8 *GetMesh() { return m_mesh; }
    EPhysics *GetPhysics() { return m_physics; }
    void DisableDeltaTransAnims();
    void UnloadPriorityAnim(int anim, float blend);

    char pad0[0x10];
    EPhysics *m_physics;            // 0x10
    char pad14[0x98 - 0x14];
    float m_viewCone;               // 0x98
    char pad9c[0xa8 - 0x9c];
    ETriggerA8 *m_mesh;             // 0xa8
    char padac[0xd8 - 0xac];
    float m_animMovementScale;      // 0xd8
    char paddc[0x10c - 0xdc];
    bool m_meshBackwards : 1;       // 0x10c
};

struct EPhysicsAnims {
    void DisableAllDeltaTransAnims();
};

void LinkScriptFunctions1();
void LinkScriptFunctions2();
void LinkScriptFunctions3();
void LinkScriptFunctions4();
void LinkScriptFunctions5();

extern float g_hackAnimYaw;

void PhysicsChanged(EPhysics *phys, const char *reason);
void SetPhysicsConstant(int id, float value);
void SetKnapsackConstant(int id, float value);
float GetKnapsackConstant(int id);

class EAnimManager {
public:
    void PreloadAnim(const char *name, float priority, int flags);
};
extern EAnimManager *g_animManager;

struct EBone {
    unsigned short GetId();
};

extern float g_radToDeg;

inline float RadToDeg(float rad) { return rad * g_radToDeg; }
void ScriptWarning(const char *msg);

extern int g_controlsPausedCount;

inline bool ControlsPaused() { return g_controlsPausedCount > 0; }


LUA_FUNCTION_NOSTACK(Yield)
{
    g_game->m_scripts.Yield(L);
    return 0;
}

LUA_FUNCTION(IsDead)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, true, true);
    if (so && !so->IsDead())
        lua_pushint(L, 0);
    else
        lua_pushint(L, 1);
    return 1;
}

LUA_FUNCTION_TODO(PassItemToPsack)

LUA_FUNCTION(NotifyItemRemovedFromInventory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    ((EInventory *)g_game->m_inventory)->ItemRemoved(name);
    return 0;
}

LUA_FUNCTION(GetParentEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    PushEntity(L, ent->GetParent(), false);
    return 1;
}

LUA_FUNCTION_TODO(AttachInventoryEntityToPlayer)

LUA_FUNCTION_TODO(AttachEntityToEntity)

LUA_FUNCTION_TODO(AttachEntityToEntityBone)

LUA_FUNCTION(DetachEntityFromParent)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EEntity *parent = n >= 2 ? GetEntityArg(L, 2, true, 0) : 0;
    if (!parent || parent == ent->GetParent())
        ent->Detach();
    return 0;
}

LUA_FUNCTION_TODO(AttachKnapsackToPlayer)

LUA_FUNCTION(DetachChildEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *parent = GetEntityArg(L, 1, false, 0);
    if (!parent)
        return 0;
    EScriptEntity *child = (EScriptEntity *)GetEntityArg(L, 2, false, 0);
    if (!child)
        return 0;
    if (child->GetParent() != parent)
        return 0;
    child->Detach();
    return 0;
}

LUA_FUNCTION_TODO(GetBoneWorldPosition)

LUA_FUNCTION(GetBoneID)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerA8 *pAnim1 = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pAnim1)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    if (!name)
        return 0;
    EBone *bone = pAnim1->FindBone(name);
    if (bone) {
        unsigned short nId = bone->GetId();
        lua_pushnumber(L, nId);
        return 1;
    } else {
        return 0;
    }
}

LUA_FUNCTION(GetJointID)
{
    return GetBoneID_impl(L);
}

LUA_FUNCTION_NOSTACK_TODO(RotateTowards2D)

LUA_FUNCTION_NOSTACK_TODO(RotateTowards3D)

LUA_FUNCTION_TODO(GetEntityOrientationVector)

LUA_FUNCTION(GetEntityForwardVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 dir = ent->GetPhysics()->m_forward;
    lua_pushnumber(L, dir.x);
    lua_pushnumber(L, dir.y);
    lua_pushnumber(L, dir.z);
    return 3;
}

LUA_FUNCTION(SetEntityGravityDir)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 dirObj(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    dirObj.Normalize();
    ent->GetPhysics()->m_gravityDir = dirObj;
    return 0;
}

LUA_FUNCTION(GetEntityUp)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 dirObj = ent->GetPhysics()->m_up;
    lua_pushnumber(L, dirObj.x);
    lua_pushnumber(L, dirObj.y);
    lua_pushnumber(L, dirObj.z);
    return 3;
}

LUA_FUNCTION(GetEntityGravityDir)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 dirObj = ent->GetPhysics()->m_gravityDir;
    lua_pushnumber(L, dirObj.x);
    lua_pushnumber(L, dirObj.y);
    lua_pushnumber(L, dirObj.z);
    return 3;
}

LUA_FUNCTION(AlignY)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    float fX1 = GetNumberArg(L, 1, false);
    float fZ13 = GetNumberArg(L, 2, false);
    float fX21 = GetNumberArg(L, 3, false);
    float z2 = GetNumberArg(L, 4, false);
    float fAngle2 = atan2f(fX1 - fX21, fZ13 - z2);
    lua_pushnumber(L, RadToDeg(fAngle2));
    return 1;
}

LUA_FUNCTION_TODO(RotateEntityY)

static inline void PushVectorXYZ(lua_State *L, const Vector3 &v)
{
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
}

LUA_FUNCTION(NormalizeVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 v(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    float len = v.Normalize();
    PushVectorXYZ(L, v);
    lua_pushnumber(L, len);
    return 4;
}

LUA_FUNCTION_TODO(EulerToAxisAngle)

LUA_FUNCTION_TODO(AxisAngleToEuler)

LUA_FUNCTION_TODO(VectorToEuler)

LUA_FUNCTION(EulerToVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 euler2(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    Vector3 dir2;
    euler2.EulerToVector(&dir2);
    lua_pushnumber(L, dir2.x);
    lua_pushnumber(L, dir2.y);
    lua_pushnumber(L, dir2.z);
    return 3;
}

LUA_FUNCTION_TODO(CrossProduct)

LUA_FUNCTION_TODO(DotProduct)

LUA_FUNCTION_TODO(GetAngleBetween)

LUA_FUNCTION_TODO(GetAxisAngleBetween)

LUA_FUNCTION_TODO(RotateLike)

LUA_FUNCTION_TODO(MathRotateY)

LUA_FUNCTION_TODO(ApplyOrientation)

LUA_FUNCTION_TODO(InvertOrientation)

LUA_FUNCTION_TODO(RotateVector)

LUA_FUNCTION_TODO(RandomVector)

LUA_FUNCTION_TODO(random)

LUA_FUNCTION(GetCurrentLevelName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    const char *name = g_game->m_levelName;
    lua_pushstring(L, name);
    return 1;
}

LUA_FUNCTION_TODO(Reset)

LUA_FUNCTION(ReturnToStartMenu)
{
    g_game->ReturnToStartMenu();
    return 0;
}

LUA_FUNCTION_TODO(ShootEntityAtTarget)

LUA_FUNCTION_TODO(LaunchEntity)

LUA_FUNCTION_TODO(FindTrajectory)

LUA_FUNCTION(IsSimulationCulled)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushnumber(L, ent->IsSimulationCulled());
    return 1;
}

LUA_FUNCTION(SetSimulationCullDistance)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    float dist = GetNumberArg(L, 2, false);
    ent->SetSimulationCullDistance(dist);
    return 0;
}

LUA_FUNCTION(GetSimulationCullDistance)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    float dist = ent->m_cullDistSq;
    if (dist > 0)
        dist = Sqrt(dist);
    lua_pushnumber(L, dist);
    return 1;
}

LUA_FUNCTION_TODO(SetEntityFlag)

LUA_FUNCTION_TODO(GetEntityFlag)

LUA_FUNCTION(SetMaxViewDistance)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float dist = GetNumberArg(L, 2, false);
    ent->m_maxViewDistance = dist;
    return 0;
}

LUA_FUNCTION(SetEntityInterestLevel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    int nLevelNum = (int)GetNumberArg(L, 2, false);
    ent->m_interestLevel = nLevelNum;
    return 0;
}

LUA_FUNCTION(SetEntityLookOffsetPos)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pt(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ent->m_lookOffset = pt;
    return 0;
}

LUA_FUNCTION(GetEntityLookOffsetPos)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pt;
    ((EEntityVirtuals *)ent)->GetLookOffsetPos(&pt);
    lua_pushnumber(L, pt.x);
    lua_pushnumber(L, pt.y);
    lua_pushnumber(L, pt.z);
    return 3;
}

LUA_FUNCTION(GetEntityLookOffsetPos_Local)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pt;
    ((EEntityVirtuals *)ent)->GetLookOffsetPosLocal(&pt);
    lua_pushnumber(L, pt.x);
    lua_pushnumber(L, pt.y);
    lua_pushnumber(L, pt.z);
    return 3;
}

LUA_FUNCTION(EnableLooking)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    bool bEnable = GetNumberArg(L, 2, false) != 0;
    ((EScriptEntity *)ent)->SetFlag(0x40, bEnable);
    return 0;
}

LUA_FUNCTION_TODO(GetLookTarget)

LUA_FUNCTION(IsRazZLocked)
{
    lua_pushint(L, g_game->GetPlayerControl()->IsZLocked() != 0);
    return 1;
}

LUA_FUNCTION(BreakZLock)
{
    g_game->GetPlayerControl()->SetZLock(false);
    return 0;
}

LUA_FUNCTION(LookAtEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    ETriggerBase *target = GetTriggerArg(L, 2, true);
    ((EEntityVirtuals *)ent)->LookAt(target, 1, 1);
    return 0;
}

LUA_FUNCTION(SetHeadJoint)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *target = GetTriggerArg(L, 1, false);
    if (!target)
        return 0;
    const char *joint = GetStringArg(L, 2, false);
    bool bOk3 = ((ELookTrigger *)target)->SetHeadJoint(joint);
    return 0;
}

LUA_FUNCTION_TODO(SetMeshLookAtParams)

LUA_FUNCTION(DumpMeshLookAtParams)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ELookTrigger *ent = (ELookTrigger *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    if (ent->m_lookMatrix) {
        EQuat rot(*ent->GetLookMatrix());
        if (rot.w < 0)
            rot = rot * -1.0f;
    }
    return 0;
}

LUA_FUNCTION_TODO(SetHeadConstraint)

LUA_FUNCTION_TODO(GetHeadForwardVector)

LUA_FUNCTION_TODO(SetPlayerLookBox)

LUA_FUNCTION_TODO(CreateNamedTimer)

LUA_FUNCTION_TODO(GetTimerSecs)

LUA_FUNCTION(KillAllTimers)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    g_game->m_scripts.KillAllTimers(so);
    return 0;
}

LUA_FUNCTION_TODO(KillTimer)

LUA_FUNCTION(HardBreak)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *msg = GetStringArg(L, 1, false);
    g_game->m_scripts.Break(msg, 0);
    return 0;
}

LUA_FUNCTION(HasEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    if (so->m_entity)
        lua_pushnumber(L, 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(SpawnScript)

LUA_FUNCTION_TODO(SendMessage)

LUA_FUNCTION_TODO(SendMessageEx)

LUA_FUNCTION_TODO(SendMessageToDomain)

LUA_FUNCTION(CanHandleMessage)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    const char *szMsg = GetStringArg(L, 2, false);
    if (so->CanHandleMessage(szMsg))
        lua_pushnumber(L, 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(FindScriptObject)

LUA_FUNCTION(PreloadLuaClass)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    g_game->m_scripts.PreloadClass(name);
    return 0;
}

LUA_FUNCTION_NOSTACK(DoFile)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool bOnce1 = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    g_game->m_scripts.DoFile(name, bOnce1);
    return 0;
}

LUA_FUNCTION_TODO(GetBoundingBox)

LUA_FUNCTION_TODO(SetMeshProperty)

LUA_FUNCTION_TODO(SetMeshMaterialProperty)

LUA_FUNCTION(SetAnimSpeedScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerA8 *anim = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!anim)
        return 0;
    float fScale = GetNumberArg(L, 2, false);
    anim->m_animSpeedScale = fScale;
    return 0;
}

LUA_FUNCTION(PointIsContainedBy)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    Vector3 pt(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    ETriggerA8 *pAnimPtr = (ETriggerA8 *)GetTriggerA8Arg(L, 4, false);
    if (!pAnimPtr)
        return 0;
    bool inside = pAnimPtr->Contains(&pt, 0);
    lua_pushnumber(L, inside);
    return 1;
}

LUA_FUNCTION(SetEntityAnimSpeedScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerA8 *anim = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!anim)
        return 0;
    float fScale = GetNumberArg(L, 2, false);
    anim->m_animSpeedScale = fScale;
    return 0;
}

LUA_FUNCTION(GetEntityAnimSpeedScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *anim = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!anim)
        return 0;
    lua_pushnumber(L, anim->GetAnimSpeedScale());
    return 1;
}

LUA_FUNCTION(SetEntityDomain)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EScriptEntity *e = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!e)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    if (!name) {
        LuaCallSite(L);
        return 0;
    }
    EDomain *pDomain2 = g_game->GetLevelManager()->GetDomainManager()->FindDomain(name);
    if (!pDomain2) {
        LuaCallSite(L);
        return 0;
    }
    e->SetDomain(pDomain2);
    return 0;
}

LUA_FUNCTION(GetEntityDomain)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptEntity *ent = (EScriptEntity *)GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushstring(L, ent->GetDomain()->GetName());
    return 1;
}

LUA_FUNCTION_TODO(FindNearestEntity)

LUA_FUNCTION(ControlsArePaused)
{
    lua_pushint(L, ControlsPaused() != 0);
    return 1;
}

LUA_FUNCTION_TODO(PausePlayerControls)

LUA_FUNCTION_TODO(APIDontRunPlayerControls)

LUA_FUNCTION_TODO(GetPlayerControlCameraRelativeVector)

LUA_FUNCTION(RegisterCutscene)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    g_game->m_scripts.RegisterCutscene(so);
    return 0;
}

LUA_FUNCTION(UnregisterCutscene)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    g_game->m_scripts.UnregisterCutscene(so);
    return 0;
}

LUA_FUNCTION_NOSTACK_TODO(ForEachEntityInRadius)

LUA_FUNCTION_NOSTACK_TODO(foreach_entity)

LUA_FUNCTION_NOSTACK_TODO(foreach_lso)

LUA_FUNCTION_NOSTACK_TODO(foreach_entity_unsafe)

LUA_FUNCTION(MakeSharedTableWritable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    if (lua_type(L, 1) != LUA_TTABLE) {
        ScriptWarning("MakeSharedTableWritable Error: first arg is not a table");
        return 0;
    }
    MakeSharedTableWritableImpl(L);
    return 0;
}

LUA_FUNCTION(SetSkyBox)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    int nIndex = n >= 2 ? (int)GetNumberArg(L, 2, true) : 0;
    if (!name)
        return 0;
    if (!g_game->SetSkyBox(name, nIndex))
        return 0;
    return 0;
}

LUA_FUNCTION(SetActiveSkyBox)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    g_game->SetActiveSkyBox(index);
    return 0;
}

LUA_FUNCTION_TODO(LoadNewLevel)

LUA_FUNCTION(DomainExists)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *str = GetStringArg(L, 1, false);
    if (!str) {
        lua_pushnumber(L, 0);
        return 1;
    }
    EDomain *domain = ((EDomainManager *)g_game->m_unk81b4)->FindDomain(str);
    int exists = 0;
    if (domain)
        exists = 1;
    lua_pushint(L, exists);
    return 1;
}

LUA_FUNCTION(DomainSleep)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *szName1 = GetStringArg(L, 1, false);
    int nSleep = (int)GetNumberArg(L, 2, false);
    if (szName1) {
        EDomain *domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain(szName1);
        if (!domain)
            return 0;
        domain->SetSleeping(nSleep != 0);
        g_game->UpdateDomainSleep();
    }
    return 0;
}

LUA_FUNCTION(IsEntitySleeping)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    lua_pushint(L, ent->IsSleeping() != 0);
    return 1;
}

LUA_FUNCTION(IsDomainSleeping)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *szName1 = GetStringArg(L, 1, false);
    int sleeping = 1;
    if (szName1) {
        EDomain *domain = g_game->GetLevelManager()->GetDomainManager()->FindDomain(szName1);
        if (!domain)
            DebugPrintf("no domain");
        else if (!domain->IsSleeping())
            sleeping = 0;
    }
    lua_pushint(L, sleeping);
    return 1;
}

LUA_FUNCTION_TODO(CreateChildDomain)

LUA_FUNCTION(GetFrameCount)
{
    lua_pushuint(L, g_game->m_frameCount);
    return 1;
}

LUA_FUNCTION(GetFrame60ths)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int nRealValue = n >= 1 ? (int)GetNumberArg(L, 1, true) : 0;
    float frames;
    if (nRealValue)
        frames = g_game->GetRealFrameSecs() * 60.0f;
    else
        frames = g_game->GetFrameSecs() * 60.0f;
    lua_pushnumber(L, frames);
    return 1;
}

LUA_FUNCTION(GetFrameSecs)
{
    lua_pushnumber(L, g_game->GetFrameSecs());
    return 1;
}

LUA_FUNCTION(GetGameTimeSecs)
{
    float secs = (float)g_game->GetTime();
    lua_pushnumber(L, secs);
    return 1;
}

LUA_FUNCTION(GetPeriodicGameTimeSecs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fPeriod = GetNumberArg(L, 1, false);
    if (fPeriod > kMinPeriod) {
        float t = (float)g_game->GetTime();
        t = ((int)(t * 1000.0f) % (int)(fPeriod * 1000.0f)) / 1000.0f;
        lua_pushnumber(L, t);
    } else {
        lua_pushnumber(L, 0);
    }
    return 1;
}

LUA_FUNCTION(GetLocalTime)
{
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    lua_pushnumber(L, st.wHour);
    lua_pushnumber(L, st.wMinute);
    return 2;
}

LUA_FUNCTION_TODO(XSayLine)

LUA_FUNCTION_TODO(ResetMeshSkelPos)

LUA_FUNCTION_TODO(SetSkeletonScale)

LUA_FUNCTION_TODO(GetSkeletonScale)

LUA_FUNCTION(IsSkelAnimDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    lua_pushint(L, ((ELookTrigger *)ent)->IsSkelAnimDone() != 0);
    return 1;
}

LUA_FUNCTION(SetLastSkelAnimTranslateMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *e = GetTriggerArg(L, 1, false);
    if (!e)
        return 0;
    int nMode2 = (int)GetNumberArg(L, 2, false);
    ((ELookTrigger *)e)->SetLastSkelAnimTranslateMode(nMode2);
    return 0;
}

LUA_FUNCTION(GetLastSkelAnimLenSecs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    lua_pushnumber(L, ((ELookTrigger *)ent)->GetLastSkelAnimLenSecs());
    return 1;
}

LUA_FUNCTION_TODO(StartMeshAnim)

LUA_FUNCTION(StopMeshAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *str = GetStringArg(L, 1, false);
    bool now = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    ETriggerA8 *pAnim1 = g_game->GetLevelManager()->GetLevel()->GetMeshAnims()->Find(str);
    if (!pAnim1)
        return 0;
    pAnim1->StopMeshAnim(now);
    return 0;
}

LUA_FUNCTION(IsMeshAnimDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    ETriggerA8 *anim = g_game->GetLevelManager()->GetLevel()->GetMeshAnims()->Find(name);
    if (!anim)
        return 0;
    lua_pushint(L, !anim->m_meshAnimPlaying);
    return 1;
}

LUA_FUNCTION(StartEntityMeshAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerA8 *anim = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!anim)
        return 0;
    int i = (int)GetNumberArg(L, 2, false);
    bool bLoop = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    anim->StartMeshAnim(i, bLoop);
    return 0;
}

LUA_FUNCTION(StopEntityMeshAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    ETriggerA8 *pAnim1 = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pAnim1)
        return 0;
    bool now = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    pAnim1->StopMeshAnim(now);
    return 0;
}

LUA_FUNCTION(IsEntityMeshAnimDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    ETriggerA8 *anim = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!anim)
        return 0;
    lua_pushint(L, !anim->m_meshAnimPlaying);
    return 1;
}

LUA_FUNCTION(GetJointWorldPos)
{
    return GetBoneWorldPosition_impl(L);
    return 3;
}

LUA_FUNCTION(RoundToInt)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float f = lua_tonumber(L, 1);
    int i = (int)(f + 0.5);
    lua_pushint(L, i);
    return 1;
}

LUA_FUNCTION(SetActionTable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ELookTrigger *ent = (ELookTrigger *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    const char *name = GetStringArg(L, 2, false);
    bool bFlag = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    if (!name)
        return 0;
    if (!ent->SetActionTable(name, bFlag, 1))
        LuaCallSite(L);
    return 0;
}

LUA_FUNCTION(ResetAction)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    ((ELookTrigger *)ent)->ResetAction();
    return 0;
}

LUA_FUNCTION_TODO(SetNewAction)

LUA_FUNCTION(GetActionState)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *entity = GetTriggerArg(L, 1, false);
    if (!entity)
        return 0;
    const char *szState = ((ELookTrigger *)entity)->GetActionState(-1);
    lua_pushstring(L, szState);
    return 1;
}

LUA_FUNCTION(ResetIdleAnimationTimer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    ((ELookTrigger *)ent)->ResetIdleAnimationTimer();
    return 1;
}

LUA_FUNCTION_TODO(AddExtraRazIdle)

LUA_FUNCTION(ClearExtraRazIdles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->GetPlayerControl()->ClearExtraIdles();
    return 0;
}

LUA_FUNCTION(TEST_RazIdleBeat)
{
    ((EPlayerControlVirtuals *)g_game->GetPlayerControl())->IdleBeat(0);
    return 0;
}

LUA_FUNCTION(ApplyVelocity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 vel(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ((EPhysicsVel *)ent->GetPhysics())->m_velocity += vel;
    return 0;
}

LUA_FUNCTION(ApplyRelativeVelocity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ELookTrigger *ent = (ELookTrigger *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 vel(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ent->ApplyRelativeVelocity(&vel);
    return 0;
}

LUA_FUNCTION(SetScriptControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *obj = GetScriptObjectArg(L, 1, true, false);
    EScriptControlHandler *pHandler = (EScriptControlHandler *)g_game->GetControlHandler(11);
    pHandler->SetScript(obj);
    g_game->SetControlHandler(11, 0);
    return 0;
}

LUA_FUNCTION_TODO(SetUIControlHandler)

LUA_FUNCTION_TODO(SetControlHandler)

LUA_FUNCTION(GetControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int nId = g_game->GetCurrentControlHandlerId();
    lua_pushint(L, nId);
    return 1;
}

LUA_FUNCTION(GetPlayerActionItem)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EActionItems *items = g_game->m_actionItems;
    PushEntity(L, items->GetItem(), false);
    return 1;
}

LUA_FUNCTION(APISetState)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    so->SetState(L);
    return 0;
}

LUA_FUNCTION(ShowAllCommands)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int num = 0;
    ELuaFunction *f = ELuaFunction::s_first;
    while (f)
        f = f->m_next;
    return 0;
}

LUA_FUNCTION_TODO(DaveCo)

LUA_FUNCTION(PurgeBlends)
{
    return 0;
}

LUA_FUNCTION(DebugLookAt)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int top = lua_gettop(L);
    int on = (int)GetNumberArg(L, 1, false);
    g_game->m_debugLookAt = on != 0;
    return 0;
}

LUA_FUNCTION(LevelFilter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int top = lua_gettop(L);
    char all = '*';
    const char *szFilter3;
    if (top == 1)
        szFilter3 = lua_tostring(L, 1);
    else
        szFilter3 = &all;
    FormatString(g_game->m_levelFilter, "%s.plb", szFilter3);
    if (g_game->m_menus)
        ((EMenuManager2 *)g_game->m_menus)->RefreshLevels();
    return 0;
}

LUA_FUNCTION(RazBlink)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float fAngle = GetNumberArg(L, 1, false);
    float b = GetNumberArg(L, 2, false);
    float cnt = GetNumberArg(L, 3, false);
    if (g_game->m_actionItems) {
        EActionItems *items = g_game->m_actionItems;
        items->m_blink1 = fAngle;
        items->m_blink2 = b;
        items->m_blink3 = cnt;
    }
    return 0;
}

LUA_FUNCTION_TODO(CreateProceduralTexture)

LUA_FUNCTION_TODO(FadeGetCurrentState)

LUA_FUNCTION(GetFadeCompletion)
{
    EFadeInfo info;
    g_game->m_fader->GetFadeInfo(&info);
    lua_pushnumber(L, info.completion);
    return 1;
}

LUA_FUNCTION_TODO(FadeToColor)

LUA_FUNCTION_TODO(MakeDialogFile)

LUA_FUNCTION(MakeEventFile)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int angle = 0;
    int nB3 = 0;
    const char *name = GetStringArg(L, 1, false);
    const char *src = GetStringArg(L, 2, false);
    g_game->m_events->MakeEventFile(name, src);
    lua_pushnumber(L, 1);
    return 1;
}

LUA_FUNCTION_TODO(LoadBlendAnim)

LUA_FUNCTION_TODO(StopAllBlendAnims)

LUA_FUNCTION_TODO(StopBlendAnim)

LUA_FUNCTION(GetBlendAnimCompletion)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETriggerBase *time = GetTriggerArg(L, 1, false);
    if (!time)
        return 0;
    const char *szName1 = GetStringArg(L, 2, false);
    EAnimEntity *ent = (EAnimEntity *)time;
    float fCompletion = ent->GetAnimState()->GetBlendAnimCompletion(szName1);
    lua_pushnumber(L, fCompletion);
    return 1;
}

LUA_FUNCTION(GamePrint)
{
    int n = lua_gettop(L);
    const char *msg = lua_tostring(L, -1);
    int nLevelObj = n == 2 ? (unsigned int)lua_tonumber(L, -2) : 2;
    if (nLevelObj == 0) {
        g_console.Print(msg);
    } else {
        const char *caller = GetScriptManager()->GetCallerName(L);
        g_console.Printf(nLevelObj, "%s %s", caller, msg);
    }
    return 0;
}

LUA_FUNCTION_TODO(DebugPrint)

LUA_FUNCTION_TODO(DebugPrint2)

LUA_FUNCTION(KillScript)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    if (!so->IsDead())
        g_game->m_scripts.KillScript(so);
    return 0;
}

LUA_FUNCTION(GetDistance)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    Vector3 v1(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    Vector3 v2(GetNumberArg(L, 4, false), GetNumberArg(L, 5, false), GetNumberArg(L, 6, false));
    lua_pushnumber(L, (v2 - v1).Length());
    return 1;
}

LUA_FUNCTION_TODO(GetPlayerDist)

LUA_FUNCTION_TODO(LoadNewPlayerMesh)

LUA_FUNCTION(GetPlayerLSO)
{
    EScriptObject *so = g_game->GetPlayerControl()->GetScriptObject();
    lua_getref(L, so->m_ref);
    return 1;
}

LUA_FUNCTION(GetAbsPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pos = ent->GetPhysics()->GetPosition();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION(GetRelPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pos = ent->GetPhysics()->GetLocalMatrix().GetTranslation();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION_TODO(SetAbsPosition)

LUA_FUNCTION(SetRelPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 5))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pt(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    EPhysics *phys = ent->GetPhysics();
    phys->SetPosition(pt, true);
    phys->m_a0 = 0;
    PhysicsChanged(phys, "Lua setpos");
    return 0;
}

LUA_FUNCTION_TODO(GetAbsOrientation)

LUA_FUNCTION_TODO(GetRelOrientation)

LUA_FUNCTION_TODO(SetAbsOrientation)

LUA_FUNCTION_TODO(SetRelOrientation)

LUA_FUNCTION_TODO(GetOrientation)

LUA_FUNCTION(SetOrientation)
{
    return SetRelOrientation_impl(L);
}

LUA_FUNCTION(GetPosition)
{
    return GetAbsPosition_impl(L);
}

LUA_FUNCTION_TODO(SetPosition)

LUA_FUNCTION(GetPlayerPosition)
{
    Vector3 pos = g_game->GetPlayerControl()->GetPhysics()->GetPosition();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION(InitPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pos(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ent->GetPhysics()->SetPosition(pos, true);
    return 0;
}

LUA_FUNCTION_TODO(MoveToPosition)

LUA_FUNCTION(GetVelocity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pos = ent->GetPhysics()->GetVelocity();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION(GetTotalVelocity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 vel = ent->GetPhysics()->GetTotalVelocity();
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    lua_pushnumber(L, vel.z);
    return 3;
}

LUA_FUNCTION(SetVelocity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 vel(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    ent->GetPhysics()->GetVelocity() = vel;
    return 0;
}

LUA_FUNCTION(GetAnimVelocity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pos = ent->GetPhysics()->GetAnimVelocity();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

LUA_FUNCTION(GetScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 size = ent->GetMesh()->GetScale();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    lua_pushnumber(L, size.z);
    return 3;
}

LUA_FUNCTION_TODO(SetScale)

LUA_FUNCTION(PlayerLockInputTransform)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool locked = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_game->GetActionItems()->LockInputTransform(false);
    return 0;
}

LUA_FUNCTION_TODO(PreloadMesh)

LUA_FUNCTION_TODO(LoadMesh)

LUA_FUNCTION_TODO(LoadWorldMesh)

LUA_FUNCTION_TODO(SetWorldMeshActive)

LUA_FUNCTION_TODO(DebugDumpDomainEntities)

LUA_FUNCTION(LockMeshEntityLODLevel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    unsigned int nLevel = (unsigned int)GetNumberArg(L, 2, false);
    ent->m_mesh->LockLODLevel(nLevel);
    return 0;
}

LUA_FUNCTION_TODO(LoadAnim)

LUA_FUNCTION_TODO(PreloadAnim)

LUA_FUNCTION(PreloadAnimData)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *szAnim = GetStringArg(L, 1, false);
    float priority = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    if (priority < 0.0)
        LuaCallSite(L);
    g_animManager->PreloadAnim(szAnim, priority, 0);
    return 0;
}

LUA_FUNCTION_TODO(LoadPriorityAnim)

LUA_FUNCTION(UnLoadPriorityAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    int animIndex = (int)GetNumberArg(L, 2, false);
    float blendSecs = n >= 3 ? GetNumberArg(L, 3, true) : 0.5f;
    ent->UnloadPriorityAnim(animIndex, blendSecs);
    return 0;
}

LUA_FUNCTION(DisableAllDeltaTransAnims)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    ent->DisableDeltaTransAnims();
    ((EPhysicsAnims *)ent->GetPhysics())->DisableAllDeltaTransAnims();
    return 0;
}

LUA_FUNCTION(EntityHasSkeleton)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, true);
    if (pMesh && pMesh->m_joints.m_count > 0)
        lua_pushnumber(L, 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(IsPlayingAnim)

LUA_FUNCTION_TODO(GetAnimCompletion)

LUA_FUNCTION_TODO(SetAnimCompletion)

LUA_FUNCTION_TODO(GetCurrentAnimName)

LUA_FUNCTION(SetMeshIsBackwards)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    bool flag = GetNumberArg(L, 2, false) != 0;
    ent->m_meshBackwards = flag;
    return 0;
}

LUA_FUNCTION(GetMeshIsBackwards)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    lua_pushnumber(L, ent->m_meshBackwards);
    return 1;
}

LUA_FUNCTION_TODO(SetPhysics)

LUA_FUNCTION_TODO(CreateObject)

LUA_FUNCTION(GetBaseClass)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *cname = GetStringArg(L, 1, false);
    int ref = GetScriptManager()->GetSharedBase(cname, "GetBaseClass", 1);
    if (!ref)
        lua_pushnil(L);
    else
        lua_getref(L, ref);
    return 1;
}

LUA_FUNCTION_TODO(GetDirectoryListing)

LUA_FUNCTION_NOSTACK_TODO(DoFileDirectory)

LUA_FUNCTION_TODO(GetBillboard)

LUA_FUNCTION(SetPhysicsConstant)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    int index = (int)GetNumberArg(L, 2, false);
    float val = GetNumberArg(L, 3, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (index < 0 || index >= 0x54) {
        LuaCallSite(L);
        return 0;
    }
    SetPhysicsConstant(index, val);
    return 0;
}

LUA_FUNCTION(SetKnapsackConstant)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    float val = GetNumberArg(L, 2, false);
    if (index < 0 || index >= 6)
        return 0;
    SetKnapsackConstant(index, val);
    return 0;
}

LUA_FUNCTION(GetKnapsackConstant)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    if (index < 0 || index >= 6)
        return 0;
    float val = GetKnapsackConstant(index);
    lua_pushnumber(L, val);
    return 1;
}

LUA_FUNCTION(SetViewCone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float fov = GetNumberArg(L, 2, false);
    ent->m_viewCone = fov;
    return 0;
}

LUA_FUNCTION_TODO(AttachDecal)

LUA_FUNCTION_TODO(RemoveDecal)

LUA_FUNCTION_TODO(RemoveAllDecals)

LUA_FUNCTION_TODO(GetEntityFeelCenter)

LUA_FUNCTION_TODO(GetEntityFeelExtent)

LUA_FUNCTION_TODO(GetEntityLookCenter)

LUA_FUNCTION(SetEntityAnimationMovementScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float speed = GetNumberArg(L, 2, false);
    ent->m_animMovementScale = speed;
    return 0;
}

LUA_FUNCTION(DoNothing)
{
    return 0;
}

LUA_FUNCTION_TODO(Levitate)

LUA_FUNCTION(LinkCheckOnly)
{
    LinkScriptFunctions1();
    LinkScriptFunctions2();
    LinkScriptFunctions3();
    LinkScriptFunctions4();
    LinkScriptFunctions5();
    return 0;
}

LUA_FUNCTION_TODO(CalculateRightVector)

LUA_FUNCTION_TODO(EvaluateCurve)

LUA_FUNCTION(PurgeBlendAnims)
{
    return 0;
}

LUA_FUNCTION_TODO(NudgeTowardPosition)

LUA_FUNCTION_TODO(SetScriptValueTest)

LUA_FUNCTION_TODO(CopyStaticPose)

LUA_FUNCTION(SetEditableLightMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int lightMode = (int)GetNumberArg(L, 1, false);
    g_game->m_editableLightMode = lightMode == 0;
    return 0;
}

LUA_FUNCTION_TODO(EnterClairvoyanceCloud)

LUA_FUNCTION(ExitClairvoyanceCloud)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->ExitClairvoyanceCloud();
    return 0;
}

LUA_FUNCTION(SetPlayerIsConfused)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool confused = GetNumberArg(L, 1, false) != 0;
    g_game->GetPlayerControl()->m_confused = confused;
    return 0;
}

LUA_FUNCTION_TODO(ForceRazJump)

LUA_FUNCTION(IsRunningAtDF)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int result = !g_game->m_runningAtDF;
    lua_pushint(L, result);
    return 1;
}

LUA_FUNCTION(IsOXMBuild)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int result = g_game->m_oxmBuild != 0;
    lua_pushint(L, result);
    return 1;
}

LUA_FUNCTION_TODO(NativeMulQuatQuat)

LUA_FUNCTION_TODO(NativeMulQuatVec)

LUA_FUNCTION_TODO(NativeQuatToEuler)

LUA_FUNCTION_TODO(NativeEulerToQuat)

LUA_FUNCTION_TODO(NativeQuatFromAxisAngle)

LUA_FUNCTION_TODO(NativeQuatFromVectorPair)

LUA_FUNCTION_TODO(NativeQuatSlerp)

LUA_FUNCTION(BitAnd)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int lhs = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int rhs = (unsigned int)GetNumberArg(L, 2, false);
    lua_pushuint(L, lhs & rhs);
    return 1;
}

LUA_FUNCTION(BitOr)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int lhs = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int rhs = (unsigned int)GetNumberArg(L, 2, false);
    lua_pushuint(L, lhs | rhs);
    return 1;
}

LUA_FUNCTION(BitXor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int lhs = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int rhs = (unsigned int)GetNumberArg(L, 2, false);
    lua_pushuint(L, lhs ^ rhs);
    return 1;
}

LUA_FUNCTION(BitNot)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int bits = (unsigned int)GetNumberArg(L, 1, false);
    lua_pushuint(L, ~bits & 0x7fffff);
    return 1;
}

LUA_FUNCTION(BitLsh)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int bits = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int shift = (unsigned int)GetNumberArg(L, 2, false);
    lua_pushuint(L, bits << shift);
    return 1;
}

LUA_FUNCTION(BitRsh)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int bits = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int shift = (unsigned int)GetNumberArg(L, 2, false);
    lua_pushuint(L, bits >> shift);
    return 1;
}

LUA_FUNCTION_TODO(BitStore)

LUA_FUNCTION_TODO(BitFetch)

LUA_FUNCTION_TODO(ListDomains)

LUA_FUNCTION_TODO(GetRootJointName)

LUA_FUNCTION(GameLog)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 5))
        return 0;
    return 0;
}

LUA_FUNCTION(SetHackAnimYaw)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float yaw = GetNumberArg(L, 1, false);
    g_hackAnimYaw = yaw;
    return 0;
}

LUA_FUNCTION_TODO(Get2DCoordsFromPos)

LUA_FUNCTION(PauseScript)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    int pause = (int)GetNumberArg(L, 2, false);
    so->m_paused = pause != 0;
    return 0;
}

LUA_FUNCTION(FreezeManager_SetInstaThaw)
{
    g_game->m_freeze->SetInstaThaw();
    return 0;
}

LUA_FUNCTION(IDsAreEqual)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    void *lhs = lua_torawvalue(L, 1);
    void *rhs = lua_torawvalue(L, 2);
    lua_pushnumber(L, lhs == rhs);
    return 1;
}
