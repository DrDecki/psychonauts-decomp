// Native mesh joint lookup; names and partial layouts follow script callers.
#include "engine.h"

struct EBoneInfo {
    const char *GetName() { return m_name; }

    char pad0[0x5c];
    const char *m_name;             // 0x5c
};

struct ESkeleton;
struct ESkeletonGroup {
    char pad0[0x54];
    TArray<ESkeleton *> m_skeletons;
    // The original ID packer inlines this search in the /Od build.
    __forceinline unsigned short GetIndex(ESkeleton *skeleton)
    {
        for (unsigned short i = 0; i < m_skeletons.m_count; i++) {
            ESkeleton *item = ((ESkeleton **)m_skeletons.m_data)[i];
            if (item == skeleton)
                return i;
        }
        return 0;
    }
};

struct ESkeleton {
    char pad0[4];
    EBoneInfo *m_bones;             // 0x04
    char pad8[4];
    struct ESkeletonGroup *m_group; // 0x0c
    struct EBone *m_runtimeBones;   // 0x10
    unsigned short GetBoneId(EBone *bone);
};

struct EBone {
    EBoneInfo *GetInfo() { return &m_skeleton->m_bones[m_index]; }
    EBone *Find(const char *name);
    EBone *GetNext();
    unsigned short GetId();

    char pad0[0xa8];
    ESkeleton *m_skeleton;          // 0xa8
    unsigned char m_index;          // 0xac
    unsigned char m_ad;
    unsigned char m_next;           // 0xae; 0xff ends the sibling chain
    unsigned char m_flags : 2;
    unsigned char m_hasChildren : 1;
    unsigned char m_otherFlags : 5;
};

struct EJointInfo {
    const char *m_name;
};

struct EJoint {
    EJointInfo *m_info;             // 0x00
    char pad4[4];
    EBone *m_bone;                  // 0x08
};

void FreeFileName(char *block, int category, int flags);
char *CopyFileName(const char *name, int category);

struct EModel {
    void SetName(const char *name);
    void *GetSkeletonData() { return (char *)this + 0x6c; }

    char *m_name;                  // 0x00
    char pad4[0x78 - 4];
    bool m_hasSkeleton;            // 0x78
    char pad79[0xa4 - 0x79];
    void *m_association;            // 0xa4; entity slot +0x14 argument
};

class EMeshAnimState {
public:
    void *operator new(size_t size) { return _aligned_malloc(size, 16); }
    EMeshAnimState();
    void SetBlendShapes(EBlendShapeManager *manager);
    void SetSkeleton(void *skeleton) { m_skeleton = skeleton; }

    char pad0[0x18];
    EBlendShapeManager *m_blendShapes; // 0x18
    char pad1c[4];
    void *m_skeleton;               // 0x20
    char pad24[0x224 - 0x24];
};

class ETriggerA8 {
public:
    EBone *FindBone(const char *name);
    EJoint *FindJoint(const char *name);
    void *operator new(size_t size) { return _aligned_malloc(size, 16); }
    ETriggerA8();
    void EnsureAnimState();
    bool HasSkeleton() { return m_model->m_hasSkeleton; }
    EMeshAnimState *GetAnimState() { return m_animState; }
    ETriggerA8 *GetActual()
    {
        if (m_link == 0 || m_link == (ETriggerA8 *)-1)
            return this;
        else
            return m_link;
    }
    void SetContext(int flags, void *context);


    char pad0[8];
    ETriggerA8 *m_link;              // 0x08
    char padc[8];
    class EMeshEntity *m_entity;    // 0x14
    EModel *m_model;                // 0x18
    char pad1c[0x54 - 0x1c];
    TArray<EJoint *> m_joints;       // 0x54
    char pad60[0x1d4 - 0x60];
    EMeshAnimState *m_animState;     // 0x1d4
    char pad1d8[0x210 - 0x1d8];
};

EBone *ETriggerA8::FindBone(const char *name)
{
    EBone *bone;
    for (unsigned int i = 0; i < m_joints.m_count; i++) {
        bone = m_joints[i]->m_bone->Find(name);
        if (bone)
            return bone;
    }
    return 0;
}

EJoint *ETriggerA8::FindJoint(const char *name)
{
    for (unsigned int i = 0; i < m_joints.m_count; i++) {
        EJoint *joint = m_joints[i];
        const char *boneName = joint->m_info->m_name;
        if (strcmp(boneName, name) == 0)
            return joint;
    }
    return 0;
}

EBone *EBone::GetNext()
{
    return m_next != 0xff ? this - m_index + m_next : 0;
}

EBone *EBone::Find(const char *name)
{
    if (_stricmp(name, GetInfo()->GetName()) == 0)
        return this;
    EBone *child = m_hasChildren ? this + 1 : 0;
    for (; child; child = child->GetNext()) {
        EBone *bone = child->Find(name);
        if (bone)
            return bone;
    }
    return 0;
}

