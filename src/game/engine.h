// Engine classes as far as they are known. Class and member names are made up
// unless the class has RTTI in Psychonauts.exe; the pads keep the members at
// the offsets the original code uses.
#ifndef ENGINE_H
#define ENGINE_H

extern "C" {
#include <malloc.h>
#include <string.h>
#include "../lua/lua.h"
#include "../lua/lstate.h"
}

class EScriptObject;
class EEntity;

inline float Sqrt(float x)
{
    __asm {
        fld x
        fsqrt
        fstp x
    }
    return x;
}

inline float Square(float x) { return x * x; }

inline bool IsNear(float a, float b, float eps) { return Square(a - b) <= Square(eps); }

// The functions with inline asm (Sqrt) are never inlined, but MSVC still keeps
// stack space for their locals in every caller.
struct Vector3 {
    float x, y, z;
    Vector3() {}
    Vector3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    // MSVC does not inline these (they return a temporary built by a
    // constructor), but keeps stack space for their locals in the callers
    Vector3 operator+(const Vector3 &v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3 &v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    float Length() const { return Sqrt(x * x + y * y + z * z); }
    float Normalize()
    {
        float lenSq = x * x + y * y + z * z;
        if (Square(1e-5f) > lenSq)
            return 0.0f;
        else if (IsNear(lenSq, 1.0f, 1e-5f))
            return 1.0f;
        else {
            float len = Sqrt(lenSq);
            float inv = 1.0f / len;
            x *= inv;
            y *= inv;
            z *= inv;
            return len;
        }
    }
    void operator+=(const Vector3 &v);
    void EulerToVector(Vector3 *out) const;
    void Set(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    void Set(const Vector3 &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }
    Vector3 Cross(const Vector3 &v) const;
    float Dot(const Vector3 &v) const;
    float LengthSq() const;
    void operator/=(float s);
    bool operator!=(const Vector3 &v) const { return x != v.x || y != v.y || z != v.z; }
    Vector3 Rotate(const Vector3 &v) const;
    struct EAxisAngle ToAxisAngle() const;
};

struct EAABB {
    Vector3 GetCenter() const { return (max + min) * 0.5f; }
    Vector3 GetExtent() const { return (max - min) * 0.5f; }

    Vector3 min;
    Vector3 max;
};

// debug output, compiled out of the release build but its arguments are still evaluated
#define DebugPrintf(...) ((void)(__VA_ARGS__))

struct Vector2 {
    float x, y;
    Vector2() {}
    Vector2(float x_, float y_) { x = x_; y = y_; }
};

struct Color {
    float r, g, b, a;
    Color() {}
    Color(float r_, float g_, float b_, float a_) { r = r_; g = g_; b = b_; a = a_; }
    void operator*=(float s);
    void operator/=(float s);
};

inline int Clamp(int x, int lo, int hi)
{
    if (x > hi)
        return hi;
    else if (x < lo)
        return lo;
    else
        return x;
}

inline float Clamp(float x, float lo, float hi)
{
    if (x > hi)
        return hi;
    else if (x < lo)
        return lo;
    else
        return x;
}


inline float Max(float a, float b) { return a > b ? a : b; }
inline float Min(float a, float b) { return a < b ? a : b; }
inline unsigned int Min(unsigned int a, unsigned int b) { return a < b ? a : b; }

// a Vector3 padded to 16 bytes for the SSE code
struct __declspec(align(16)) Vector3A : public Vector3 {
};

// set when the CPU has SSE
extern bool g_useSSE;
struct EMatrix;
void SSETransformPoint(Vector3 *out, const EMatrix *m, const Vector3 *v);

// 16 byte aligned for the SSE code, like D3DXMATRIXA16
struct __declspec(align(16)) EMatrix {
    EMatrix() {}
    EMatrix GetRotation() const;
    void GetBillboardAxes(Vector3 *right, Vector3 *up, Vector3 *dir);
    Vector3 ToEuler() const;
    Vector3 TransformPoint(const Vector3 &v)
    {
        if (g_useSSE) {
            Vector3A out;
            SSETransformPoint(&out, this, &v);
            return out;
        } else {
            return Vector3(v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0],
                           v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1],
                           v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2]);
        }
    }
    Vector3 &GetTranslation() { return *(Vector3 *)m[3]; }

    float m[4][4];
};

struct EAxisAngle {
    EAxisAngle() {}
    EAxisAngle(const Vector3 &a, float an)
    {
        axis.x = a.x;
        axis.y = a.y;
        axis.z = a.z;
        angle = an;
    }

    Vector3 ToEuler() const;

    Vector3 axis;
    float angle;
};

struct EQuat {
    EQuat() {}
    EQuat(float x_, float y_, float z_, float w_) { x = x_; y = y_; z = z_; w = w_; }
    EQuat(const EMatrix &m);
    explicit EQuat(const Vector3 &euler) { SetEuler(euler); }
    EQuat(const EQuat &q)
    {
        x = q.x;
        y = q.y;
        z = q.z;
        w = q.w;
    }
    EQuat &operator=(const EQuat &q)
    {
        x = q.x;
        y = q.y;
        z = q.z;
        w = q.w;
        return *this;
    }
    void SetEuler(const Vector3 &angles);
    void SetAxisAngle(const EAxisAngle &aa);
    EQuat operator*(const EQuat &q) const;
    EQuat Slerp(const EQuat &to, float t, bool shortest) const;
    Vector3 ToEuler() const;
    EQuat operator*(float s) const { return EQuat(x * s, y * s, z * s, w * s); }
    float x, y, z, w;
};

