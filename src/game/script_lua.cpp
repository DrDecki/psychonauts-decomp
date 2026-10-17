// Script functions, file name made up.

#include <math.h>
#include <stdio.h>
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
    virtual bool HasMesh();         // 0x10
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
    virtual EAABB GetLookBox();     // 0x74
    virtual void Vf78();
    virtual void Vf7c();
    virtual void LookAt(EEntity *target, int a, int b);     // 0x80
};

class EInventory {
public:
    void ItemRemoved(const char *name);
    void AddItem(const char *name, bool flag);
};

struct EModelBounds {
    char pad0[0x20];
    EAABB m_box;                    // 0x20
};

struct EModel {
    const char *GetName() { return m_name; }

    const char *m_name;             // 0x00
    char pad4[0x80 - 4];
    EModelBounds *m_bounds;         // 0x80
};

struct EBoneInfo {
    const char *GetName() { return m_name; }

    char pad0[0x40];
    Vector3 m_restPos;              // 0x40
    char pad4c[0x5c - 0x4c];
    const char *m_name;             // 0x5c
};

struct ESkeleton {
    char pad0[4];
    EBoneInfo *m_bones;             // 0x04
};

struct ESkelBone {
    EBoneInfo *GetInfo() { return &m_skeleton->m_bones[m_index]; }

    char pad0[0x9c];
    Vector3 m_pos;                  // 0x9c
    ESkeleton *m_skeleton;          // 0xa8
    unsigned char m_index;          // 0xac
};

struct EJoint {
    void Update();

    char pad0[8];
    ESkelBone *m_bone;              // 0x08
    char padc[0x14 - 0xc];
    Vector3 m_scale;                // 0x14
};

inline const char *GetBoneName(ESkelBone *bone) { return bone->GetInfo()->GetName(); }

struct EBlendStopFlags {
    bool m_now;
    char pad1[3];
};

// the entity at GameApp 0x8190, only its script object is used here
struct EScriptHolder {
    EScriptObject *GetScriptObject() { return m_scriptObject; }

    char pad0[0xc];
    EScriptObject *m_scriptObject;  // 0x0c
};

class EMeshManager {
public:
    void PreloadMesh(const char *name, int mode);
};
extern EMeshManager *g_meshManager;
void CreateMeshManager();

// a Vector3 whose default constructor clears it (back to front)
struct EZeroVector3 : public Vector3 {
    EZeroVector3() { z = 0; y = 0; x = 0; }
};

struct ECamera {
    Vector3 WorldToScreen(const Vector3 &pos, int flags);
    EMatrix &GetViewMatrix() { return m_view; }

    char pad0[0x190];
    EMatrix m_view;                 // 0x190
};

struct ERenderer {
    void *CreateTexture(int a, unsigned int width, unsigned int height, int levels, int format, int flags);
};

class EDecalManager {
public:
    void RemoveDecal(void *id);
    void RemoveDecalNow(void *id);
    void RemoveAll(class ETriggerA8 *mesh, void *texture);
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
    void SetScale(const Vector3 &scale);
    void StopBlendAnim(const char *name, float blend);
    void StopBlendAnimNow(const char *name);
    void CopyStaticPose(ETriggerA8 *from, bool flag);
    EAABB GetBounds();
    struct EModel *GetModel() { return m_model; }
    void SetProperty(unsigned int prop, bool on, void *texture);
    void SetMaterialProperty(unsigned int prop, bool on, void *texture);

    char pad0[0x18 - 0x0];
    struct EModel *m_model;             // 0x18
    EAABB m_box;                        // 0x1c
    char pad34[0x54 - 0x34];
    TArray<struct EJoint *> m_joints;   // 0x54
    char pad60[0x94 - 0x60];
    bool m_94_0 : 1;                // 0x94
    bool m_meshAnimPlaying : 1;
    char pad95[0xb0 - 0x95];
    EMatrix m_animMatrix;               // 0xb0
    char padf0[0x1b0 - 0xf0];
    Vector3 m_scale;                // 0x1b0
    char pad1bc[0x1d8 - 0x1bc];
    float m_animSpeedScale;         // 0x1d8
};

struct EListNode {
    EListNode *m_next;              // 0x00
    EListNode *m_prev;              // 0x04
    void *m_data;                   // 0x08
};

inline void *ListNext(EListNode *&node)
{
    EListNode *cur = node;
    node = node->m_next;
    return cur->m_data;
}

struct EDomain {
    void SetSleeping(bool on);
    EListNode *GetChildren() { return m_children; }
    const char *GetName() { return m_name; }
    bool IsSleeping() { return m_sleeping; }
    EDomain *FindDomain(const char *name);
    void SendMessage(EScriptObject *from, const char *msg, const char *data, int delay, bool recurse);
    void CreateChild(const char *name, int flags);

    char pad0[0x14];
    EListNode *m_children;          // 0x14
    char pad18[0x44 - 0x18];
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

struct ELookAtInfo {
    EScriptObject *GetTarget() { return m_target; }

    char pad0[0xc];
    EScriptObject *m_target;        // 0x0c
};

// a file on disk, only used here to see if one exists
class EFile {
public:
    EFile();
    ~EFile();
    bool Open(const char *path, int mode);
    void Close();

    char pad0[8];
};

#pragma pack(push, 4)
struct ETimer {
    char pad0[0xc];
    double m_endTime;               // 0x0c
};
#pragma pack(pop)

double GetScriptTime();

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
    bool SetNewAction(const char *action, int flags);
    bool SetNewActionState(const char *action, int a, int b, const char *state);

    char pad0[0x6c];
    EMatrix *m_lookMatrix;          // 0x6c
    EQuat m_lookRot;                // 0x70
    struct ELookAtInfo *m_lookAt;   // 0x80
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
    float GetCameraRelativeVector(const float *input, Vector3 *out, int a, int b);
    void PassItem(const char *name);
    void SetLadderEntity(EEntity *ent);
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void Vf14();
    virtual void Vf18();
    virtual int GetMode();          // 0x1c
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
    Color color;                    // 0x00
    char name[300];                 // 0x10
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
    void UpdateCollisionMesh();
    void FadeIn(float secs);