class EMeshManager {
public:
    ETriggerA8 *Load(const char *name, int flags);
    ETriggerA8 *Create(class EMeshEntity *entity);
    void Destroy(ETriggerA8 *mesh);
};
extern EMeshManager *g_meshManager;
void CreateMeshManager();
inline EMeshManager *GetMeshManager()
{
    if (!g_meshManager)
        CreateMeshManager();
    return g_meshManager;
}

class EMeshEntity {
public:
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void Vf14(void *association);
    bool LoadMesh(const char *name, int flags);
    void SetMesh(ETriggerA8 *mesh, bool flag);
    void SetHeadBone(const char *name);
    void *GetContext() { return m_context; }
    ETriggerA8 *GetMesh()
    {
        ETriggerA8 *mesh = m_mesh->GetActual();
        return mesh;
    }

    char pad4[0x34 - 4];
    void *m_context;                // 0x34
    char pad38[0xa8 - 0x38];
    ETriggerA8 *m_mesh;             // 0xa8
};

bool EMeshEntity::LoadMesh(const char *name, int flags)
{
    bool success = true;
    ETriggerA8 *actual = 0;
    if (name) {
        actual = GetMeshManager()->Load(name, flags);
        if (actual) {
            if (!actual->HasSkeleton()) {
            } else {
                actual->EnsureAnimState();
                actual->GetAnimState()->SetSkeleton(actual->m_model->GetSkeletonData());
            }
        } else {
            // SetMesh consumes 1 as its empty-mesh request, not an object pointer.
            actual = (ETriggerA8 *)1;
            success = false;
        }
    } else {
        // Same empty-mesh sentinel on the null-name path; never dereference it.
        actual = (ETriggerA8 *)1;
        success = false;
    }
    SetMesh(actual, false);
    ETriggerA8 *newMesh = GetMesh();
    if (newMesh->m_model->m_association)
        Vf14(newMesh->m_model->m_association);
    SetHeadBone("headJA_1");
    m_mesh->SetContext(0, GetContext());
    return success;
}

unsigned short ESkeleton::GetBoneId(EBone *bone)
{
    unsigned short index = m_group->GetIndex(this);
    return (index << 12) | (unsigned short)(bone - m_runtimeBones);
}

unsigned short EBone::GetId()
{
    return m_skeleton->GetBoneId(this);
}

ETriggerA8 *EMeshManager::Create(EMeshEntity *entity)
{
    ETriggerA8 *mesh = new ETriggerA8;
    mesh->m_entity = entity;
    return mesh;
}

inline EBlendShapeManager *GetBlendShapes()
{
    return g_game->m_blendShapes;
}

void ETriggerA8::EnsureAnimState()
{
    if (!m_animState) {
        m_animState = new EMeshAnimState;
        m_animState->SetBlendShapes(GetBlendShapes());
    }
}

void EMeshAnimState::SetBlendShapes(EBlendShapeManager *manager)
{
    m_blendShapes = manager;
}

void EModel::SetName(const char *name)
{
    if (m_name) {
        FreeFileName(m_name, 7, 1);
        m_name = 0;
    }
    if (name)
        m_name = CopyFileName(name, 7);
}

// Partial lifetime-only views, derived from the six observed native predicates.
// These offsets do not establish full layouts or reference-count semantics.
struct MeshOwnerLifetimeView {
    char pad0[0x80];
    unsigned int m_unknown80;
    char pad84[0xa0 - 0x84];
    unsigned int m_unknowna0;
};
class MeshLifetimeView {
public:
    virtual ~MeshLifetimeView();
    virtual void Vf04();
    virtual void Vf08();
    bool IsNullLink() { return m_link == 0; }
    bool IsSentinel() { return m_link == (MeshLifetimeView *)-1; }
    bool IsUnknown60Zero() { return m_unknown60 == 0; }
    char pad4[4];
    MeshLifetimeView *m_link;
    char padc[0x18 - 0x0c];
    MeshOwnerLifetimeView *m_owner;
    char pad1c[0x3c - 0x1c];
    unsigned int m_low3c : 5;
    unsigned int m_high3c : 27;
    char pad40[8];
    unsigned int m_low48 : 5;
    unsigned int m_high48 : 27;
    char pad4c[0x60 - 0x4c];
    unsigned int m_unknown60;
};
void DetachMeshForUnload(ETriggerA8 *mesh);
void EMeshManager::Destroy(ETriggerA8 *mesh)
{
    if (!((MeshLifetimeView *)mesh)->IsSentinel() &&
        !((MeshLifetimeView *)mesh)->m_high3c &&
        !((MeshLifetimeView *)mesh)->m_high48 &&
        !((MeshLifetimeView *)mesh)->m_owner->m_unknown80 &&
        ((MeshLifetimeView *)mesh)->IsUnknown60Zero() &&
        !((MeshLifetimeView *)mesh)->m_owner->m_unknowna0) {
        delete (MeshLifetimeView *)mesh;
    } else {
        DetachMeshForUnload(mesh);
        ((MeshLifetimeView *)mesh)->Vf08();
        if (!((MeshLifetimeView *)mesh)->IsNullLink() &&
            !((MeshLifetimeView *)mesh)->IsSentinel())
            delete (MeshLifetimeView *)mesh;
    }
}