extern Vector3 g_zeroVector;
extern EQuat g_identityQuat;
extern Vector3 g_upVector;
EMatrix EulerToMatrix(const Vector3 &angles);
EQuat QuatFromVectors(const Vector3 &from, const Vector3 &to);
Vector3 GetRandomVector(float maxLen, float minLen, float a, float b);
extern struct EMatrix g_identityMatrix;

// growable array, the first two words are bit fields
inline void *MemAlloc(int size, int category)
{
    void *p = _aligned_malloc(size, 16);
    return p;
}

inline void MemFree(void *block)
{
    void *mem = block;
    _aligned_free(mem);
}

class EArrayBase {
public:
    // not inlined by MSVC either, but its locals take stack space in the callers
    void Reserve(int elemSize, int count)
    {
        int size = elemSize == 0 ? -1 : m_capacity * elemSize;
        void *newMem = count ? MemAlloc(elemSize * count, m_category) : 0;
        void *oldData = m_data;
        if (m_count > (unsigned int)count)
            m_count = count;
        if (m_count)
            memcpy(newMem, oldData, m_count * elemSize);
        MemFree(oldData);
        m_capacity = count;
        m_data = newMem;
    }

    unsigned int m_category : 5;
    unsigned int m_count : 27;
    unsigned int m_flag : 1;
    unsigned int m_capacity : 31;
    void *m_data;
};

template <class T> class TArray : public EArrayBase {
public:
    TArray(int category, bool flag);
    ~TArray() { Clear(); Reserve(0, 0); }
    void Clear()
    {
        if (__is_class(T)) {
            for (unsigned int i = 0; i < m_count; i++)
                GetData()[i].~T();
        }
        Reserve(sizeof(T), 0);
    }
    T *GetData() { return (T *)m_data; }
    T &operator[](int i) { return ((T *)m_data)[i]; }
};

template <class T> TArray<T>::TArray(int category, bool flag)
{
    m_category = category;
    m_count = 0;
    m_flag = flag;
    m_capacity = 0;
    m_data = 0;
}

struct EBitSet {
    void Set(int bit)
    {
        int index = bit >> 5;
        unsigned int mask = 1 << (bit & 31);
        m_bits[index] |= mask;
    }
    void Clear(int bit)
    {
        int index = bit >> 5;
        unsigned int mask = 1 << (bit & 31);
        m_bits[index] &= ~mask;
    }
    bool Test(int bit)
    {
        int index = bit >> 5;
        unsigned int mask = 1 << (bit & 31);
        return (m_bits[index] & mask) != 0;
    }
    unsigned int m_bits[1];
};

char *StrDup(const char *s, int category);

// m_10 of every entity, RTTI EPhysics and subclasses
// what the collision code found under an entity
struct ESurfaceInfo {
    unsigned int m_bits : 24;
    unsigned int m_material : 4;
    unsigned int m_number : 4;
};

struct ECollideSphere {
    Vector3 m_center;
    float m_radius;
};

class EPhysics {
public:
    bool TestPhysFlag(unsigned int flag) { return (flag & m_physFlags) != 0; }
    unsigned int GetSurfaceFlags() { return *(unsigned int *)&m_surface; }
    void SetCollideMode(int mode);
    ESurfaceInfo GetSurface() { return m_surface; }
    EEntity *GetTouching(int i) { return m_touching[i]; }
    void SetNavGroup(unsigned int group);
    int GetMode();
    void SetMode(int mode, int flags);
    Vector3 &GetPosition() { return *(Vector3 *)m_matrix.m[3]; }
    bool HasFlag(unsigned int flag) { return (flag & m_physFlags) != 0; }
    Vector3 &GetVelocity() { return m_velocity; }
    Vector3 &GetAnimVelocity() { return m_animVelocity; }
    Vector3 GetTotalVelocity() { return m_velocity + m_animVelocity; }
    EMatrix &GetLocalMatrix() { return m_localMatrix ? *m_localMatrix : m_matrix; }
    void ResetSplineDelta();
    void SetPosition(const Vector3 &pos, bool teleport);
    void MoveTo(const Vector3 &pos, int mode, int flags);
    void SetAbsPosition(const Vector3 &pos, bool collide);
    void SetRotation(const EMatrix &rot, int flags);
    void SetLocalRotation(const EMatrix &rot, int flags);
    void SetMatrix(const EMatrix &m, int flags);
    Vector3 MoveBy(const Vector3 &delta)
    {
        Vector3 pos = GetLocalMatrix().GetTranslation();
        SetPosition(pos + delta, false);
        return GetLocalMatrix().GetTranslation();
    }

    char pad0[0x10];
    EMatrix m_matrix;               // 0x10
    char pad50[0x54 - 0x50];
    unsigned int m_physFlags;           // 0x54
    char pad58[0x68 - 0x58];
    Vector3 m_velocity;                 // 0x68
    Vector3 m_animVelocity;             // 0x74
    Vector3 m_splineDelta;              // 0x80
    char pad8c[0x94 - 0x8c];
    EBitSet m_flags;                // 0x94
    char pad98[0x9c - 0x98];
    ESurfaceInfo m_surface;         // 0x9c
    int m_a0;                           // 0xa0
    char pada4[0xa8 - 0xa4];
    EEntity *m_touching[4];         // 0xa8