    char pad0[0x10];
    EPhysics *m_physics;            // 0x10
    char pad14[0x84 - 0x14];
    char m_headConstraint[3];           // 0x84
    char pad87[0x98 - 0x87];
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
bool SetPlayerControlsPaused(bool pause, bool flag);

// the action the player control switches to for a forced jump
extern const char *g_jumpActionName;
extern const char *g_dontRunActionName;
void ResetPlayerInput(int flags);
bool SetPlayerDontRun(bool on);

class EPlayerControlActions {
public:
    void SetAction(const char *name, int a, bool b, int c);
};

class ENormalControlHandler : public EControlHandler {
public:
    void ForceJump(bool on, const char *target);
};
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
void ScriptWarning(const char *fmt, ...);
void ScriptError(const char *fmt, ...);

// compiled out in the release build, the argument is still evaluated
inline void DebugName(const char *name) {}

extern int g_controlsPausedCount;

inline bool ControlsPaused() { return g_controlsPausedCount > 0; }


LUA_FUNCTION_NOSTACK(Yield)
{
    g_game->m_scripts.Yield(L);
    return 0;
}

static inline void PushVectorXYZ(lua_State *L, const Vector3 &v)
{
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
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

LUA_FUNCTION(PassItemToPsack)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *item = GetStringArg(L, 1, false);
    bool bFlag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (g_game->m_actionItems && g_game->m_actionItems->GetMode() == 2)
        g_game->GetActionItems()->PassItem(item);
    if (1)
        g_game->GetInventory()->AddItem(item, bFlag);
    return 0;
}

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

LUA_FUNCTION(AttachEntityToEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EEntity *pParent = GetEntityArg(L, 2, false, 0);
    if (!pParent)
        return 0;
    bool keep = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    if (keep)
        ent->AttachTo(pParent, 0, 0);
    else
        ent->AttachTo(pParent, &g_identityMatrix, 0);
    return 0;
}

LUA_FUNCTION(AttachEntityToEntityBone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EEntity *pParent = GetTriggerArg(L, 2, false);
    if (!pParent)
        return 0;
    const char *szBone = GetStringArg(L, 3, false);
    bool keep = n >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    ent->AttachTo(pParent, keep ? 0 : &g_identityMatrix, szBone);
    return 0;
}

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

LUA_FUNCTION(GetEntityOrientationVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 dir(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    Vector3 newEuler;
    newEuler.Set(ent->GetMesh()->m_animMatrix.ToEuler());
    Vector3 result = dir.Rotate(newEuler);
    result.Normalize();
    lua_pushnumber(L, result.x);
    lua_pushnumber(L, result.y);
    lua_pushnumber(L, result.z);
    return 3;
}

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

LUA_FUNCTION(EulerToAxisAngle)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 euler(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    EAxisAngle out = euler.ToAxisAngle();
    lua_pushnumber(L, out.axis.x);
    lua_pushnumber(L, out.axis.y);
    lua_pushnumber(L, out.axis.z);
    lua_pushnumber(L, out.angle);
    return 4;
}

LUA_FUNCTION(AxisAngleToEuler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    Vector3 axis(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    float angle = GetNumberArg(L, 4, false);
    EAxisAngle myAa(axis, angle);
    Vector3 newEuler = myAa.ToEuler();
    lua_pushnumber(L, newEuler.x);
    lua_pushnumber(L, newEuler.y);
    lua_pushnumber(L, newEuler.z);
    return 3;
}

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

LUA_FUNCTION(DotProduct)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 7))
        return 0;
    Vector3 rot(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    Vector3 b(GetNumberArg(L, 4, false), GetNumberArg(L, 5, false), GetNumberArg(L, 6, false));
    bool normalize = n >= 7 ? GetNumberArg(L, 7, true) != 0 : false;
    if (normalize) {
        rot.Normalize();
        b.Normalize();
    }
    lua_pushnumber(L, rot.Dot(b));
    return 1;
}

LUA_FUNCTION_TODO(GetAngleBetween)

LUA_FUNCTION_TODO(GetAxisAngleBetween)

LUA_FUNCTION_TODO(RotateLike)

LUA_FUNCTION_TODO(MathRotateY)

LUA_FUNCTION_TODO(ApplyOrientation)

LUA_FUNCTION_TODO(InvertOrientation)

LUA_FUNCTION(RotateVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    Vector3 curVec(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    Vector3 rot(GetNumberArg(L, 4, false), GetNumberArg(L, 5, false), GetNumberArg(L, 6, false));
    Vector3 aResult = rot.Rotate(curVec);
    lua_pushnumber(L, aResult.x);
    lua_pushnumber(L, aResult.y);
    lua_pushnumber(L, aResult.z);
    return 3;
}

LUA_FUNCTION(RandomVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    float myMaxLen = n >= 1 ? GetNumberArg(L, 1, true) : 1.0f;
    float theMinLen = n >= 2 ? GetNumberArg(L, 2, true) : -1.0f;
    PushVectorXYZ(L, GetRandomVector(myMaxLen, theMinLen, -1.0f, -1.0f));
    return 3;
}

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

LUA_FUNCTION(Reset)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    char myPath[200];
    _snprintf(myPath, 200, "workresource\\levels\\%s.plb", g_game->m_levelName);
    myPath[199] = 0;
    for (int nIndex = 0; myPath[nIndex]; nIndex++) {
        if (myPath[nIndex] == '/')
            myPath[nIndex] = '\\';
    }
    EFile theFile;
    if (!theFile.Open(myPath, 0)) {
        LuaCallSite(L);
    } else {
        theFile.Close();
        g_game->ResetLevel(myPath, 0);
    }
    return 0;
}

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

LUA_FUNCTION(SetEntityFlag)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned int type = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    bool on = GetNumberArg(L, 3, false) != 0;
    if (type == 0x10) {
        ent->GetPhysics()->SetMode(0x80, on);
        return 0;
    }
    ((EScriptEntity *)ent)->SetFlag(type, on);
    if (type == 0x200000 && ((EEntityVirtuals *)ent)->HasMesh()) {
        EMeshEntity *theMesh = (EMeshEntity *)ent;
        theMesh->UpdateCollisionMesh();
    }
    if (type == 0x80000000 && ((EEntityVirtuals *)ent)->HasMesh() && on)
        ((EMeshEntity *)ent)->FadeIn(0.001f);
    return 0;
}

