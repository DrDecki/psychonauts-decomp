// Engine classes the script functions use, as far as they are known.
// Like engine.h: members at the offsets the code uses, padding in between.

#ifndef CLASSES_H
#define CLASSES_H

#include "engine.h"

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

struct EPowerButton {
    int m_power;                    // 0x00
    char pad4[0x74 - 0x4];
};

// The player: powers, inventory, thought bubble.
class EPlayer {
public:
    void ItemRemoved(const char *name);
    void AddItem(const char *name, bool flag);
    void FirestartingIgnited();
    void FirestartingDetonated();
    void ActivateLevitation(int frames);
    void SetConfused(bool on);
    void SetInvisibilityShader(float a, float b, float c, float d);
    bool IsPowerCoolingDown(int power);
    void DisablePower(int power);
    void EnablePower(int power, bool quiet);
    bool CanMapPower(int power);
    void StartClairvoyance();
    void CancelPsiBlast();
    void MapButton(int button, int power) { m_buttons[button].m_power = power; }
    EEntity *GetBubbleItem() { return m_bubbleItem; }
    int GetNumPowers() { return m_numPowers; }
    void SetPowerEnabled(int power, bool on) { s_powerEnabled[power] = on; }
    void SetPowerCoolDown(int power, bool on) { s_powerCoolDown[power] = on; }
    bool IsPowerEnabled(int power)
    {
        if (power < 8)
            return s_powerEnabled[power] && !IsPowerCoolingDown(power);
        else
            return false;
    }

    static bool s_powerEnabled[32];
    static bool s_powerCoolDown[8];

    char pad0[0x23c];
    EEntity *m_bubbleItem;          // 0x23c
    char pad240[0x24c - 0x240];
    int m_numPowers;                // 0x24c
    char pad250[0x274 - 0x250];
    bool m_bubbleSelectOnRelease;   // 0x274
    bool m_bubbleUseLeftStick;      // 0x275
    char pad276[0x294 - 0x276];
    float m_fmi;                    // 0x294
    char pad298[0x2e4 - 0x298];
    EPowerButton m_buttons[6];      // 0x2e4
    char pad59c[0x5a0 - 0x59c];
    ETriggerBase *m_bubbleSource;   // 0x5a0
};

class EFirestarting {
public:
    void PunchBalloon();
    void SlamGround();
    void StartFireBlast(const Vector3 &pos, int flags);
};

class EInvisibility {
public:
    void HopThroughLasso();
    void SetTime(void *ent, float secs);
};

class EPsiBlastManager {
public:
    float GetCharge(int index);
    void SetAmmo(int ammo);
    void AdjustAmmo(int amount);
    void SetColor(int index, const Color &color);
    void SetTexture(int index, void *texture);
    void Detonate(const Vector3 &pos, int owner);
    int Launch(const Vector3 &pos, const Vector3 &dir, int owner, float speed, bool homing, float delay);
    void Fire(EEntity *target, float range, EEntity **ignore, int numIgnore, int chain);

    char pad0[0x1c];
    ETriggerBase *m_source;         // 0x1c
    char pad20[0x28 - 0x20];
    bool m_new;                     // 0x28
    char pad29[0x2c - 0x29];
    int m_ammo;                     // 0x2c
};

struct EPowerSlot {
    int m_power;                    // 0x00
    char pad4[0x8 - 0x4];
    bool m_shown;                   // 0x08
    char pad9[0x14 - 0x9];
};

// the three powers shown in the corner of the screen
class EPowerHUD {
public:
    void SetButton(int button);
    void ClearButton(int button)
    {
        if (button == 0) {
            m_slots[1].m_shown = false;
            m_slots[1].m_power = 0;
        } else if (button == 1) {
            m_slots[0].m_shown = false;
            m_slots[0].m_power = 0;
        } else if (button == 2) {
            m_slots[2].m_shown = false;
            m_slots[2].m_power = 0;
        }
    }

    char pad0[0x20];
    EPowerSlot m_slots[3];          // 0x20
};

class EPsiBlast {
public:
    float GetCharge();
};

// the telekinesis hand
class ETKClaw {
public:
    static EClassInfo s_class;