    EMatrix *m_localMatrix;             // 0xb8
    char padbc[0xc8 - 0xbc];
    Vector3 m_forward;              // 0xc8
    char padd4[0xe0 - 0xd4];
    Vector3 m_up;                   // 0xe0
    Vector3 m_gravityDir;           // 0xec
    char padf8[0xfc - 0xf8];
    EEntity *m_ignoreEntity;        // 0xfc
    float m_ignoreTime;             // 0x100
    char pad104[0x108 - 0x104];
    unsigned short m_pushSpeed;     // 0x108
    unsigned char m_pushPriority;   // 0x10a
    char pad10b[0x110 - 0x10b];
    int m_collideType;              // 0x110
    float m_stepHeight;             // 0x114
    Vector3 m_surfaceNormal;        // 0x118
    Vector3 m_smoothSurfaceNormal;  // 0x124
    float m_surfaceDistance;        // 0x130
    char pad134[0x140 - 0x134];
    ECollideSphere m_spheres[3];    // 0x140
    int m_numSpheres;               // 0x170

    // 0x68 is the velocity
    void AddVelocity(const Vector3 &v);
};

class EClassInfo *GetClassByType(int type);

// run time type of the entities
class EClassInfo {
public:
    const char *m_name;
    int m_type;
    bool IsDerivedFrom(EClassInfo *base);
};

class EEntity {
public:
    EEntity *FindEntity(const char *name);
    void StopSounds();            // local verified native 0x00421520; provisional name
    class ESplineController *getSplineController();
    Vector3 GetPosition() { return GetPhysics()->GetPosition(); }
    void *GetSoundInstance() { return m_soundInstance; }
    void *PlaySound(void *sound, bool a, bool b);
    void *PlaySoundOnce(void *sound, bool a);
    void AttachTo(EEntity *parent, const struct EMatrix *offset, const char *bone);
    EScriptObject *GetScriptObject() { return m_scriptObject; }
    EPhysics *GetPhysics() { return m_physics; }
    const char *GetName() { return m_name; }
    EClassInfo *GetClass() { return m_class; }
    bool HasFlag(unsigned int flag) { return (m_entityFlags & flag) != 0; }
    bool IsDeleted() { return (m_flags & 3) != 0; }
    bool IsSleeping() { return m_sleeping; }
    void GetScriptValue(const char *name, int *out);
    bool IsType(int type)
    {
        if (GetClass()->m_type == type)
            return true;
        if (0)
            return false;
        return GetClass()->IsDerivedFrom(GetClassByType(type));
    }
    float GetInvisibility();
    void SetInvisibility(float amount);
    bool IsA(EClassInfo *cls)
    {
        if (cls == GetClass())
            return true;
        if (0)
            return false;
        return GetClass()->IsDerivedFrom(cls);
    }

    char pad0[0x4 - 0x0];
    unsigned int m_entityFlags;         // 0x4
    char pad8[0xc - 0x8];
    EScriptObject *m_scriptObject;  // 0x0c
    EPhysics *m_physics;            // 0x10
    char pad14[0x1c - 0x14];
    EClassInfo *m_class;            // 0x1c
    const char *m_name;             // 0x20
    char pad24[0x38 - 0x24];
    void *m_soundInstance;          // local binary-verified +0x38
    char pad3c[0x64 - 0x3c];
    union {
        unsigned int m_flags;       // 0x64
        struct {
            unsigned int m_flagBits : 5;
            unsigned int m_sleeping : 1;
        };
        struct {
            unsigned int m_unknownFlagsLow : 2;
            unsigned int m_echo : 1;
            unsigned int m_unknownFlagsHigh : 29;
        };
    };
};

class ETriggerBase : public EEntity {
public:
    float GetAlpha();
    void SetAuraColor(const Color &color);
    void SetCameraAlphaRadius(float radius);
    void SetDistortionScale(float scale);
    Vector2 GetTexCoordVel();
    void SetTexCoordVel(const Vector2 &vel);
    void SetDistortionColor(const Vector3 &color);
    void SetLightRigScale(const Vector3 &scale);
    void SetSolidColorIntensity(float intensity, float secs);
    void SetAlpha(float alpha, float ms);
    void SetAmbientLight(const Vector3 &color);
    void SetSolidColor(const Color &color);
    void SetGlow(float scale, float ms);
    static EClassInfo s_class;
};

// the C++ side of a script object (the table field m_pScriptObject points here)
class EScriptObject {
public:
    const char *GetName();
    int GetId();
    void SetState(lua_State *L);
    bool CanHandleMessage(const char *msg);
    bool GetStringField(const char *key, const char **out);
    EEntity *GetEntity();

    bool IsDead() { return m_dead; }

    EEntity *m_entity;      // 0x00
    char pad4[0x1c - 4];
    bool m_dead : 1;        // 0x1c
    bool m_1c_1 : 1;
    bool m_1c_2 : 1;
    bool m_1c_3 : 1;
    bool m_1c_4 : 1;
    bool m_paused : 1;
    char pad1d[0x20 - 0x1d];
    short m_ref;            // 0x20 lua_ref of the script table
};

// owns the Lua state and all script objects, embedded in GameApp
class EScriptManager {
public:
    EScriptObject *FindById(int id);
    EScriptObject *FindByName(const char *name);
    bool Yield(lua_State *L);
    void KillAllTimers(EScriptObject *so);
    void DoFile(const char *name, bool once);
    void KillScript(EScriptObject *so);
    void CreateTimer(EScriptObject *so, float secs, unsigned short id, int repeat, const char *name);
    bool KillTimer(EScriptObject *so, unsigned short id, const char *name);
    struct ETimer *FindTimer(EScriptObject *so, unsigned short id, const char *name, int flags);
    void SendMessage(const char *msg, const char *data, EScriptObject *to, EScriptObject *from, int delay);
    void SetValue(EScriptObject *so, const char *path, EScriptObject *value);
    void SetValue(EScriptObject *so, const char *path, const char *value);
    void SetValue(EScriptObject *so, const char *path, float value);
    void GetNumber(EScriptObject *so, const char *path, float *out);
    const char *GetCallerName(lua_State *L);
    void RegisterCutscene(EScriptObject *so);
    void UnregisterCutscene(EScriptObject *so);
    void Break(const char *msg, int flags);
    void PreloadClass(const char *name);
    bool IsBusy();
    void DumpDoFileArray(bool all);
    void DumpBaseClassHash();
    void DumpLSODupes();
    int GetSharedBase(const char *name, const char *source, int flags);
    bool ReloadClass(const char *name, char *buf, int size, const char *source, int flags);
    int GetMemoryUsage();