LUA_FUNCTION(GetEntityFlag)
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
    if (flag == 0x10) {
        float val = ent->GetPhysics()->HasFlag(0x80) ? 1.0f : 0.0f;
        lua_pushnumber(L, val);
        return 1;
    }
    float val = ent->HasFlag(flag) ? 1.0f : 0.0f;
    lua_pushnumber(L, val);
    return 1;
}

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

LUA_FUNCTION(GetLookTarget)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ELookTrigger *ent = (ELookTrigger *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    ELookAtInfo *look = ent->m_lookAt;
    if (look && look->GetTarget()) {
        lua_getref(L, look->GetTarget()->m_ref);
        if ((void *)g_game->GetPlayerControl() != ent)
            lua_pushnil(L);
        else
            lua_pushint(L, g_game->GetPlayerControl()->IsZLocked() != 0);
    } else {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}

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

LUA_FUNCTION(SetMeshLookAtParams)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    ELookTrigger *ent = (ELookTrigger *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 theAxis(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    float lenSqArg = theAxis.LengthSq();
    if (lenSqArg > 1.0) {
        theAxis /= sqrt(lenSqArg);
        lenSqArg = 1.0f;
    }
    EQuat rot(0, 0, 0, 1);
    *(Vector3 *)&rot = theAxis;
    rot.w = sqrt(1.0f - lenSqArg);
    EQuat *myDst = &ent->m_lookRot;
    *myDst = rot;
    return 0;
}

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

LUA_FUNCTION(SetHeadConstraint)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 curLimits(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    for (int index = 0; index < 3; index++)
        ent->m_headConstraint[index] = (int)(&curLimits.x)[index] >> 1;
    return 0;
}

LUA_FUNCTION_TODO(GetHeadForwardVector)

LUA_FUNCTION(SetPlayerLookBox)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    float width = GetNumberArg(L, 1, false);
    float height = GetNumberArg(L, 2, false);
    float fDepth = n >= 3 ? GetNumberArg(L, 3, true) : -1.0f;
    EPlayerControl *myPc = g_game->GetPlayerControl();
    myPc->m_lookBoxWidth = width;
    myPc->m_lookBoxHeight = height;
    if (fDepth > 0.0)
        g_game->GetPlayerControl()->m_lookBoxDepth = fDepth;
    return 0;
}

LUA_FUNCTION(CreateNamedTimer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 5))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    float secs = GetNumberArg(L, 2, false);
    const char *szName = GetStringArg(L, 3, false);
    unsigned short idx = (unsigned short)GetNumberArg(L, 4, false);
    bool bRepeat = n >= 5 ? GetNumberArg(L, 5, true) != 0 : false;
    if (szName && !*szName)
        szName = 0;
    if (secs <= 0.0 || !idx)
        LuaCallSite(L);
    else
        g_game->m_scripts.CreateTimer(so, secs, idx, bRepeat != 0, szName);
    return 0;
}

LUA_FUNCTION(GetTimerSecs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    unsigned short id = (unsigned short)GetNumberArg(L, 2, false);
    const char *szName = n >= 3 ? GetStringArg(L, 3, true) : 0;
    ETimer *timer = GetScriptManager()->FindTimer(so, id, szName, 0);
    if (!timer) {
        lua_pushnumber(L, -1.0f);
        return 1;
    } else {
        lua_pushnumber(L, (float)(timer->m_endTime - GetScriptTime()));
        return 1;
    }
}

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

LUA_FUNCTION(KillTimer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 4))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    unsigned short id = (unsigned short)GetNumberArg(L, 2, false);
    bool quiet = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    const char *name = n >= 4 ? GetStringArg(L, 4, true) : 0;
    if (!id) {
        LuaCallSite(L);
        return 0;
    }
    if (!GetScriptManager()->KillTimer(so, id, name) && !quiet)
        DebugPrintf("%s: no timer on %s", LuaCallSite(L), so->GetName());
    return 0;
}

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

LUA_FUNCTION(SendMessage)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 5))
        return 0;
    EScriptObject *target = GetScriptObjectArg(L, 1, false, false);
    if (!target)
        return 0;
    EScriptObject *theSender = GetScriptObjectArg(L, 2, false, false);
    if (!theSender)
        return 0;
    const char *szMsg = GetStringArg(L, 3, false);
    const char *pszData = lua_tostring(L, 4);
    int nFrames = n >= 5 ? (int)GetNumberArg(L, 5, true) : 0;
    int time = nFrames;
    if (!pszData)
        pszData = "";
    if (!szMsg) {
        LuaCallSite(L);
        return 0;
    }
    g_game->m_scripts.SendMessage(szMsg, pszData, theSender, target, time);
    return 0;
}

LUA_FUNCTION_TODO(SendMessageEx)

LUA_FUNCTION(SendMessageToDomain)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 6))
        return 0;
    EScriptObject *myFromVal = GetScriptObjectArg(L, 1, false, false);
    if (!myFromVal)
        return 0;
    const char *pszDomainName = GetStringArg(L, 2, false);
    bool bRecurse = GetNumberArg(L, 3, false) != 0;
    const char *myMsg = GetStringArg(L, 4, false);
    const char *pszData = GetStringArg(L, 5, false);
    int delay = n >= 6 ? (int)GetNumberArg(L, 6, true) : 0;
    int frames = delay;
    if (!pszDomainName || !myMsg || !pszData) {
        LuaCallSite(L);
        return 0;
    }
    EDomain *domain = g_game->m_levels->GetDomainManager()->FindDomain(pszDomainName);
    if (!domain) {
        LuaCallSite(L);
        return 0;
    }
    domain->SendMessage(myFromVal, myMsg, pszData, delay, bRecurse);
    return 0;
}

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

