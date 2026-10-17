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
    void Set(const Vector3 &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }
    Vector3 Cross(const Vector3 &v) const;
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
};

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
    Vector3 ToEuler() const;
    EQuat operator*(float s) const { return EQuat(x * s, y * s, z * s, w * s); }
    float x, y, z, w;
};

extern Vector3 g_zeroVector;
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
    unsigned int m_bits[1];
};

char *StrDup(const char *s, int category);

// m_10 of every entity, RTTI EPhysics and subclasses
class EPhysics {
public:
    void SetNavGroup(unsigned int group);
    Vector3 &GetPosition() { return *(Vector3 *)m_matrix.m[3]; }
    Vector3 &GetVelocity() { return m_velocity; }
    Vector3 &GetAnimVelocity() { return m_animVelocity; }
    Vector3 GetTotalVelocity() { return m_velocity + m_animVelocity; }
    EMatrix &GetLocalMatrix() { return m_localMatrix ? *m_localMatrix : m_matrix; }
    void SetPosition(const Vector3 &pos, bool teleport);
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
    char pad50[0x68 - 0x50];
    Vector3 m_velocity;                 // 0x68
    Vector3 m_animVelocity;             // 0x74
    char pad80[0x94 - 0x80];
    EBitSet m_flags;                // 0x94
    char pad98[0xa0 - 0x98];
    int m_a0;                           // 0xa0
    char pada4[0xb8 - 0xa4];
    EMatrix *m_localMatrix;             // 0xb8
    char padbc[0xc8 - 0xbc];
    Vector3 m_forward;              // 0xc8
    char padd4[0xe0 - 0xd4];
    Vector3 m_up;                   // 0xe0
    Vector3 m_gravityDir;           // 0xec

    // 0x68 is the velocity
    void AddVelocity(const Vector3 &v);
};

// run time type of the entities
class EClassInfo {
public:
    const char *m_name;
    bool IsDerivedFrom(EClassInfo *base);
};

class EEntity {
public:
    EEntity *FindEntity(const char *name);
    void AttachTo(EEntity *parent, const struct EMatrix *offset, const char *bone);
    EScriptObject *GetScriptObject() { return m_scriptObject; }
    EPhysics *GetPhysics() { return m_physics; }
    const char *GetName() { return m_name; }
    EClassInfo *GetClass() { return m_class; }
    bool IsDeleted() { return (m_flags & 3) != 0; }
    bool IsSleeping() { return m_sleeping; }
    bool IsA(EClassInfo *cls)
    {
        if (cls == GetClass())
            return true;
        if (0)
            return false;
        return GetClass()->IsDerivedFrom(cls);
    }

    char pad0[0xc];
    EScriptObject *m_scriptObject;  // 0x0c
    EPhysics *m_physics;            // 0x10
    char pad14[0x1c - 0x14];
    EClassInfo *m_class;            // 0x1c
    const char *m_name;             // 0x20
    char pad24[0x64 - 0x24];
    union {
        unsigned int m_flags;       // 0x64
        struct {
            unsigned int m_flagBits : 5;
            unsigned int m_sleeping : 1;
        };
    };
};

class ETriggerBase : public EEntity {
public:
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
    void Yield(lua_State *L);
    void KillAllTimers(EScriptObject *so);
    void DoFile(const char *name, bool once);
    void KillScript(EScriptObject *so);
    struct ETimer *FindTimer(EScriptObject *so, unsigned short id, const char *name, int flags);
    void SendMessage(const char *msg, const char *data, EScriptObject *to, EScriptObject *from, int delay);
    void SetValue(EScriptObject *so, const char *path, EScriptObject *value);
    void SetValue(EScriptObject *so, const char *path, const char *value);
    void SetValue(EScriptObject *so, const char *path, float value);
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
    void ClearExtraIdles();
    void LoadMesh(const char *name, int flags, bool keep);
    void AddExtraIdle(const char *anim, bool a, bool b);

    char pad0[0xc - 0x0];
    EScriptObject *m_scriptObject;      // 0xc
    EPhysics *m_player;        // 0x10
    char pad14[0x118 - 0x14];
    EBubble *m_bubble;              // 0x118
    float m_lookBoxDepth;               // 0x11c
    float m_movementScale;          // 0x120
    char pad124[0x128 - 0x124];
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
    void *m_handle;
    void **m_data;
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

    char pad0[8];
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
class ELightManager;
class EBlendShapeManager;
class EResourceManager {
public:
    int AddTexture(void *tex, int flags, const char *name);
    EResource *GetById(int id);
    void DumpInfo(bool all);
    void WriteAllToPackFile(bool all, unsigned short flags);
    void *Get(void *handle);
    void *Find(const char *name);
    void *Load(const char *name, int flags, int unknown);
};

class ENavMesh;

class ELevel {
public:
    ENavMesh *GetNavMesh();
    EEntity *GetRoot() { return m_root; }