    char pad0[8];
    lua_State *m_L;         // 0x08
};



class EBackpack {
public:
    char pad0[0x6d];
    bool m_bounceDisabled;          // 0x6d
};

class EBubble {
public:
    char pad0[0x158];
    float m_squish;                 // 0x158
};

// player input settings
class EPlayerControl {
public:
    EBubble *GetBubble() { return m_bubble; }
    EScriptObject *GetScriptObject() { return m_scriptObject; }
    EPhysics *GetPhysics() { return m_player; }
    EBackpack *GetBackpack() { return m_backpack; }
    bool IsZLocked() { return m_zLocked; }
    void SetZLock(bool on);
    void GetScriptValue(const char *name, int *out);
    float GetInvisibility();
    void SetInvisibility(float amount);
    void ClearExtraIdles();
    void LoadMesh(const char *name, int flags, bool keep);
    void AddExtraIdle(const char *anim, bool a, bool b);
    EEntity *GetLockTarget() { return m_lockTarget; }

    char pad0[0xc - 0x0];
    EScriptObject *m_scriptObject;      // 0xc
    EPhysics *m_player;        // 0x10
    char pad14[0x80 - 0x14];
    EEntity *m_lockTarget;              // 0x80
    char pad84[0x118 - 0x84];
    EBubble *m_bubble;              // 0x118
    float m_lookBoxDepth;               // 0x11c
    float m_movementScale;          // 0x120
    bool m_noFallOffBall;           // 0x124
    unsigned char m_wallMaterial;   // 0x125
    char pad126[0x128 - 0x126];
    EBackpack *m_backpack;          // 0x128
    char pad12c[0x184 - 0x12c];
    bool m_zLocked;                 // 0x184
    bool m_confused;                // 0x185
    bool m_jumpingEnabled;          // 0x186
    char pad187;
    float m_lookBoxWidth;           // 0x188
    float m_lookBoxHeight;          // 0x18c
};

// one per player state (slide, ladder, ...), RTTI names like ELadderControlHandler
class EControlHandler {
public:
    virtual void Vf00();
};

class EFreezeManager {
public:
    void SetInstaThaw();
    void AddFrozen(const char *name, char **arg2, void *arg3, const Vector3 &pos, const EQuat &rot);

    char pad0[0x44];
    float m_freezeDistSq;           // 0x44
    float m_thawDistSq;             // 0x48
};

struct EProfile {
    const char *GetName() { return m_name; }

    char pad0[0x16f4];
    const char *m_name;             // 0x16f4
    char pad16f8[0x1724 - 0x16f8];
};

class ESaveManager {
public:
    void GetSaveString(TArray<char> &out);
    void ClearRazTable();
    EProfile *GetCurrentProfile() { return &m_profiles[m_current]; }
    const char *GetCurrentProfileName() { return GetCurrentProfile()->GetName(); }

    char pad0[0xc];
    int m_current;                  // 0x0c
    char pad10[4];
    EProfile m_profiles[3];         // 0x14
    char pad4580[0x46b0 - 0x14 - 3 * 0x1724];
    bool m_anyProfile;              // 0x46b0
    bool m_continueExists;          // 0x46b1
};

struct EResource {
    void *GetHandle() { return m_handle; }
    const char *GetName() { return m_name; }
    void *m_handle;
    void **m_data;
    char pad8[0xc - 0x8];
    const char *m_name;             // 0x0c
    char pad10[0x24 - 0x10];
    unsigned int m_flags;           // 0x24
};

class EWater {
public:
    void SetBumpTexture(void *tex) { m_bumpTexture = tex; }
    void SetRefractionTexture(void *tex) { m_refractionTexture = tex; }
    void SetFresnelTexture(void *tex) { m_fresnelTexture = tex; }

    char pad0[0x114];
    void *m_bumpTexture;            // 0x114
    char pad118[0x1a4 - 0x118];
    void *m_refractionTexture;      // 0x1a4
    char pad1a8[0x1b0 - 0x1a8];
    void *m_fresnelTexture;         // 0x1b0
};

class EOverlay {
public:
    int Display(void *tex, Vector2 pos, float sx, float sy, float u0, float v0, float u1, float v1,
                int a, int b, Color color);
    void SetSpriteColor(void *sprite, Color color);
    void FadeSprite(void *sprite, bool in, int frames);
    void RemoveSprite(void *sprite);
};

extern Color g_white;

// size EScriptManager 0xc
// size EDebugDisplay 0x4
// size EHUD 0x494
// size EHUDElements 0x24

// debug display modes, embedded in GameApp
class EDebugDisplay {
public:
    void CycleCollisionDisplay();
    void CycleDebugInfo();
    void CycleHighlightedSurface(int step);
    void CycleHighlightedSurface2();

    int m_mode;
};

// HUD messages, embedded in GameApp
class EHUD {
public:
    void DeletePrintMessages();
    bool IsPrintMessageDone();
    void PrintNote(const char *text);
    void SetProximityHint(const char *text, bool flag);
    void SetCollectedItemText(const char *text, bool flag);

    char pad0[0x2fa];
};

class EKeypad {
public:
    void Exit();
    void AddNumber(int number);
};