LUA_FUNCTION(GetBoundingBox)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    Vector3 myCenter = pMesh->m_box.GetCenter();
    lua_pushnumber(L, myCenter.x);
    lua_pushnumber(L, myCenter.y);
    lua_pushnumber(L, myCenter.z);
    Vector3 newExtent = pMesh->m_box.GetExtent();
    lua_pushnumber(L, newExtent.x);
    lua_pushnumber(L, newExtent.y);
    lua_pushnumber(L, newExtent.z);
    return 6;
}

LUA_FUNCTION(SetMeshProperty)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    ETriggerA8 *theMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!theMesh)
        return 0;
    unsigned int nProp = (unsigned int)GetNumberArg(L, 2, false);
    unsigned int fVal = (unsigned int)GetNumberArg(L, 3, false);
    EResource *pTex = n >= 4 ? (EResource *)GetResourceArg(L, 4) : 0;
    if (!nProp)
        ScriptError("ScriptVM: 0 passed in as property constant. This probably isn't what you want.");
    theMesh->SetProperty(nProp, fVal != 0, pTex ? pTex->m_handle : 0);
    return 0;
}

LUA_FUNCTION(SetMeshMaterialProperty)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 4))
        return 0;
    ETriggerA8 *theMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!theMesh)
        return 0;
    unsigned int nProp = (unsigned int)GetNumberArg(L, 2, false);
    unsigned int fVal = (unsigned int)GetNumberArg(L, 3, false);
    EResource *pTex = n >= 4 ? (EResource *)GetResourceArg(L, 4) : 0;
    if (!nProp)
        ScriptError("ScriptVM: 0 passed in as property constant. This probably isn't what you want.");
    theMesh->SetMaterialProperty(nProp, fVal != 0, pTex ? pTex->m_handle : 0);
    return 0;
}

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

LUA_FUNCTION(PausePlayerControls)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool pause = GetNumberArg(L, 1, false) != 0;
    bool flag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!SetPlayerControlsPaused(pause, flag))
        DebugPrintf("%s: %s", LuaOwnerName(L), LuaCallSite(L));
    return 0;
}

LUA_FUNCTION(APIDontRunPlayerControls)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool bDontRun = GetNumberArg(L, 1, false) != 0;
    bool keep = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!keep) {
        ResetPlayerInput(0);
        g_game->SetControlHandler(0, 0);
        EPlayerControl *pPc = g_game->GetPlayerControl();
        if (bDontRun)
            ((EPlayerControlActions *)g_game->GetPlayerControl())->SetAction(g_dontRunActionName, 0, true, 0);
        else
            ((EPlayerControlActions *)g_game->GetPlayerControl())->SetAction(g_jumpActionName, 0, true, 0);
    }
    if (!SetPlayerDontRun(bDontRun))
        LuaCallSite(L);
    return 0;
}

LUA_FUNCTION(GetPlayerControlCameraRelativeVector)
{
    EZeroVector3 v;
    g_game->GetActionItems()->GetCameraRelativeVector(g_game->m_moveInput, &v, 0, 0);
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    return 3;
}

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

LUA_FUNCTION(LoadNewLevel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    if (!name)
        return 0;
    const char *ext = strrchr(name, '.');
    char path[200];
    if (!ext) {
        _snprintf(path, 200, "workresource\\levels\\%s.plb", name);
        path[199] = 0;
    } else {
        strncpy(path, name, 200);
        path[199] = 0;
    }
    for (int i = 0; path[i]; i++) {
        if (path[i] == '/')
            path[i] = '\\';
    }
    if (g_game->m_skipLevelCheck) {
        g_game->ResetLevel(path, 0);
    } else {
        EFile file;
        if (!file.Open(path, 0)) {
            LuaCallSite(L);
        } else {
            file.Close();
            g_game->ResetLevel(path, 0);
        }
    }
    return 0;
}

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

LUA_FUNCTION(CreateChildDomain)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *theParentName = GetStringArg(L, 1, false);
    const char *szName = GetStringArg(L, 2, false);
    if (!theParentName || !szName)
        return 0;
    EDomain *existing = g_game->GetLevelManager()->GetDomainManager()->FindDomain(szName);
    if (existing)
        LuaCallSite(L);
    EDomain *parent = ((EDomainManager *)g_game->m_unk81b4)->FindDomain(theParentName);
    if (!parent)
        return 0;
    parent->CreateChild(szName, 0);
    return 0;
}

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

LUA_FUNCTION(ResetMeshSkelPos)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    for (unsigned int idx = 0; idx < pMesh->m_joints.m_count; idx++) {
        EJoint *joint = ((EJoint **)pMesh->m_joints.m_data)[idx];
        joint->m_bone->m_pos = Vector3(0, 0, 0);
        joint->m_bone->GetInfo()->m_restPos = joint->m_bone->m_pos;
        joint->Update();
    }
    return 0;
}

LUA_FUNCTION_TODO(SetSkeletonScale)

LUA_FUNCTION(GetSkeletonScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    unsigned int index = n >= 2 ? (unsigned int)GetNumberArg(L, 2, true) : 0;
    if (index < 0 || index >= pMesh->m_joints.m_count) {
        DebugName(pMesh->GetModel()->GetName());
        return 0;
    }
    EJoint *joint = ((EJoint **)pMesh->m_joints.m_data)[index];
    PushVectorXYZ(L, joint->m_scale);
    return 3;
}

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

LUA_FUNCTION(StartMeshAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    int nAnim = (int)GetNumberArg(L, 2, false);
    bool loop = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    ETriggerA8 *aMesh = g_game->GetLevelManager()->GetLevel()->GetMeshAnims()->Find(name);
    if (!aMesh)
        return 0;
    aMesh->StartMeshAnim(nAnim, loop);
    return 0;
}

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

