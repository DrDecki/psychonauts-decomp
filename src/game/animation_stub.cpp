// Versioned animation input; class/field names are provisional.
#include "engine.h"
#include "diagnostic.h"
#include <new>

class EFileReader {
public:
    void ReadBytes(void *data, unsigned int size);
    void ReadString2b(char *data, int capacity);
    DiagnosticFile *GetFile() { return m_file; }
    char unknown0[0x10];
    DiagnosticFile *m_file;
};

struct AnimationHeader {
    unsigned int m_version;
    unsigned char m_field4, m_field5, m_field6, m_field7;
    unsigned int m_field8, m_fieldc, m_field10;
    union {
        unsigned int m_flags;
        struct { unsigned int m_format : 2; unsigned int m_otherFlags : 30; };
    };
};

struct AnimationHeaderOld {
    unsigned int m_magic, m_version;
    char unknown08[0x1c];
    unsigned int m_field24, m_field28, m_field2c, m_field30;
    char unknown34[0xc];
    unsigned int m_field40, m_field44, m_field48, m_flags;
};

#pragma pack(push, 1)
struct AnimationHeaderNew {
    unsigned int m_version;
    unsigned short m_flags, m_field26;
    unsigned char m_otherFlags;
    unsigned char m_field4, m_field5, m_field6, m_field7;
    unsigned char unknown;
};
#pragma pack(pop)

struct AnimationTrack {
    unsigned short m_count, m_capacity;
    unsigned char *m_data;
};
struct AnimationTrackSet {
    AnimationTrack *m_track;
    unsigned int unknown4[3];
};

template <class T> class AnimationArray : public TArray<T> {
public:
    void Resize(unsigned int count, bool exact);
};

template <class T> void AnimationArray<T>::Resize(unsigned int count, bool exact)
{
    if (!__is_class(T)) {
        if (exact || count > this->m_capacity)
            this->Reserve(sizeof(T), count + (exact ? 0 : ((count * 3) >> 3) + 16));
        this->m_count = count;
    } else if (count > this->m_count) {
        if (exact || count > this->m_capacity)
            this->Reserve(sizeof(T), count + (exact ? 0 : ((count * 3) >> 3) + 16));
        for (unsigned int i = this->m_count; i < count; ++i)
            new (&((T *)this->m_data)[i]) T;
        this->m_count = count;
    } else if (count < this->m_count) {
        for (unsigned int j = count; j < this->m_count; ++j)
            ((T *)this->m_data)[j].~T();
        this->m_count = count;
        if (exact)
            this->Reserve(sizeof(T), count);
    }
}

char *CopyFileName(const char *name, int category);

class AnimationStub {
public:
    void ReadHeader(EFileReader *reader);
    void RecalculateBytes();
    AnimationHeader m_header;
    char *m_name;
    char *m_sourceName;
    unsigned int m_bytes;
    unsigned short m_field24, m_field26;
    char unknown28[0xc];
    AnimationArray<AnimationTrackSet> m_sets;
    AnimationArray<AnimationTrack> m_tracks;
    AnimationArray<unsigned char> m_payload;
    void *m_events;
    void *m_blend;
    unsigned int m_field60, m_field64;
    unsigned char m_flags68;
};

void AnimationStub::ReadHeader(EFileReader *reader)
{
    unsigned int magic, formatVersion;
    unsigned int filePosition = reader->GetFile()->Seek(0, 1);
    reader->ReadBytes(&magic, 4);
    reader->ReadBytes(&formatVersion, 4);
    if (formatVersion < 200 || formatVersion > 201)
        return;
    if (formatVersion == 200) {
        AnimationHeaderOld old;
        old.m_magic = magic;
        old.m_version = formatVersion;
        reader->ReadBytes((char *)&old + 8, 0x48);
        memset(&m_header, 0, sizeof(m_header));
        m_header.m_version = old.m_version;
        m_header.m_field4 = (unsigned char)old.m_field24;
        m_header.m_field5 = (unsigned char)old.m_field28;
        m_header.m_field6 = (unsigned char)old.m_field2c;
        m_header.m_field7 = (unsigned char)old.m_field30;
        m_header.m_field8 = old.m_field40;
        m_header.m_fieldc = old.m_field44;
        m_header.m_field10 = old.m_field48;
        m_header.m_flags = old.m_flags;
    } else if (formatVersion == 201) {
        AnimationHeaderNew current;
        current.m_version = formatVersion;
        reader->ReadBytes((char *)&current + 4, 10);
        memset(&m_header, 0, sizeof(m_header));
        m_header.m_version = current.m_version;
        m_header.m_field4 = current.m_field4;
        m_header.m_field5 = current.m_field5;
        m_header.m_field6 = current.m_field6;
        m_header.m_field7 = current.m_field7;
        m_header.m_flags = current.m_otherFlags | current.m_flags;
        m_field26 = current.m_field26;
    }
}

void AnimationStub::RecalculateBytes()
{
    m_bytes = 0;
    m_bytes += m_sets.m_capacity * sizeof(AnimationTrackSet);
    m_bytes += m_tracks.m_capacity * sizeof(AnimationTrack);
    m_bytes += m_payload.m_capacity;
    m_bytes += 0x6c;
}

template void AnimationArray<AnimationTrackSet>::Resize(unsigned int, bool);
template void AnimationArray<AnimationTrack>::Resize(unsigned int, bool);
template void AnimationArray<unsigned char>::Resize(unsigned int, bool);