// HUD elements, embedded in GameApp
class EHUDElements {
public:
    void Show(int what, int on);
    void CustomAutosave(const char *name);
    void Autosave(const char *name);
    void AddNewGoal(const char *name);
    bool IsNewGoalDone();
    bool IsCompletedGoalDone();
    void ShowDemoEnd(bool show, int flags);

    class EPowerHUD *m_powers;          // 0x00
    char pad4[4];
    struct EHUDCounter *m_arrowheads;   // 0x08
    struct EHUDCounter *m_psiCards;     // 0x0c
    struct EHUDCounter *m_rank;         // 0x10
    EKeypad *m_keypad;                  // 0x14
    char pad18[4];
    struct ECheer *m_rankupCheer;       // 0x1c
    struct ECheer *m_cheering;          // 0x20
};

class EMenuPage {
public:
    void Reset();
    void Append(const char *name, const char *arg3, const char *arg2);
    void SetCheck(const char *name, bool check);
    void Select(const char *name);
};
class EMenuManager {
public:
    struct EScavengerSheet *GetScavengerSheet() { return m_scavenger; }
    void InitMap(const char *name);
    void ShowMap();
    EMenuPage *GetPage(const char *name);
    void ToggleInGameMenu();
    void ShowQuickPickMenu(int flags);
    void HideQuickPickMenu();
    void ExitJournal();

    char pad0[0x1cc];
    struct EMainMenuPage *m_mainPage;   // 0x1cc
    struct EScavengerSheet *m_scavenger;    // 0x1d0
};
class ESoundManager;
class EBlendShapeManager;
class EResourceManager {
public:
    int AddTexture(void *tex, int flags, const char *name);
    EResource *GetById(int id);
    void DumpInfo(bool all);
    void WriteAllToPackFile(bool all, unsigned short flags);
    EResource *Get(void *handle);
    void *Find(const char *name);
    void *Load(const char *name, int flags, int unknown);
    void SetTrapTextureString(const char *str);
    void *GetTexture(void *handle, int flags);
};

class ENavMesh;

class ELevel {
public:
    void SetMotionBlurState(unsigned int state, float ms);
    ENavMesh *GetNavMesh();
    EEntity *GetRoot() { return m_root; }

    char pad0[0x8 - 0x0];
    float m_nearZ;                      // 0x8
    float m_farZ;                       // 0xc
    char pad10[0x1c - 0x10];
    bool m_distanceFog;                 // 0x1c
    bool m_heightFog;                   // 0x1d
    char pad1e[0x20 - 0x1e];
    Vector3 m_fogColor;                 // 0x20
    float m_fogStart;                   // 0x2c
    float m_fogEnd;                     // 0x30
    Vector2 m_heightFogRange;           // 0x34
    char pad3c[0x50 - 0x3c];
    float m_detailCullStart;            // 0x50
    float m_detailCullEnd;              // 0x54
    void *m_distortionTexture;          // 0x58
    float m_distortionScale;            // 0x5c
    Vector3 m_distortionColor;          // 0x60
    unsigned int m_distortionFilter;    // 0x6c
    bool m_distortionFullScreen;        // 0x70
    char pad71[0x84 - 0x71];
    int m_shadowBlendMode;              // 0x84
    bool m_shadowFixedDirection;        // 0x88
    char pad89[0x8c - 0x89];
    void *m_shadowLightSource;          // 0x8c
    float m_shadowCullingBias;          // 0x90
    float m_staticLightIntensity;       // 0x94
    float m_dynamicLightIntensity;      // 0x98
    float m_lightRigIntensity;          // 0x9c
    char pada0[0xb8 - 0xa0];
    unsigned int m_numWaterBlurs;       // 0xb8
    float m_reflectionScale;        // 0xbc
    float m_refractionScale;        // 0xc0
    float m_reflectionBumpScale;    // 0xc4
    float m_refractionBumpScale;    // 0xc8
    float m_fresnelBumpScale;       // 0xcc
    unsigned int m_numGlowBlurs;        // 0xd0
    char padd4[0xe4 - 0xd4];
    bool m_auras;                       // 0xe4
    char pade5[0xe8 - 0xe5];
    void *m_environmentMap;             // 0xe8
    bool m_motionBlur;                  // 0xec
    char paded[0xfc - 0xed];
    float m_motionBlurOutput;           // 0xfc
    char pad100[0x104 - 0x100];
    float m_motionBlurTime;             // 0x104
    float m_motionBlurScale;            // 0x108
    unsigned int m_motionBlurFilter;    // 0x10c
    void *m_motionBlurMask;             // 0x110
    float m_motionBlurOutput2;          // 0x114
    float m_motionBlurOutputTime2;      // 0x118
    char pad11c[0x120 - 0x11c];
    void *m_movieStaticTexture;         // 0x120
    void *m_splineTexture;              // 0x124
    float m_splineThickness;            // 0x128
    float m_splineDensity;              // 0x12c
    bool m_freezeCoyoteProjections;     // 0x130
    char pad131[0x134 - 0x131];
    float m_psychedelicAlpha;           // 0x134
    float m_psychedelicTarget;          // 0x138
    float m_psychedelicTime;            // 0x13c
    char pad140[0x144 - 0x140];
    void *m_sortingSphereEntity;        // 0x144
    char pad148[0x15c - 0x148];
    bool m_shaderLOD;                   // 0x15c
    bool m_mirrorScene;                 // 0x15d
    unsigned char m_binaryAlphaThreshold; // 0x15e
    char pad15f[0x164 - 0x15f];
    EEntity *m_root;        // 0x164
    struct EMeshAnimManager *m_meshAnims;   // 0x168

    struct EMeshAnimManager *GetMeshAnims() { return m_meshAnims; }
};

struct EDomain;

