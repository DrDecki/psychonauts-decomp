// Skeleton storage and input reconstructed from the supported Steam executable.
// Layout and helper names are provisional; recursive loading remains external.
#include "engine.h"

// Local value views preserve established shared empty default constructors.
struct BoneMatrix : EMatrix {
    BoneMatrix() { *(EMatrix *)this = g_identityMatrix; }
    BoneMatrix &operator=(const BoneMatrix &other) {
        memcpy(this, &other, sizeof(BoneMatrix));
        return *this;
    }
};
struct BoneTranslation : Vector3 {
    BoneTranslation() { x = y = z = 0.0f; }
};
struct BoneRotation : EQuat {
    BoneRotation() { x=0.0f; y=0.0f; z=0.0f; w=1.0f; }
};
struct BoneScale : Vector3 {
    BoneScale() { x=1.0f; y=1.0f; z=1.0f; }
};
struct SkeletonData {
    SkeletonData();
    void *name;
    struct BoneInfo *info;
    unsigned int count;
    void *operator new(size_t size) { return _aligned_malloc(size,16); }
};
struct BoneInfo {
    BoneInfo();
    ~BoneInfo();
    void SetName(const char *name);
    BoneMatrix matrix;
    BoneTranslation translation;
    BoneRotation rotation;
    char *name;
    void *operator new[](size_t size) { return _aligned_malloc(size,16); }
};
struct Skeleton;
struct RuntimeBone {
    RuntimeBone();
    ~RuntimeBone();
    RuntimeBone *ReadInput(class EStream *,unsigned int,Skeleton *);
    void Transform(const EMatrix &);
    BoneMatrix first,second;
    BoneRotation rotation;
    BoneScale scale;
    BoneTranslation translation;
    Skeleton *owner;
    unsigned char index,parent,next;
    unsigned char flag0:1,flag1:1,hasChildren:1,flag3:1,flag4:1,flag5:1,flag6:1,flag7:1;
    void *operator new[](size_t size) { return _aligned_malloc(size,16); }
};
BoneInfo::~BoneInfo() { SetName(0); }
RuntimeBone::~RuntimeBone() {}

SkeletonData::SkeletonData() { name=0; info=0; count=0; } // 0x499ac0,43
BoneInfo::BoneInfo() { name=0; } // 0x491a10,117; member default init does rest
RuntimeBone::RuntimeBone() // 0x491b00,419
{
    owner=0; index=0xff; parent=0xff; next=0xff;
    flag0=1; flag1=0; hasChildren=0; flag3=0; flag4=0; flag5=0; flag6=0; flag7=0;
    *(EMatrix *)&first=g_identityMatrix;
    second=first;
}

class EStream {
public:
    virtual ~EStream() {}
    virtual const char *GetName() { return 0; }
    virtual void Read(void *,unsigned int)=0;
    template <class T> void Transfer(T *data) { Read(data, sizeof(T)); }
};
void ReadSkeletonName(EStream *,SkeletonData *); // cdecl 0x6a5a10
struct Skeleton {
    bool ReadInput(EStream *,unsigned int);
    SkeletonData *data;
    BoneInfo *info;
    RuntimeBone *root;
    void *group;
    RuntimeBone *allocation;
    char unknown14[0x3c-0x14];
    unsigned char flagsLow0:1,flag1:1,flagsHigh2:6;
};
bool Skeleton::ReadInput(EStream *stream,unsigned int version) // 0x49a040,608
{
    data=new SkeletonData;
    flag1=0;
    ReadSkeletonName(stream,data);
    stream->Transfer(&data->count);
    allocation=new RuntimeBone[data->count];
    data->info=new BoneInfo[data->count];
    info=data->info;
    for(unsigned int i=0;i<data->count;++i)
        allocation[i].index=(unsigned char)i;
    root=allocation;
    unsigned short serializedRootId;
    stream->Read(&serializedRootId,2);
    if(!root->ReadInput(stream,version,this)) return false;
    root->flag1=1;
    root->Transform(g_identityMatrix);
    return true;
}
// The 16-byte aligned value members give 0x60-byte info and 0xb0-byte runtime
// records. User-declared destructors preserve the observed 16-byte array cookies.

typedef char BoneInfoSize[(sizeof(BoneInfo) == 0x60) ? 1 : -1];
typedef char RuntimeBoneSize[(sizeof(RuntimeBone) == 0xb0) ? 1 : -1];
typedef char SkeletonDataSize[(sizeof(SkeletonData) == 12) ? 1 : -1];
