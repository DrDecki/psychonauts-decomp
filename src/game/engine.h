// Engine classes as far as they are known. Class and member names are made up
// unless the class has RTTI in Psychonauts.exe; the pads keep the members at
// the offsets the original code uses.
#ifndef ENGINE_H
#define ENGINE_H

extern "C" {
#include "../lua/lua.h"
#include "../lua/lstate.h"
}

class EScriptObject;
class EEntity;

struct Vector3 {
    float x, y, z;
    Vector3() {}
    Vector3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    Vector3 operator+(const Vector3 &v) const;
    Vector3 operator-(const Vector3 &v) const;
    Vector3 operator*(float s) const;
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

inline float Max(float a, float b) { return a > b ? a : b; }

struct EMatrix {
    Vector3 TransformPoint(const Vector3 &v);
    float m[4][4];
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
    float x, y, z, w;
};

extern Vector3 g_zeroVector;

// growable array, the first two words are bit fields
class EArrayBase {
public:
    void Reserve(int elemSize, int count);

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
    void Clear();
    T *GetData() { return (T *)m_data; }
};

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

    char pad0[0x10];
    EMatrix m_matrix;               // 0x10
    char pad50[0x94 - 0x50];
    EBitSet m_flags;                // 0x94
};

// run time type of the entities
class EClassInfo {
public:
    bool IsDerivedFrom(EClassInfo *base);
};

class EEntity {
public:
    EEntity *FindEntity(const char *name);
    EScriptObject *GetScriptObject() { return m_scriptObject; }
    EPhysics *GetPhysics() { return m_physics; }
    EClassInfo *GetClass() { return m_class; }
    bool IsDeleted() { return (m_flags & 3) != 0; }
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
    char pad20[0x64 - 0x20];
    unsigned int m_flags;           // 0x64
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

    EEntity *m_entity;      // 0x00
    char pad4[0x20 - 4];
    short m_ref;            // 0x20 lua_ref of the script table
};

// owns the Lua state and all script objects, embedded in GameApp
class EScriptManager {
public:
    EScriptObject *FindById(int id);
    EScriptObject *FindByName(const char *name);
    void Yield(lua_State *L);

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
    EBackpack *GetBackpack() { return m_backpack; }

    char pad0[0x10];
    EPhysics *m_player;        // 0x10
    char pad14[0x118 - 0x14];
    EBubble *m_bubble;              // 0x118
    char pad11c[0x120 - 0x11c];
    float m_movementScale;          // 0x120
    char pad124[0x128 - 0x124];
    EBackpack *m_backpack;          // 0x128
    char pad12c[0x186 - 0x12c];
    bool m_jumpingEnabled;          // 0x186
};

// one per player state (slide, ladder, ...), RTTI names like ELadderControlHandler
class EControlHandler {
public:
    virtual void Vf00();
};

class EFreezeManager {
public:
    void AddFrozen(const char *name, char **arg2, void *arg3, const Vector3 &pos, const EQuat &rot);

    char pad0[0x44];
    float m_freezeDistSq;           // 0x44
    float m_thawDistSq;             // 0x48
};

class ESaveManager {
public:
    void GetSaveString(TArray<char> &out);
};

struct EResource {
    void *GetHandle() { return m_handle; }
    void *m_handle;
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
};

extern Color g_white;

class EResourceManager {
public:
    EResource *GetById(int id);
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
};

class ELevelManager {
public:
    ELevel *GetLevel() { return m_level; }

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
    EWater *GetWater() { return m_water; }
    ELevel *GetLevel() { return GetLevelManager()->m_level; }
    EControlHandler *GetControlHandler(int id);
    void SetSteamRichPresence(const char *key, const char *value);

    char pad8[0x65 - 8];
    bool m_waterEnabled;            // 0x65
    char pad66[0x150 - 0x66];       // the vtable pointer is padded to 8 because of the doubles
    double m_time;                  // 0x150
    char pad158[0x19c - 0x158];
    EResourceManager *m_resources;  // 0x19c
    EWater *m_water;                // 0x1a0
    char pad1a4[0x1cc - 0x1a4];
    ELevelManager *m_levels;              // 0x1cc
    char pad1d0[0x1dc - 0x1d0];
    EFreezeManager *m_freeze;       // 0x1dc
    char pad1e0[0x818c - 0x1e0];
    EPlayerControl *m_playerControl;    // 0x818c
    char pad8190[0x81b4 - 0x8190];
    void *m_unk81b4;                // 0x81b4
    char pad81b8[0x8c7c - 0x81b8];
    EOverlay *m_overlay;            // 0x8c7c
    ESaveManager *m_save;           // 0x8c80
    char pad8c84[0x9570 - 0x8c84];
    int m_language;         // 0x9570
    char pad9574[0x9a34 - 0x9574];
    EScriptManager m_scripts;   // 0x9a34
};

extern GameApp *g_game;

inline EScriptManager *GetScriptManager() { return &g_game->m_scripts; }

// the script object running on a Lua thread (lua_State field 0x20)
#define LUA_OWNER(L) ((EScriptObject *)(L)->unknown20)

#endif