class EDomainManager {
public:
    EDomain *FindDomain(const char *name);
};

class ELevelManager {
public:
    ELevel *GetLevel() { return m_level; }
    EDomainManager *GetDomainManager();

    ELevel *m_level;
};

// Partial sound interface. Class name corroborated by Steam-reference RTTI;
// method names/signatures are reconstructed from observed binding dispatch.
// Unidentified slots are placeholders only. See docs/native-session-20261017.md.
class ESoundManager {
public:
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void Vf14();
    virtual void Vf18();
    virtual void PurgeSoundData(const char *name, const char *suffix, bool all); // 0x1c
    virtual void *FindSound(const char *name); // 0x20; provisional name/handle type
    virtual void Vf24();
    virtual const char *GetSoundName(void *sound); // 0x28; observed pointer return
    virtual void *Play(void *sound, int a, int b); // 0x2c
    virtual void Stop(void *sound, void *entity_instance, void *sound_instance); // 0x30; handle semantics provisional
    virtual bool IsPlaying(void *sound); // 0x34; provisional method name
    virtual void StopOnInstance(void *instance); // 0x38; provisional name
    virtual void Vf3c();
    virtual int GetCueLength(void *sound); // 0x40; result type/units provisional
    virtual bool HasFlag(void *sound, int flag); // 0x44
    virtual bool IsAvailable();     // 0x48; provisional name, AL result observed
    virtual void Vf4c();
    virtual void Vf50();
    virtual bool IsDolbyDigital();  // 0x54
    virtual void StopAll();         // 0x58; binding null-checks the manager
    virtual void StopCategory(int category);    // 0x5c
    virtual void Vf60();
    virtual void MuteCategory(int category, float seconds);    // 0x64
    virtual void Vf68();
    virtual void UnmuteCategory(int category);  // 0x6c
    virtual void Vf70();
    virtual void PauseAll();         // 0x74
    virtual void PauseCategory(int category);   // 0x78
    virtual void ResumeAll();        // 0x7c
    virtual void ResumeCategory(int category);  // 0x80
    virtual void PauseNonMenu();     // 0x84
    virtual void ResumeNonMenu();    // 0x88
    virtual void Vf8c();
    virtual void Vf90();
    virtual void SetCategoryVolume(int category, float volume, float time);  // 0x94
    virtual void Vf98();
    virtual void SetMusicVolume(float volume, float time);  // 0x9c
    virtual void SetFXVolume(float volume, float time);     // 0xa0
    virtual void SetVoiceVolume(float volume, float time);  // 0xa4
    virtual void Vfa8();
    virtual void Vfac();
    virtual void Vfb0();
    virtual void Vfb4();
    virtual void Vfb8();
    virtual void Vfbc();
    // Third setter argument is an observed zero DWORD, semantic type unknown.
    virtual void SetTriggerVariable(unsigned int variable, float value, unsigned int unknown); // 0xc0
    virtual float GetTriggerVariable(unsigned int variable); // 0xc4
    virtual void SetParameterControl(const char *name, float value, void *instance, void *sound); // 0xc8; handle meanings provisional
    virtual void Vfcc();
    virtual void Vfd0();
    virtual void Vfd4();
    virtual void SetDistanceFactor(float factor);  // 0xd8
    virtual void SetRolloffFactor(float factor);   // 0xdc
    virtual void SetEcho(void *instance, bool enable);  // 0xe0
    virtual bool IsCategoryPlaying(void *instance, int category); // 0xe4
    virtual bool IsEntityPlayingCategory(EEntity *entity, int category); // 0xe8
    virtual void AttachInstance(EEntity *entity, void *sound_instance); // 0xec
    virtual void Vff0();
    virtual void Vff4();
    virtual void Vff8();
    virtual void Vffc();
    virtual void DumpSounds(bool all); // upstream observed slot 0x100
    virtual void DumpChannels();       // upstream observed slot 0x104
    virtual void Vf108();
    virtual void Set2DAttenuation(int value);   // 0x10c
};

struct ESaylineManager {
    bool IsDone(EScriptObject *speaker);
    void Remove(bool all);
    EScriptObject *GetSpeaker();

    char pad0[0xc];
    bool m_resetPosition;           // 0x0c
    char padd[0x1c - 0xd];
    Vector2 m_position;             // 0x1c
};

struct EInputRecorder {
    void RecordUseSaveGame(const char *name);
    void PlaybackUseSaveGame(const char *name);
    void PlaybackFile(const char *name);
    void StopRecording();
    void StopPlayback();
    void Playback(const char *name, int flags);
    bool IsRecording() { return m_recording; }
    bool IsPlaying() { return m_playing; }
    bool WaitForPlayback();
    void PauseRecording(bool pause);

    char pad0[0x12];
    bool m_showInfo;                // 0x12
    char pad13[0x19 - 0x13];
    bool m_recording;               // 0x19
    bool m_playing;                 // 0x1a
};

class ELight;

class ELightManager {
public:
    void DumpInfo();
    unsigned int FindLight(const char *name, void *mesh);
    void SetPsiLightColor(const Vector3 &color);
    ELight *Get(unsigned int handle);
};

class GameApp {
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
    virtual void UnlockAchievement(const char *name);   // 0x50