    void PunchBalloon();
    void OpenClaw();
    void HideBubble();
    void SetSnapType(const char *type);
    void RemoveTarget(EEntity *ent, bool now);
    void SetState(int state);
    void SetTarget(EEntity *ent);
    void SetMoving(bool on);
    bool MoveToward(const Vector3 &pos, float turn);
    float GetAngle() { return m_angle; }
    Vector3 GetVelocity() { return m_velocity; }
    void UseDefaultSource() { m_defaultSource = true; }
    void SetSource(const Vector3 &pos)
    {
        m_defaultSource = false;
        m_source = pos;
    }

    char pad0[0x110];
    int m_state;                    // 0x110
    char pad114[0x11c - 0x114];
    EEntity *m_target;              // 0x11c
    char pad120[0x148 - 0x120];
    float m_angle;                  // 0x148
    Vector3 m_velocity;             // 0x14c
    char pad158[0x15e - 0x158];
    bool m_render;                  // 0x15e
    char pad15f[0x25c - 0x15f];
    Vector3 m_apparentDestination;  // 0x25c
    char pad268[0x2a0 - 0x268];
    bool m_defaultSource;           // 0x2a0
    char pad2a1[0x2a4 - 0x2a1];
    Vector3 m_source;               // 0x2a4
};

class ETelekinesis {
public:
    ETKClaw *GetClaw() { return m_claws[m_current]; }

    char pad0[0x230];
    ETKClaw *m_claws[4];            // 0x230
    int m_current;                  // 0x240
};

// control handler 8
class ETKControlHandler : public EControlHandler {
public:
    char pad4[0x170 - 4];
    int m_previousHandler;          // 0x170
    char pad174[0x254 - 0x174];
    bool m_holding;                 // 0x254
    bool m_requireHeldButton;       // 0x255
    bool m_requireTarget;           // 0x256
    bool m_oldWay;                  // 0x257
};

// control handler 3
class ELevitationControlHandler : public EControlHandler {
public:
    void InhibitJump(float secs);
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
    EPhysics *GetPhysics() { return m_physics; }

    char pad0[0xc];
    EScriptObject *m_scriptObject;  // 0x0c
    EPhysics *m_physics;            // 0x10
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

// One of the cameras of the game (Game/Camera.cpp). Type 1 follows a
// camera path, type 2 is the chase camera.
float NormalizeAngle(float degrees);

struct ECamera {
    Vector3 WorldToScreen(const Vector3 &pos, int flags);
    EMatrix &GetViewMatrix() { return m_view; }
    int GetType() { return m_type; }
    Vector3 GetPosition() { return m_position; }
    EQuat GetOrientation() { return m_orientation; }
    Vector3 GetRight() { return Vector3(m_matrix.m[0][0], m_matrix.m[1][0], m_matrix.m[2][0]); }
    Vector3 GetView() { return Vector3(-m_matrix.m[0][2], -m_matrix.m[1][2], -m_matrix.m[2][2]); }
    // the stack layout of the callers shows a local that is not used
    void SetHintAzimuth(float azimuth)
    {
        float old;
        m_hintAzimuth = azimuth;
    }
    void SetLadderAzimuth(float azimuth)
    {
        float old;
        m_ladderAzimuth = azimuth;
    }
    void SetPosition(const Vector3 &pos) { m_position = pos; }
    void SetOrientation(const EQuat &rot) { m_orientation = rot; }
    void SetMoved() { m_moved = true; }
    void SetRailRadius(float radius) { m_railRadius = radius; }
    void SetRailAltitude(float altitude) { m_railAltitude = altitude; }
    void ResetMode();
    void MoveToIdeal();
    float GetFieldOfView();
    float GetTargetHeight();
    void SetTargetHeight(float height);
    void ClearPath();
    bool IsPathRunning();
    void StopShake();
    void StartShakeDriven(float amount);
    void StartShake(const Vector3 &amount, float secs, float period, float phase);
    Vector3 GetIdealPosition();
    EQuat GetIdealOrientation();
    void SetAltitude(float altitude);
    void SetRadius(float radius);
    void SetRailPaths(void *a, void *b);
    void SetFreePosition(bool on);
    void SetFloatingTarget(const Vector3 &pos);
    void SetFixedTarget(const Vector3 &pos, bool snap);
    void SetFixedSpherical(const Vector3 &spherical, bool snap);
    void StartShakePeriodic(const Vector3 &amount, float period, float secs, float a, float b);
    void SetFieldOfView(float fov, float secs, float a, float b);
    void SetTertiaryTarget(class EMeshEntity *ent);
    void SetPrimaryTarget(void *ent, bool snap);
    void SetSecondaryTarget(void *ent, unsigned int mode, bool snap);
    void SetFixedPosition(const Vector3 &pos, bool snap, bool keepTarget, int flags);
    ECamera();
    Vector3 GetTargetPosition();
    void AttachTo(ETriggerBase *ent, const Vector3 &offset, const EQuat &rot, bool snap);
    static void Pause(int type);
    void *operator new(size_t size) { return _aligned_malloc(size, 16); }
    void SetFreeOrientation(bool on);
    bool TestSphere(const Vector3 &center, float radius);
    static void SetPhysicsAltitude(unsigned int index, float altitude);
    static void SetPhysicsRadius(unsigned int index, float radius);