LUA_FUNCTION(SetNewAction)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ELookTrigger *ent = (ELookTrigger *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    const char *action = GetStringArg(L, 2, false);
    const char *szState = n >= 3 ? GetStringArg(L, 3, true) : 0;
    if (!action) {
        LuaCallSite(L);
        return 0;
    }
    if (szState) {
        if (!ent->SetNewActionState(action, 0, 0, szState))
            LuaCallSite(L);
    } else {
        if (!ent->SetNewAction(action, 0))
            LuaCallSite(L);
    }
    return 0;
}

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

LUA_FUNCTION(AddExtraRazIdle)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 3))
        return 0;
    const char *anim = GetStringArg(L, 1, false);
    bool bLoop = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    bool enable = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    g_game->GetPlayerControl()->AddExtraIdle(anim, bLoop, enable);
    return 0;
}

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

LUA_FUNCTION(SetUIControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    int index = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    switch (index) {
    case 11: {
        EScriptObject *obj = n >= 2 ? GetScriptObjectArg(L, 2, false, false) : 0;
        EScriptControlHandler *pHandler = (EScriptControlHandler *)g_game->GetControlHandler(index);
        pHandler->SetScript(obj);
        g_game->PushControlHandler(index);
        break;
    }
    case -1:
        g_game->PushControlHandler(index);
        break;
    default:
        LuaCallSite(L);
    }
    return 0;
}

LUA_FUNCTION(SetControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    int index = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    int cur = g_game->GetCurrentControlHandlerId();
    if (cur != index) {
        switch (index) {
        case 15: {
            EEntity *theLadder = n >= 2 ? GetEntityArg(L, 2, true, 0) : 0;
            if (!theLadder)
                return 0;
            g_game->GetActionItems()->SetLadderEntity(theLadder);
            break;
        }
        case 10: {
            EEntity *e = n >= 2 ? GetEntityArg(L, 2, true, 0) : 0;
            if (!e)
                return 0;
            g_game->SetControlHandler(index, (int)e);
            break;
        }
        default:
            g_game->SetControlHandler(index, 0);
        }
    }
    lua_pushint(L, cur);
    return 1;
}

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

LUA_FUNCTION(DaveCo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 5))
        return 0;
    float f = GetNumberArg(L, 1, false);
    const char *name = GetStringArg(L, 2, false);
    EEntity *ent = GetTriggerArg(L, 3, false);
    if (!ent)
        return 0;
    int mode = n >= 4 ? (int)GetNumberArg(L, 4, true) : 0x45;
    bool flag = n >= 5 ? GetNumberArg(L, 5, true) != 0 : false;
    int nIvalue = (int)f;
    lua_pushnumber(L, 4.0f);
    return 1;
}

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

LUA_FUNCTION(CreateProceduralTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    unsigned int width = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int height = (unsigned int)GetNumberArg(L, 2, false);
    const char *szName = n >= 3 ? GetStringArg(L, 3, true) : 0;
    void *tex = g_game->m_renderer->CreateTexture(0, width, height, 1, 0x50008, 0);
    int idx = g_game->m_resources->AddTexture(tex, 1, szName);
    lua_pushrawnumber(L, idx);
    return 1;
}

LUA_FUNCTION(FadeGetCurrentState)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    EFadeInfo theInfo;
    g_game->m_fader->GetFadeInfo(&theInfo);
    char curMsg[300];
    strncpy(curMsg, theInfo.name, 300);
    curMsg[299] = 0;
    ScriptError(curMsg);
    Color c = theInfo.color;
    c *= 255.0f;
    lua_pushnumber(L, c.r);
    lua_pushnumber(L, c.g);
    lua_pushnumber(L, c.b);
    lua_pushnumber(L, c.a);
    lua_pushstring(L, theInfo.name);
    return 5;
}

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

LUA_FUNCTION(StopAllBlendAnims)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    float newBlend = n >= 2 ? GetNumberArg(L, 2, true) : -1.0f;
    bool keep = false;
    bool active = true;
    EBlendStopFlags newFlags;
    newFlags.m_now = false;
    if (newBlend > kMinPeriod)
        ent->GetMesh()->StopBlendAnim(0, newBlend);
    else if (newBlend < -kMinPeriod)
        ent->GetMesh()->StopBlendAnim(0, newBlend);
    else
        ent->GetMesh()->StopBlendAnimNow(0);
    lua_pushnumber(L, 1);
    return 1;
}

LUA_FUNCTION(StopBlendAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    EEntity *entity = GetTriggerArg(L, 1, false);
    if (!entity)
        return 0;
    const char *aName = GetStringArg(L, 2, false);
    float blend = n >= 3 ? GetNumberArg(L, 3, true) : -1.0f;
    EMeshEntity *pMesh = (EMeshEntity *)entity;
    if (blend > kMinPeriod)
        pMesh->GetMesh()->StopBlendAnim(aName, blend);
    else if (blend < -kMinPeriod)
        pMesh->GetMesh()->StopBlendAnim(aName, blend);
    else
        pMesh->GetMesh()->StopBlendAnimNow(aName);
    lua_pushnumber(L, 1);
    return 1;
}

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

LUA_FUNCTION(DebugPrint)
{
    int n = lua_gettop(L);
    if (!n)
        return 0;
    for (int i = 1; i <= n; i++) {
        int luaType = lua_type(L, i);
        const char *typeName = lua_typename(L, luaType);
        switch (luaType) {
        case LUA_TUSERDATA:
        case LUA_TTABLE: {
            EScriptObject *so = GetScriptObjectArg(L, i, true, true);
            if (so) {
                const char *name = "??";
                const char *objType = "??";
                so->GetStringField("Name", &name);
                so->GetStringField("Type", &objType);
            }
            break;
        }
        case LUA_TNONE:
        case LUA_TNIL:
        case LUA_TNUMBER:
        case LUA_TFUNCTION:
            lua_tonumber(L, i);
            break;
        case LUA_TSTRING:
            lua_tostring(L, i);
            break;
        default:
            DebugPrintf("%s", typeName ? typeName : "unknown");
        }
    }
    return 0;
}