    char pad0[0xbc];
    float m_reflectionScale;        // 0xbc
    float m_refractionScale;        // 0xc0
    float m_reflectionBumpScale;    // 0xc4
    float m_refractionBumpScale;    // 0xc8
    float m_fresnelBumpScale;       // 0xcc
    char padd0[0x164 - 0xd0];
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
    float GetFrameSecs() { return m_frameSecs; }
    bool IsRunningAtDF() { return m_runningAtDF; }
    bool IsOXMBuild() { return m_oxmBuild; }
    void ExitClairvoyanceCloud();
    void ResetLevel(const char *path, int flags);
    float GetRealFrameSecs() { return m_realFrameSecs; }
    int GetCurrentControlHandlerId();
    struct EActionItems *GetActionItems() { return m_actionItems; }
    struct EInventory *GetInventory() { return (struct EInventory *)m_inventory; }
    void SetSteamRichPresence(const char *key, const char *value);

    // members of GameApp
    // start 0x8
    char pad8[0x10 - 0x8];
    EBlendShapeManager *m_blendShapes;  // 0x10
    struct EEventFileManager *m_events; // 0x14
    char pad18[0x2c - 0x18];
    EDebugDisplay m_debug;              // 0x2c
    char pad30[0x38 - 0x30];
    int m_timingInfo;                   // 0x38
    char pad3c[0x65 - 0x3c];
    bool m_waterEnabled;                // 0x65
    char pad66[0x72 - 0x66];
    bool m_debugLookAt;                 // 0x72
    char pad73[0xa2 - 0x73];
    unsigned char m_editableLightMode;          // 0xa2
    char pada3[0xba - 0xa3];
    bool m_subtitles;                   // 0xba
    char padbb[0xcd - 0xbb];
    bool m_timingInfoAll;               // 0xcd
    char padce[0xe5 - 0xce];
    bool m_e5;                          // 0xe5
    char pade6[0xeb - 0xe6];
    bool m_goalDisplay;                 // 0xeb
    char padec[0x100 - 0xec];
    float m_gameSpeed;                  // 0x100
    char pad104[0x144 - 0x104];
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
    char pad1a4[0x1b8 - 0x1a4];
    ELightManager *m_lights;            // 0x1b8
    char pad1bc[0x1c0 - 0x1bc];
    struct EDecalManager *m_decals;     // 0x1c0
    char pad1c4[0x1cc - 0x1c4];
    ELevelManager *m_levels;            // 0x1cc
    char pad1d0[0x1dc - 0x1d0];
    EFreezeManager *m_freeze;           // 0x1dc
    char pad1e0[0x818c - 0x1e0];
    EPlayerControl *m_playerControl;    // 0x818c
    struct EScriptHolder *m_8190;       // 0x8190
    char pad8194[0x81b4 - 0x8194];
    void *m_unk81b4;                    // 0x81b4
    char pad81b8[0x8238 - 0x81b8];
    struct EActionItems *m_actionItems; // 0x8238
    void *m_inventory;                  // 0x823c
    char pad8240[0x8398 - 0x8240];
    char m_levelFilter[0x100];          // 0x8398
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
    char pad8c8c[0x9035 - 0x8c8c];
    bool m_9035_0 : 1;                  // 0x9035
    bool m_9035_1 : 1;
    bool m_journalEnabled : 1;
    bool m_9035_3 : 1;
    bool m_9035_4 : 1;
    bool m_lowDetailMeshes : 1;
    char pad9036[0x9570 - 0x9036];
    int m_language;                     // 0x9570
    char pad9574[0x9576 - 0x9574];
    bool m_runningAtDF;                 // 0x9576
    char pad9577[0x9579 - 0x9577];
    bool m_oxmBuild;                    // 0x9579
    char pad957a[0x9584 - 0x957a];
    bool m_productionLoading;           // 0x9584
    char pad9585[0x95a0 - 0x9585];
    EHUD m_hud;                         // 0x95a0
    char m_levelName[0x9a34 - 0x989a];  // 0x989a
    EScriptManager m_scripts;           // 0x9a34
    // end of GameApp
};

extern GameApp *g_game;

inline EScriptManager *GetScriptManager() { return &g_game->m_scripts; }


// the script object running on a Lua thread (lua_State field 0x20)
#define LUA_OWNER(L) ((EScriptObject *)(L)->unknown20)

#endif