    ELevelManager *GetLevelManager() { return m_levels; }
    double GetTime() { return m_time; }
    EPlayerControl *GetPlayerControl() { return m_playerControl; }
    struct EScriptHolder *Get8190() { return m_8190; }
    EWater *GetWater() { return m_water; }
    ELevel *GetLevel() { return GetLevelManager()->m_level; }
    EControlHandler *GetControlHandler(int id);
    struct ECamera *GetCamera();
    struct ECamera *GetRenderCamera();
    bool CamerasEnabled();
    void SetCurrentCamera(unsigned int index, unsigned int channel);
    struct ECamera *GetCameraByIndex(int index);
    struct ERenderChannel *GetChannel(unsigned int index);
    void SetCameraType(struct ECamera *cam, unsigned int type);
    void SetChannelCamera(struct ECamera *cam, int channel);
    void DumpEntityInfo();
    const char *GetLocalizedText(const char *id);
    EHUD *GetHUD() { return &m_hud; }
    struct ESaylineManager *GetSaylines() { return m_saylines; }
    struct EInputRecorder *GetRecorder() { return m_recorder; }
    struct EHUDCounter *GetArrowheadCounter() { return m_hudElements.m_arrowheads; }
    struct EHUDCounter *GetPsiCardCounter() { return m_hudElements.m_psiCards; }
    struct EDialogBubble *GetDialogBubble() { return m_dialogBubble; }
    struct EBunkhouseUI *GetBunkhouse() { return m_bunkhouse; }
    struct EMainMenu *GetMainMenu() { return m_mainMenu; }
    struct ECredits *GetCredits() { return m_credits; }
    ESaveManager *GetSave() { return m_save; }
    EMenuManager *GetMenus() { return m_menus; }
    int GetLanguage() { return m_language; }
    void SetControlHandler(int id, int flags);
    void PushControlHandler(int id);
    float GetGameTimePlayed();
    void RemoveAllFontTexturedMeshes();
    void ReturnToStartMenu();
    bool SetSkyBox(const char *name, int index);
    void SetActiveSkyBox(int index);
    void UpdateDomainSleep();
    void DestroyEmitter(class EParticleEmitter *pe);
    float GetFrameSecs() { return m_frameSecs; }
    bool IsRunningAtDF() { return m_runningAtDF; }
    bool IsOXMBuild() { return m_oxmBuild; }
    void ExitClairvoyanceCloud();
    void EnterClairvoyanceCloud(Color color);
    void ResetLevel(const char *path, int flags);
    float GetRealFrameSecs() { return m_realFrameSecs; }
    int GetCurrentControlHandlerId();
    struct EActionItems *GetActionItems() { return m_actionItems; }
    class EPlayer *GetPlayer() { return m_player; }
    void SetSteamRichPresence(const char *key, const char *value);
    void LoadSoundData(const char *name, bool a, bool b);
    void StartMusicCue();
    void StartStinger();
    EScriptManager *GetScriptVM() { return &m_scripts; }
    class EPathManager *GetPathManager() { return m_pathManager; }
    class EPsiBlastManager *GetPsiBlasts() { return m_psiBlasts; }
    void SetVideoVolume(int volume) { m_videoVolume = volume; }
    class ETelekinesis *GetTelekinesis() { return m_telekinesis; }
    EScriptHolder *GetThoughtBubble() { return m_8190; }
    void SetClairvoyanceMask(void *texture) { m_clairvoyanceMask = texture; }
    void SetClairvoyanceFilter(unsigned int filter) { m_clairvoyanceFilter = filter; }
    void SetClairvoyanceTarget(EEntity *ent, bool fade);
    void SetSoundCameraRelative(bool relative, bool second)
    {
        m_soundCameraRelative = relative;
        m_soundCameraSecond = second;
    }