    int m_type;                     // 0x00
    char pad4[0x8 - 0x4];
    Vector3 m_position;             // 0x08
    char pad14[0x20 - 0x14];
    EQuat m_orientation;            // 0x20
    char pad30[0x50 - 0x30];
    EMatrix m_matrix;               // 0x50
    char pad90[0x190 - 0x90];
    EMatrix m_view;                 // 0x190
    char pad1d0[0x458 - 0x1d0];
    float m_idealBlend;             // 0x458
    char pad45c[0x47c - 0x45c];
    EEntity *m_primaryTarget;       // 0x47c
    EEntity *m_secondaryTarget;     // 0x480
    EEntity *m_tertiaryTarget;      // 0x484
    char pad488[0x4a8 - 0x488];
    bool m_4a8_0 : 2;               // 0x4a8
    bool m_upwardTracking : 1;
    bool m_upwardOnly : 1;
    bool m_rotateToTarget : 1;
    char pad4a9[0x530 - 0x4a9];
    bool m_moved : 1;               // 0x530
    bool m_530_1 : 4;
    bool m_ir : 1;
    char pad531[0x914 - 0x531];
    float m_hintAzimuth;            // 0x914
    float m_ladderAzimuth;          // 0x918
    Vector2 m_autoRotateRange;      // 0x91c
    float m_distanceAdjustRange;    // 0x924
    float m_distanceAdjustParam;    // 0x928
    float m_fallingAltitude;        // 0x92c
    char pad930[0x954 - 0x930];
    bool m_954_0 : 1;               // 0x954
    bool m_free : 1;
    char pad955[0x968 - 0x955];
    float m_radius;                 // 0x968
    float m_altitude;               // 0x96c
    char pad970[0xa38 - 0x970];
    bool m_zLock : 1;               // 0xa38
    char pada39[0xa48 - 0xa39];
    float m_railRadius;             // 0xa48
    char pada4c[0xa50 - 0xa4c];
    float m_railAltitude;           // 0xa50
    char pada54[0xab0 - 0xa54];
};

// what one of the render channels shows (screen, monitors in the levels)
struct ERenderChannel {
    ECamera *m_camera;              // 0x00
    unsigned int m_flags;           // 0x04
    char pad8[0x18 - 0x8];
    float m_staticStart;            // 0x18
    float m_staticEnd;              // 0x1c
    float m_staticAmount;           // 0x20
    void *m_maskTexture;            // 0x24
    unsigned int m_filter;          // 0x28
    void *m_deeLiteTexture;         // 0x2c
    void *m_fixedTexture;           // 0x30
    ETriggerBase *m_deeLiteEntity;  // 0x34
    ETriggerBase *m_queryEntity;    // 0x38
    float m_visibility;             // 0x3c
};

class ECameraPathManager {
public:
    void *Find(unsigned int ref);
    void Delete(void *path);
};

class ELightmaps {
public:
    void Palettize();
};

struct ELightShadow {
    char pad0[0xc];
    float m_attenStart;             // 0x0c
    float m_attenEnd;               // 0x10
    char pad14[0x20 - 0x14];
    ETriggerBase *m_entity;         // 0x20
};

// RTEngine/Light.cpp
class ELight {
public:
    Vector3 GetPosition(int a, int b);
    void Update();