LUA_FUNCTION(DebugPrint2)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *prefix = GetStringArg(L, 1, false);
    int index = 2;
    int luaType = lua_type(L, 2);
    const char *tn = lua_typename(L, luaType);
    switch (luaType) {
    case LUA_TUSERDATA:
    case LUA_TTABLE: {
        EScriptObject *so = GetScriptObjectArg(L, 2, false, true);
        if (so) {
            const char *name = "??";
            const char *objType = "??";
            so->GetStringField("Name", &name);
            so->GetStringField("Type", &objType);
        }
        break;
    }
    case LUA_TNONE:
    case LUA_TNIL:
    case LUA_TNUMBER:
    case LUA_TFUNCTION:
        lua_tonumber(L, 2);
        break;
    case LUA_TSTRING:
        lua_tostring(L, 2);
        break;
    }
    return 0;
}

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

LUA_FUNCTION(GetPlayerDist)
{
    EEntity *ent = 0;
    if (lua_gettop(L) > 0)
        ent = GetEntityArg(L, 1, false, 0);
    else
        ent = LUA_OWNER(L)->m_entity;
    if (!ent) {
        lua_pushnumber(L, 0);
        return 1;
    }
    Vector3 playerPos = g_game->GetPlayerControl()->GetPhysics()->GetPosition();
    Vector3 loc = ent->GetPhysics()->GetPosition();
    Vector3 myDiff = loc - playerPos;
    lua_pushnumber(L, myDiff.Length());
    return 1;
}

LUA_FUNCTION(LoadNewPlayerMesh)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool flag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (name) {
        EPlayerControl *pc = g_game->GetPlayerControl();
        EMatrix saved = pc->GetPhysics()->m_matrix;
        pc->LoadMesh(name, 3, flag);
        pc->GetPhysics()->SetMatrix(saved, 0xb);
    }
    return 0;
}

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

LUA_FUNCTION(SetAbsPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 6))
        return 0;
    EEntity *target = GetEntityArg(L, 1, false, 0);
    if (!target)
        return 0;
    Vector3 pt(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    bool move = n >= 5 ? GetNumberArg(L, 5, true) != 0 : false;
    bool noCollide = n >= 6 ? GetNumberArg(L, 6, true) != 0 : false;
    EPhysics *phys = target->GetPhysics();
    if (move)
        phys->MoveTo(pt, 3, 0);
    else
        phys->SetAbsPosition(pt, !noCollide);
    phys->m_a0 = 0;
    PhysicsChanged(phys, "Lua setpos");
    return 0;
}

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

LUA_FUNCTION(GetAbsOrientation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EMatrix rot = ent->GetPhysics()->m_matrix.GetRotation();
    Vector3 euler;
    euler.Set(rot.ToEuler());
    lua_pushnumber(L, euler.x);
    lua_pushnumber(L, euler.y);
    lua_pushnumber(L, euler.z);
    return 3;
}

LUA_FUNCTION(GetRelOrientation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    EMatrix rot = ent->GetPhysics()->GetLocalMatrix().GetRotation();
    Vector3 euler;
    euler.Set(rot.ToEuler());
    lua_pushnumber(L, euler.x);
    lua_pushnumber(L, euler.y);
    lua_pushnumber(L, euler.z);
    return 3;
}

LUA_FUNCTION(SetAbsOrientation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 euler(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    Vector3 angles;
    angles.Set(euler);
    ent->GetPhysics()->SetRotation(EulerToMatrix(angles), 0);
    PhysicsChanged(ent->GetPhysics(), "Lua setori");
    return 0;
}

LUA_FUNCTION(SetRelOrientation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 euler(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    Vector3 angles;
    angles.Set(euler);
    ent->GetPhysics()->SetLocalRotation(EulerToMatrix(angles), 0);
    PhysicsChanged(ent->GetPhysics(), "Lua setori");
    return 0;
}

LUA_FUNCTION_TODO(GetOrientation)

LUA_FUNCTION(SetOrientation)
{
    return SetRelOrientation_impl(L);
}

LUA_FUNCTION(GetPosition)
{
    return GetAbsPosition_impl(L);
}

LUA_FUNCTION(SetPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 5))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    Vector3 pt(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    bool move = n >= 5 ? GetNumberArg(L, 5, true) != 0 : false;
    if (move)
        ent->GetPhysics()->MoveTo(pt, 3, 0);
    else
        ent->GetPhysics()->SetPosition(pt, true);
    ent->GetPhysics()->m_a0 = 0;
    PhysicsChanged(ent->GetPhysics(), "Lua setpos");
    return 0;
}

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

LUA_FUNCTION(SetScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 fScale(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    Vector3 one(1, 1, 1);
    EPhysics *thePhys;
    if (fScale != one) {
        thePhys = ent->GetPhysics();
        if (thePhys->GetMode() == 4 && thePhys->HasFlag(1)) {
            LuaCallSite(L);
            thePhys->SetMode(1, 0);
        }
    }
    ent->m_mesh->SetScale(fScale);
    return 0;
}

LUA_FUNCTION(PlayerLockInputTransform)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool locked = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_game->GetActionItems()->LockInputTransform(false);
    return 0;
}

LUA_FUNCTION(PreloadMesh)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool big = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (!name) {
        LuaCallSite(L);
        DumpLuaStack(L, 0, 0);
        return 0;
    }
    int nModeVal = !big && g_game->m_lowDetailMeshes ? 3 : 1;
    if (!g_meshManager)
        CreateMeshManager();
    g_meshManager->PreloadMesh(name, nModeVal);
    return 0;
}

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

LUA_FUNCTION(PreloadAnim)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 4))
        return 0;
    const char *szAnim = GetStringArg(L, 2, false);
    float priority = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    bool enable = n >= 4 ? GetNumberArg(L, 4, true) != 0 : false;
    if (!szAnim) {
        LuaCallSite(L);
        return 0;
    }
    if (priority < 0.0)
        LuaCallSite(L);
    int nFlags = 1;
    if (enable)
        nFlags |= 4;
    g_animManager->PreloadAnim(szAnim, priority, nFlags);
    return 0;
}

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