    // members of GameApp
    // start 0x8
    char pad8[0x10 - 0x8];
    EBlendShapeManager *m_blendShapes;  // 0x10
    struct EEventFileManager *m_events; // 0x14
    char pad18[0x1c - 0x18];
    class EPathManager *m_pathManager;  // 0x1c
    char pad20[0x24 - 0x20];
    int m_videoVolume;                  // 0x24
    char pad28[0x2c - 0x28];
    EDebugDisplay m_debug;              // 0x2c
    char pad30[0x38 - 0x30];
    int m_timingInfo;                   // 0x38
    char pad3c[0x56 - 0x3c];
    bool m_movieCaptureFlag;            // 0x56
    char pad57[0x5c - 0x57];
    bool m_noPlayerGlow;                // 0x5c
    char pad5d[0x65 - 0x5d];
    bool m_waterEnabled;                // 0x65
    char pad66[0x72 - 0x66];
    bool m_debugLookAt;                 // 0x72
    char pad73[0xa2 - 0x73];
    unsigned char m_editableLightMode;          // 0xa2
    char pada3[0xba - 0xa3];
    bool m_subtitles;                   // 0xba
    char padbb[0xc0 - 0xbb];
    bool m_screenshot;                  // 0xc0
    bool m_screenshotA;                 // 0xc1
    bool m_screenshotB;                 // 0xc2
    bool m_movieCapture;                // 0xc3
    char padc4[0xca - 0xc4];
    bool m_captureEnvironmentMap;       // 0xca
    bool m_refreshVertexShaders;        // 0xcb
    bool m_refreshPixelShaders;         // 0xcc
    bool m_timingInfoAll;               // 0xcd
    char padce[0xd6 - 0xce];
    bool m_notMovieCaptureFlag;         // 0xd6
    char padd7[0xe5 - 0xd7];
    bool m_e5;                          // 0xe5
    char pade6[0xeb - 0xe6];
    bool m_goalDisplay;                 // 0xeb
    char padec[0xee - 0xec];
    bool m_cameraHints;                 // 0xee
    char padef[0x100 - 0xef];
    float m_gameSpeed;                  // 0x100
    char pad104[0x118 - 0x104];
    unsigned int m_environmentMapSize;  // 0x118
    float m_environmentMapTime;         // 0x11c
    char pad120[0x144 - 0x120];
    void *m_animDisplayEntity;          // 0x144
    float m_frameSecs;                  // 0x148
    float m_realFrameSecs;              // 0x14c
    double m_time;                      // 0x150
    char pad158[0x190 - 0x158];
    unsigned int m_frameCount;          // 0x190
    struct ERenderer *m_renderer;       // 0x194
    ESoundManager *m_sound;             // 0x198
    EResourceManager *m_resources;      // 0x19c
    EWater *m_water;                    // 0x1a0
    char pad1a4[0x1ac - 0x1a4];
    class ELightmaps *m_lightmaps;      // 0x1ac
    char pad1b0[0x1b8 - 0x1b0];
    ELightManager *m_lights;            // 0x1b8
    char pad1bc[0x1c0 - 0x1bc];
    struct EDecalManager *m_decals;     // 0x1c0
    char pad1c4[0x1cc - 0x1c4];
    ELevelManager *m_levels;            // 0x1cc
    char pad1d0[0x1dc - 0x1d0];
    EFreezeManager *m_freeze;           // 0x1dc
    char pad1e0[0x209 - 0x1e0];
    bool m_skipSounds;                  // 0x209
    char pad20a[0x8140 - 0x20a];
    void *m_deeLiteTexture;             // 0x8140
    char pad8144[0x818c - 0x8144];
    EPlayerControl *m_playerControl;    // 0x818c
    struct EScriptHolder *m_8190;       // 0x8190
    class EFirestarting *m_firestarting; // 0x8194
    class EInvisibility *m_invisibility; // 0x8198
    char pad819c[0x81a4 - 0x819c];
    class EPsiBlastManager *m_psiBlasts; // 0x81a4
    char pad81a8[0x81b4 - 0x81a8];
    void *m_unk81b4;                    // 0x81b4
    char pad81b8[0x8238 - 0x81b8];
    struct EActionItems *m_actionItems; // 0x8238
    class EPlayer *m_player;                  // 0x823c
    class ETelekinesis *m_telekinesis;  // 0x8240
    char pad8244[0x8294 - 0x8244];
    float m_moveInput[2];               // 0x8294
    char pad829c[0x82b0 - 0x829c];
    bool m_skipDialog;                  // 0x82b0
    char pad82b1[0x8398 - 0x82b1];
    char m_levelFilter[0x80];           // 0x8398
    char pad8418[0x8420 - 0x8418];
    void *m_clawCloseSound;             // 0x8420
    void *m_clawOpenSound;              // 0x8424
    char pad8428[0x8462 - 0x8428];
    bool m_clairvoyanceOff;             // 0x8462
    bool m_clairvoyanceOn;              // 0x8463
    char pad8464[0x8468 - 0x8464];
    void *m_clairvoyanceMask;           // 0x8468
    unsigned int m_clairvoyanceFilter;  // 0x846c
    char pad8470[0x8498 - 0x8470];
    char pad8498[0x88c4 - 0x8498];
    struct EBunkhouseUI *m_bunkhouse;   // 0x88c4
    struct EMainMenu *m_mainMenu;       // 0x88c8
    char pad88cc[0x88d0 - 0x88cc];
    struct EDialogBubble *m_dialogBubble;// 0x88d0
    struct ESlideshow *m_slideshow;     // 0x88d4
    char pad88d8[0x8930 - 0x88d8];
    EHUDElements m_hudElements;         // 0x8930
    char pad8954[0x895c - 0x8954];
    struct ECredits *m_credits;         // 0x895c
    char pad8960[0x8c64 - 0x8960];
    EMenuManager *m_menus;              // 0x8c64
    struct EFader *m_fader;             // 0x8c68
    struct EInputRecorder *m_recorder;  // 0x8c6c
    struct ESaylineManager *m_saylines; // 0x8c70
    char pad8c74[0x8c7c - 0x8c74];
    EOverlay *m_overlay;                // 0x8c7c
    ESaveManager *m_save;               // 0x8c80
    struct ELine2DManager *m_lines2d;   // 0x8c84
    struct EText3DManager *m_text3d;    // 0x8c88
    char pad8c8c[0x8f9c - 0x8c8c];
    class ECameraPathManager *m_cameraPaths; // 0x8f9c
    char pad8fa0[0x9035 - 0x8fa0];
    union {
        struct {
            bool m_9035_0 : 1;                  // 0x9035
            bool m_9035_1 : 1;
            bool m_journalEnabled : 1;
            bool m_9035_3 : 1;
            bool m_9035_4 : 1;
            bool m_lowDetailMeshes : 1;
        };
        struct {
            unsigned char m_unknownSoundFlags : 6;
            unsigned char m_soundCameraRelative : 1;
            unsigned char m_soundCameraSecond : 1;
        };
    };
    char pad9036[0x9570 - 0x9036];
    int m_language;                     // 0x9570
    char pad9574[0x9576 - 0x9574];
    bool m_runningAtDF;                 // 0x9576
    char pad9577[0x9579 - 0x9577];
    bool m_oxmBuild;                    // 0x9579
    char pad957a[0x9583 - 0x957a];
    bool m_skipLevelCheck;              // 0x9583
    bool m_productionLoading;           // 0x9584
    char pad9585[0x95a0 - 0x9585];
    EHUD m_hud;                         // 0x95a0
    char m_levelName[0x9a30 - 0x989a];  // 0x989a
    int m_gameState;                    // 0x9a30
    EScriptManager m_scripts;           // 0x9a34
    // end of GameApp
};

extern GameApp *g_game;

inline EScriptManager *GetScriptManager() { return &g_game->m_scripts; }


// the script object running on a Lua thread (lua_State field 0x20)
#define LUA_OWNER(L) ((EScriptObject *)(L)->unknown20)

#endif