    char pad0[0x8];
    int m_type;                     // 0x08
    unsigned int m_flags;           // 0x0c
    char pad10[0x14 - 0x10];
    Vector3 m_color;                // 0x14
    float m_brightness;             // 0x20
    char pad24[0x30 - 0x24];
    Vector3 m_position;             // 0x30
    char pad3c[0x54 - 0x3c];
    float m_radius;                 // 0x54
    char pad58[0x6c - 0x58];
    float m_targetBrightness;       // 0x6c
    float m_fadeTime;               // 0x70
    char pad74[0x78 - 0x74];
    ELightShadow *m_shadow;         // 0x78
};

struct ERenderer {
    void GetViewport(unsigned int *width, unsigned int *height);
    void GetTextureSize(void *texture, int *width, int *height);
    bool FogSupported() { return m_fogSupported; }
    void *CreateTexture(int a, unsigned int width, unsigned int height, int levels, int format, int flags);

    char pad0[0x324];
    bool m_fogSupported;            // 0x324
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
    bool HasCollideGeometry();
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
    void SetMonitorChannel(int channel, int flags);

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
    bool OpenThoughtBubble(EEntity *item, int flags);
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

    char pad0[0xaf];
    unsigned char m_boneBits : 4;   // 0xaf
    unsigned char m_noDraw : 1;
    unsigned char m_noDrawSkip : 1;
    unsigned char m_noDrawChildren : 1;
    unsigned char m_flare : 1;
};

extern float g_radToDeg;

inline float RadToDeg(float rad) { return rad * g_radToDeg; }
void ScriptWarning(const char *fmt, ...);
void ScriptError(const char *fmt, ...);

// compiled out in the release build, the argument is still evaluated
inline void DebugName(const char *name) {}

extern int g_controlsPausedCount;

inline bool ControlsPaused() { return g_controlsPausedCount > 0; }

// Pool of elements that are found by an id (DFUtil/DataArray.inl). The id of
// a free element is 0.
template <class T>
class EDataArray {
public:
    unsigned int ElementNew();
    T *ElementGet(unsigned int id);
    T *ElementTryToGet(unsigned int id);
    void ElementDeleteByID(unsigned int id);
    bool ElementIsUsed(const T *element) { return element->m_dataId != 0; }

    char pad0[0xc];
    int m_capacity;                 // 0x0c
    int m_count;                    // 0x10
    int m_lastIndex;                // 0x14
    T *m_data;                      // 0x18
    char pad1c[0x20 - 0x1c];
    int m_firstFree;                // 0x20
    int m_nextKey;                  // 0x24
};

// A spline an entity can move along. Paths are shared and counted by EPathManager.
class ESplinePath {
public:
    void createGravityPath(int count, const Vector3 &start, const Vector3 &end, float time, float gravity, const Vector3 &up);
    int getLastWaypoint() const { return m_lastWaypoint; }

    float getWayPointParametricValue(unsigned int index);
    float getNearestPoint(const Vector3 &point);
    void SetExplicitParametricValues(float *values, unsigned int count);

    int m_lastWaypoint;             // 0x00
    char pad4[0x8 - 0x4];
    float m_length;                 // 0x08
    unsigned int m_numNormals;      // 0x0c
    char pad10[0x14 - 0x10];
};

struct EPathEntry {
    unsigned int m_dataId;          // 0x00
    ESplinePath m_path;             // 0x04
    unsigned int m_reference;       // 0x18
    int m_refCount;                 // 0x1c
    bool m_deleted;                 // 0x20
};

// Path/EPathManager.h
class EPathManager {
public:
    unsigned int getNewPath()
    {
        unsigned int id = m_paths->ElementNew();
        EPathEntry *entry = m_paths->ElementGet(id);
        entry->m_reference = id;
        entry->m_refCount = 1;
        entry->m_deleted = false;
        m_nPathCount++;
        return id;
    }
    ESplinePath *getPathByReference(unsigned int ref)
    {
        if (ref) {
            EPathEntry *entry = m_paths->ElementGet(ref);
            return &entry->m_path;
        } else {
            ref = getNewPath();
            if (ref) {
                EPathEntry *created = m_paths->ElementGet(ref);
                created->m_refCount = 1;
                created->m_deleted = false;
                return &created->m_path;
            }
        }
        return 0;
    }
    ESplinePath *getPath(unsigned int ref) { return getPathByReference(ref); }
    bool deletePathByReference(unsigned int ref)
    {
        EPathEntry *entry = m_paths->ElementTryToGet(ref);
        if (!entry)
            return false;
        if (entry->m_deleted)
            return false;
        entry->m_refCount--;
        entry->m_deleted = true;
        if (entry->m_refCount <= 0) {
            m_paths->ElementDeleteByID(ref);
            m_nUseCount--;
            m_nPathCount--;
        }
        return true;
    }
    bool releasePath(unsigned int ref)
    {
        EPathEntry *entry = m_paths->ElementGet(ref);
        entry->m_refCount--;
        if (entry->m_refCount <= 0) {
            m_paths->ElementDeleteByID(ref);
            m_nPathCount--;
        }
        return true;
    }
    void referencePath(unsigned int ref)
    {
        if (ref) {
            EPathEntry *entry = m_paths->ElementGet(ref);
            entry->m_refCount++;
        }
    }
    unsigned int findPath(ESplinePath *path)
    {
        for (int i = 0; i <= m_paths->m_lastIndex; i++) {
            EPathEntry *entry = &m_paths->m_data[i];
            if (m_paths->ElementIsUsed(entry) && &entry->m_path == path)
                return entry->m_dataId;
        }
        return 0;
    }