LUA_FUNCTION(SetPhysics)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    int myMode = (int)GetNumberArg(L, 2, false);
    Vector3 pt = ent->GetPhysics()->GetPosition();
    switch (myMode) {
    case 0:
        break;
    case 1:
        ent->GetPhysics()->SetMode(1, true);
        break;
    case 2:
        ent->GetPhysics()->SetMode(1, true);
        ent->GetPhysics()->SetMode(2, true);
        ent->GetPhysics()->SetMode(8, true);
        ent->GetPhysics()->SetMode(0x20, true);
        break;
    default:
        lua_pushnumber(L, 0);
        return 1;
    }
    lua_pushnumber(L, 1);
    return 1;
}

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

LUA_FUNCTION(GetBillboard)
{
    EMatrix view = g_game->GetRenderCamera()->GetViewMatrix();
    EZeroVector3 right;
    EZeroVector3 up;
    EZeroVector3 dir;
    view.GetBillboardAxes(&right, &up, &dir);
    lua_pushnumber(L, up.x);
    lua_pushnumber(L, up.y);
    lua_pushnumber(L, up.z);
    lua_pushnumber(L, right.x);
    lua_pushnumber(L, right.y);
    lua_pushnumber(L, right.z);
    return 6;
}

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

LUA_FUNCTION(RemoveDecal)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    void *index = lua_torawvalue(L, 1);
    bool now = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (now)
        g_game->m_decals->RemoveDecalNow(index);
    else
        g_game->m_decals->RemoveDecal(index);
    return 0;
}

LUA_FUNCTION(RemoveAllDecals)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    ETriggerA8 *newMesh = n >= 1 ? (ETriggerA8 *)GetTriggerA8Arg(L, 1, true) : 0;
    EResource *tex = n >= 2 ? (EResource *)GetResourceArg(L, 2) : 0;
    g_game->m_decals->RemoveAll(newMesh, tex ? tex->m_handle : 0);
    return 0;
}

LUA_FUNCTION(GetEntityFeelCenter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    Vector3 center = pMesh->m_model->m_bounds->m_box.GetCenter();
    lua_pushnumber(L, center.x);
    lua_pushnumber(L, center.y);
    lua_pushnumber(L, center.z);
    return 3;
}

LUA_FUNCTION(GetEntityFeelExtent)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    if (pMesh->m_model->m_bounds) {
        Vector3 extentVal = pMesh->m_model->m_bounds->m_box.GetExtent();
        lua_pushnumber(L, extentVal.x);
        lua_pushnumber(L, extentVal.y);
        lua_pushnumber(L, extentVal.z);
        return 3;
    } else {
        Vector3 size = pMesh->GetBounds().GetExtent();
        lua_pushnumber(L, size.x);
        lua_pushnumber(L, size.y);
        lua_pushnumber(L, size.z);
        return 3;
    }
}

LUA_FUNCTION(GetEntityLookCenter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntityVirtuals *ent = (EEntityVirtuals *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    Vector3 center = ent->GetLookBox().GetCenter();
    lua_pushnumber(L, center.x);
    lua_pushnumber(L, center.y);
    lua_pushnumber(L, center.z);
    return 3;
}

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

LUA_FUNCTION(Levitate)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EEntity *e = GetTriggerArg(L, 1, false);
    if (!e)
        return 0;
    float height = GetNumberArg(L, 2, false);
    float timeArg = GetNumberArg(L, 3, false);
    e->GetPhysics()->MoveBy(Vector3(0, height, 0));
    return 0;
}

LUA_FUNCTION(LinkCheckOnly)
{
    LinkScriptFunctions1();
    LinkScriptFunctions2();
    LinkScriptFunctions3();
    LinkScriptFunctions4();
    LinkScriptFunctions5();
    return 0;
}

LUA_FUNCTION(CalculateRightVector)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 dir(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    PushVectorXYZ(L, dir.Cross(g_upVector));
    return 3;
}

LUA_FUNCTION_TODO(EvaluateCurve)

LUA_FUNCTION(PurgeBlendAnims)
{
    return 0;
}

LUA_FUNCTION_TODO(NudgeTowardPosition)

LUA_FUNCTION(SetScriptValueTest)
{
    g_game->m_scripts.SetValue(g_game->GetPlayerControl()->GetScriptObject(), "stats.Yes1", g_game->Get8190()->GetScriptObject());
    g_game->m_scripts.SetValue(g_game->GetPlayerControl()->GetScriptObject(), "stats.Yes2", "Howdy");
    g_game->m_scripts.SetValue(g_game->GetPlayerControl()->GetScriptObject(), "stats.Yes3", 34.0f);
    g_game->m_scripts.SetValue(g_game->GetPlayerControl()->GetScriptObject(), "stats.Yes4", 98.7f);
    return 0;
}

LUA_FUNCTION(CopyStaticPose)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    ETriggerA8 *newFrom = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!newFrom)
        return 0;
    ETriggerA8 *theTo = (ETriggerA8 *)GetTriggerA8Arg(L, 2, false);
    if (!theTo)
        return 0;
    bool flag = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    theTo->CopyStaticPose(newFrom, flag);
    return 0;
}

LUA_FUNCTION(SetEditableLightMode)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int lightMode = (int)GetNumberArg(L, 1, false);
    g_game->m_editableLightMode = lightMode == 0;
    return 0;
}

LUA_FUNCTION(EnterClairvoyanceCloud)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 4))
        return 0;
    float r = n >= 1 ? GetNumberArg(L, 1, true) : 252.0f;
    float g = n >= 2 ? GetNumberArg(L, 2, true) : 103.0f;
    float b = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    float angle = n >= 4 ? GetNumberArg(L, 4, true) : 51.0f;
    Color num(r, g, b, angle);
    float pos;  // not used, but the original has it on the stack too
    num /= 255.0f;
    g_game->EnterClairvoyanceCloud(num);
    return 0;
}

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