    static int m_nPathCount;
    static int m_nUseCount;

    EDataArray<EPathEntry> *m_paths;    // 0x00
};

// Moves an entity along a spline path. Every entity can have one.
class ESplineController {
public:
    void releasePath()
    {
        if (m_path) {
            unsigned int ref = m_pathManager->findPath(m_path);
            if (ref)
                m_pathManager->releasePath(ref);
            m_path = 0;
        }
        m_listener = 0;
    }
    void setPath(unsigned int ref)
    {
        releasePath();
        if (ref) {
            m_path = m_pathManager->getPathByReference(ref);
            m_pathManager->referencePath(ref);
        }
    }
    void setPath(ESplinePath *path)
    {
        releasePath();
        if (path) {
            unsigned int ref = m_pathManager->findPath(path);
            if (ref) {
                m_path = m_pathManager->getPathByReference(ref);
                m_pathManager->referencePath(ref);
            }
        }
    }
    float getLength() const { return m_path->m_length; }
    void setSync(bool on) {}    // nothing left of it in the release build
    float splineTraversalPeriod() const { return 1.0f / m_period; }
    void setFormationPosition(float x, float y, float z) { m_formationPosition.Set(x, y, z); }
    void init(int loopMode, float secs);
    void createUniformPath(int count);
    void setSplineTraversalPosition(float t);
    void setSplineTraversalPositionInMeters(float meters);
    void setSplineEaseInOut(float in, float out, bool smooth);

    static void setExplicitEvaluation(bool on) { m_bExplicitEvaluation = on; }
    void SetListener(EScriptObject *listener) { m_listener = listener; }
    ESplinePath *getSplinePath() const { return m_path; }
    void setActive(bool on) { m_active = on; }
    void setRelativeSplineFlag(bool on) { m_relative = on; }
    void setCharacterSplineFlag(bool on) { m_character = on; }
    void setDisplaySplineFlag(bool on) { m_display = on; }
    void setDisplaySplineUpFlag(bool on) { m_displayUp = on; }
    void setDisplaySplineAxisFlag(bool on) { m_displayAxis = on; }
    void setOrientateWithSplineFlag(bool on) { m_orientate = on; }
    void setFollowSplineFlag(bool on) { m_follow = on; }
    void setFormationFlag(bool on) { m_formation = on; }
    void setAnimationFlag(bool on) { m_animation = on; }
    void setFwdOrientateSplineFlag(bool on) { m_fwdOrientate = on; }
    void setConformJointsFlag(bool on) { m_conformJoints = on; }
    void setForceMoveFlag(bool on) { m_forceMove = on; }
    void setExternalController(EEntity *ent) { m_controller = ent; }
    void setSplineLoopMode(unsigned int mode) { m_loopMode = mode; }
    unsigned int splineLoopMode() const { return m_loopMode; }
    void setSplineTraversalPeriod(float secs) { m_period = 1.0f / secs; }
    void setSplineRelSpeed(float speed) { m_relSpeed = speed; }
    void setSplineTraversalDirection(float dir) { m_direction = dir; }
    float splineTraversalDirection() const { return m_direction; }
    void setOffset(float offset)
    {
        float len = getLength();
        if (len != 0)
            m_offset = offset / len;
    }
    void setSplineUniDirection(float dir) { m_uniDirection = dir; }
    void setSplineRollFactor(float factor) { m_rollFactor = factor; }
    void setSplineChaseDistance(float dist) { m_chaseDistance = dist; }
    void setSplineChaseVelocity(float min, float max) { m_chaseMinVelocity = min; m_chaseMaxVelocity = max; }
    void setSplineOrigin(const Vector3 &origin) { m_origin = origin; }
    void setSplineEntity(EEntity *ent) { m_splineEntity = ent; }
    void SplineUniformSubdivision(int count) { createUniformPath(count); }
    void setSplineAbsSpeed(float speed)
    {
        if (m_path) {
            float len = m_path->m_length;
            if (len != 0)
                m_period = 1.0f / (len / speed);
            m_relSpeed = 1.0f;
        }
    }
    void setSplineTraversalToWaypoint(unsigned int index)
    {
        if (m_path) {
            float t = m_path->getWayPointParametricValue(index);
            setSplineTraversalPosition(t);
        }
    }
    void setWorldOffset(Vector3 &offset)
    {
        m_worldOffset = offset;
        if (m_worldOffset.LengthSq() > 0)
            m_hasWorldOffset = true;
        else
            m_hasWorldOffset = false;
    }

    static bool m_bExplicitEvaluation;

    EEntity *m_controller;          // 0x00
    bool m_active : 1;              // 0x04
    bool m_relative : 1;
    bool m_character : 1;
    bool m_display : 1;
    bool m_displayUp : 1;
    bool m_displayAxis : 1;
    bool m_orientate : 1;
    bool m_follow : 1;
    bool m_formation : 1;           // 0x05
    bool m_animation : 1;
    bool m_fwdOrientate : 1;
    bool m_conformJoints : 1;
    bool m_forceMove : 1;
    bool m_unknown5 : 2;
    bool m_hasWorldOffset : 1;
    char pad6[0x8 - 0x6];
    unsigned int m_loopMode;        // 0x08
    char padc[0x14 - 0xc];
    float m_period;                 // 0x14
    float m_speed;                  // 0x18
    float m_relSpeed;               // 0x1c
    float m_direction;              // 0x20
    float m_offset;                 // 0x24
    float m_uniDirection;           // 0x28
    char pad2c[0x34 - 0x2c];
    float m_rollFactor;             // 0x34
    float m_chaseDistance;          // 0x38
    float m_chaseMinVelocity;       // 0x3c
    float m_chaseMaxVelocity;       // 0x40
    Vector3 m_origin;               // 0x44
    Vector3 m_formationPosition;    // 0x50
    EEntity *m_splineEntity;        // 0x5c
    Vector3 m_worldOffset;          // 0x60
    EScriptObject *m_listener;      // 0x6c
    char pad70[0x98 - 0x70];
    ESplinePath *m_path;            // 0x98
    EPathManager *m_pathManager;    // 0x9c
};

// RTEngine/ParticleEmitterEntity.cpp
class EParticleEmitter {
public:
    static EClassInfo s_class;

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
    virtual void SetMaxParticles(int count);    // 0x68
    virtual void Vf6c();
    virtual void Reset();                       // 0x70
    virtual bool IsTimed();                     // 0x74
    virtual void Vf78();
    virtual void Vf7c();
    virtual void Tick(float secs);              // 0x80

    void DetachChild();
    void AttachChild(EParticleEmitter *parent);
    void AgeParticles(float secs);
    void SetMaxStartOffset(float offset);
    void SetStageSize(float size);
    void SetStageDuration(float secs);
    bool IsActive();
    void SetActive(bool on);
    void SetNumStages(int count);
    void SetStartAlpha(float alpha);
    void SetEndAlpha(float alpha);
    void SetStartColor(float r, float g, float b, float a);
    void SetEndColor(float r, float g, float b, float a);
    void GetScale(float *x, float *y);
    void SetScale(float x, float y);
    void InjectParticles(int count);
    void SetParticleSize(float start, float end);
    void SetParticleStartEndSize(float start, float end);
    void SetPrimaryAffector(class EParticleAffector *aff);
    void SetSecondaryAffector(class EParticleAffector *aff);
    int AddAffector(class EParticleAffector *aff);
    void SetAffector(int index, class EParticleAffector *aff);
    void SetConeAngle(float a, float b);
    void UpdateLifetime();
    void SetParticleForce(float x, float y, float z);
    void SetPaused(bool on);
    void GetTexAnimInfo(int *a, int *b, int *c, float *d);
    void SetTexAnimInfo2D(int a, int b, int c, int d, float rate, bool flag);
    void SetActiveStage(unsigned int stage)
    {
        if (stage > (m_numStagesBits >> 5 & 0x7ffffff) - 1) {
            return;
        } else {
            m_activeStage = stage;
        }
    }
    bool TestFlag(unsigned int flag) { return (m_flags & flag) != 0; }
    void SetFlag(unsigned int flag, bool on)
    {
        if (on)
            m_flags |= flag;
        else
            m_flags &= ~flag;
    }