LUA_FUNCTION(ForceRazJump)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *to = GetStringArg(L, 1, false);
    EPlayerControl *pc = g_game->GetPlayerControl();
    if (g_game->GetCurrentControlHandlerId())
        g_game->SetControlHandler(0, 0);
    ENormalControlHandler *myHandler = (ENormalControlHandler *)g_game->GetControlHandler(0);
    ((EPlayerControlActions *)pc)->SetAction(g_jumpActionName, 0, true, 0);
    pc->GetPhysics()->m_flags.Set(0x15);
    myHandler->ForceJump(true, to);
    return 0;
}

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

LUA_FUNCTION(NativeMulQuatQuat)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 8, 8))
        return 0;
    EQuat q1(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    EQuat q2(GetNumberArg(L, 5, false), GetNumberArg(L, 6, false), GetNumberArg(L, 7, false), GetNumberArg(L, 8, false));
    PushColor(L, *(Color *)&(q1 * q2));
    return 4;
}

LUA_FUNCTION_TODO(NativeMulQuatVec)

LUA_FUNCTION(NativeQuatToEuler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EQuat q(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    PushVectorXYZ(L, q.ToEuler());
    return 3;
}

LUA_FUNCTION(NativeEulerToQuat)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    Vector3 theEuler(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    EQuat q;
    Vector3 newAngles;
    newAngles.x = theEuler.x;
    newAngles.y = theEuler.y;
    newAngles.z = theEuler.z;
    q.SetEuler(newAngles);
    PushColor(L, *(Color *)&q);
    return 4;
}

LUA_FUNCTION(NativeQuatFromAxisAngle)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    Vector3 myAxis(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    float angle = GetNumberArg(L, 4, false);
    EQuat result;
    EAxisAngle myAa(myAxis, angle);
    result.SetAxisAngle(myAa);
    PushColor(L, *(Color *)&result);
    return 4;
}

LUA_FUNCTION(NativeQuatFromVectorPair)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 6, 6))
        return 0;
    Vector3 from(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false));
    Vector3 newToVal(GetNumberArg(L, 4, false), GetNumberArg(L, 5, false), GetNumberArg(L, 6, false));
    from.Normalize();
    newToVal.Normalize();
    PushColor(L, *(Color *)&QuatFromVectors(from, newToVal));
    return 4;
}

LUA_FUNCTION(NativeQuatSlerp)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 9, 9))
        return 0;
    EQuat newFrom(GetNumberArg(L, 1, false), GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    EQuat theTo(GetNumberArg(L, 5, false), GetNumberArg(L, 6, false), GetNumberArg(L, 7, false), GetNumberArg(L, 8, false));
    float t = GetNumberArg(L, 9, false);
    PushColor(L, *(Color *)&newFrom.Slerp(theTo, t, true));
    return 4;
}

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

LUA_FUNCTION(BitStore)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    unsigned int value = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int iBits = (unsigned int)GetNumberArg(L, 2, false);
    unsigned int myShift = (unsigned int)GetNumberArg(L, 3, false);
    unsigned int width = (unsigned int)GetNumberArg(L, 4, false);
    if (width + myShift > 32) {
        // does not fit into one word, the value stays as it is
    } else {
        unsigned int nMask = (1 << width) - 1;
        if (~nMask & iBits)
            iBits &= nMask;
        nMask <<= myShift;
        iBits <<= myShift;
        if (value & nMask)
            value &= ~nMask;
        value |= iBits;
    }
    lua_pushrawnumber(L, value);
    return 1;
}

LUA_FUNCTION(BitFetch)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    unsigned int value = (unsigned int)GetNumberArg(L, 1, false);
    unsigned int shift = (unsigned int)GetNumberArg(L, 2, false);
    unsigned int bits = (unsigned int)GetNumberArg(L, 3, false);
    unsigned int result = 0;
    if (bits + shift > 32) {
        // more than one word, the result stays 0
    } else {
        unsigned int mask = (1 << bits) - 1;
        result = (value >> shift) & mask;
    }
    lua_pushrawnumber(L, result);
    return 1;
}

static void ListDomainTree(EDomain *domain, int depth);

LUA_FUNCTION(ListDomains)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    const char *name = n >= 1 ? GetStringArg(L, 1, true) : 0;
    EDomain *theDomain = name ? g_game->GetLevelManager()->GetDomainManager()->FindDomain(name)
                           : (EDomain *)g_game->GetLevelManager()->GetDomainManager();
    if (!theDomain)
        return 0;
    ScriptWarning("Listing domains starting at '%s'", theDomain->GetName() ? theDomain->GetName() : "Root");
    ListDomainTree(theDomain, 0);
    return 0;
}

static void ListDomainTree(EDomain *domain, int depth)
{
    int i = 0;
    char indent[100];
    for (i = 0; i < depth; i++)
        indent[i] = ' ';
    indent[i] = 0;
    ScriptWarning("%s%s", indent, domain->GetName() ? domain->GetName() : "Root");
    EListNode *myNode = domain->GetChildren();
    while (myNode) {
        EDomain *kid = (EDomain *)ListNext(myNode);
        ListDomainTree(kid, depth + 4);
    }
}

LUA_FUNCTION(GetRootJointName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerA8 *pMesh = (ETriggerA8 *)GetTriggerA8Arg(L, 1, false);
    if (!pMesh)
        return 0;
    if (pMesh->m_joints.m_count > 0) {
        lua_pushstring(L, GetBoneName(pMesh->m_joints[0]->m_bone));
        return 1;
    }
    return 0;
}

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

LUA_FUNCTION(Get2DCoordsFromPos)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    EEntity *who = GetTriggerArg(L, 1, false);
    if (!who)
        return 0;
    float fScale = n >= 2 ? GetNumberArg(L, 2, true) : 1.0f;
    Vector3 pos = who->GetPhysics()->GetPosition();
    Vector3 screen = g_game->GetCamera()->WorldToScreen(pos, 0);
    float z = screen.z != 0 ? screen.z : 1.0f;
    float size = 1000.0 / z * fScale;
    lua_pushnumber(L, screen.x);
    lua_pushnumber(L, screen.y);
    lua_pushnumber(L, size);
    return 3;
}

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