    char pad4[0x88 - 0x4];
    unsigned int m_numStagesBits;   // 0x88
    char pad8c[0x94 - 0x8c];
    unsigned int m_activeStage;     // 0x94
    char pad98[0xa4 - 0x98];
    int m_maxParticles;             // 0xa4
    int m_numActive;                // 0xa8
    char padac[0xb8 - 0xac];
    void *m_texture;                // 0xb8
    void *m_depthTexture;           // 0xbc
    char padc0[0xc4 - 0xc0];
    float m_coneAngle1;             // 0xc4
    float m_coneAngle2;             // 0xc8
    float m_maxStartOffset;         // 0xcc
    float m_speedMin;               // 0xd0
    float m_speedMax;               // 0xd4
    float m_sizeStart;              // 0xd8
    float m_sizeEnd;                // 0xdc
    float m_stageSizeStart;         // 0xe0
    float m_stageSizeEnd;           // 0xe4
    char pade8[0xf8 - 0xe8];
    float m_ppsMin;                 // 0xf8
    float m_ppsMax;                 // 0xfc
    float m_lifetimeMin;            // 0x100
    float m_lifetimeMax;            // 0x104
    char pad108[0x10c - 0x108];
    float m_rotation1;              // 0x10c
    char pad110[0x118 - 0x110];
    float m_rotation2;              // 0x118
    char pad11c[0x120 - 0x11c];
    unsigned int m_flags;           // 0x120
    unsigned char m_meshBits0 : 1;  // 0x124
    unsigned char m_meshLoaded : 1;
    char pad125[0x128 - 0x125];
    unsigned int m_blendMode;       // 0x128
    char pad12c[0x130 - 0x12c];
    float m_bounceFactor;           // 0x130
    char pad134[0x154 - 0x134];
    Vector3 m_force;                // 0x154
    char pad160[0x168 - 0x160];
    Color m_materialColor;          // 0x168
    char pad178[0x194 - 0x178];
    ETriggerBase *m_sortBuddy;      // 0x194
};

// an emitter that runs for a while and then stops
class ETimedEmitter : public EParticleEmitter {
public:
    bool IsDone() { return m_timeLeft <= 0.0; }

    char pad198[0x1b0 - 0x198];
    float m_timeLeft;               // 0x1b0
};

class EMeshEmitter : public EParticleEmitter {
public:
    static EClassInfo s_class;

    char pad198[0x1a8 - 0x198];
    char m_meshName[0x200];         // 0x1a8
};

// the soft line particle emitters (lightning and the like)
class ELineParticleEmitter {
public:
    static EClassInfo s_class;

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
    virtual void Deactivate();                  // 0xa4

    char pad4[0x1a0 - 0x4];
    float m_periodMin;              // 0x1a0
    float m_periodMax;              // 0x1a4
    char pad1a8[0x1b4 - 0x1a8];
    float m_frequency;              // 0x1b4
    float m_amplitude;              // 0x1b8
    char pad1bc[0x1c8 - 0x1bc];
    float m_density;                // 0x1c8
};

class EParticleAffector {
public:
    static EClassInfo s_class;

    char pad0[0x6c];
    unsigned int m_type;            // 0x6c
    char pad70[0x88 - 0x70];
    float m_force1;                 // 0x88
    float m_force3;                 // 0x8c
    float m_force2;                 // 0x90
    char pad94[0xac - 0x94];
    Vector3 m_turbulence;           // 0xac
    bool m_active;                  // 0xb8
};

#endif
