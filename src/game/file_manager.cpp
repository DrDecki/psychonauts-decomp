// Native file-manager dispatch; EFileManager name is verified by RTTI.
// Method/helper/record names and unresolved handle meanings are provisional.
#include "diagnostic.h"

struct PackedFileSlot {
    char opaque0[8];             // original vtable/other state, preserved
    void *m_first;               // +0x08
    void *m_second;              // +0x0c
    unsigned int m_position;     // +0x10; reset on acquisition

    bool IsUsed() { return m_first != 0 || m_second != 0; }

    // Partial views of EPackageFile::EPackageFileAccessor methods. The actual
    // vtable remains opaque above; these declarations do not model lifetime.
    void Close();
    unsigned int Read(void *data, unsigned int size);
    unsigned int Seek(unsigned int offset, int origin);
    unsigned int Size();
};

struct PackageReadSource {
    char opaque0[0x20];
    DiagnosticFile *m_file;
};

struct PackageReadEntry {
    char opaque0[8];
    unsigned int m_offset;
    unsigned int m_size;
};

// 0x006aabb0. Unsigned wrap and past-end subtraction are native behavior.
unsigned int ReadPackedData(PackageReadSource *package, PackageReadEntry *entry,
                           unsigned int position, void *data, unsigned int size)
{
    DiagnosticFile *file = package->m_file;
    if (position + size > entry->m_size)
        size = entry->m_size - position;
    file->Seek(entry->m_offset + position, 0);
    return file->Read(data, size);
}

void PackedFileSlot::Close()
{
    m_first = 0;
    m_second = 0;
}

unsigned int PackedFileSlot::Read(void *data, unsigned int size)
{
    unsigned int result = ReadPackedData((PackageReadSource *)m_first,
        (PackageReadEntry *)m_second, m_position, data, size);
    m_position += result;
    return result;
}

unsigned int PackedFileSlot::Seek(unsigned int offset, int origin)
{
    if (origin == 1) {
        m_position += offset;
        return m_position;
    } else if (origin == 0) {
        m_position = offset;
        return m_position;
    }
    return 0xffffffff;
}

unsigned int PackedFileSlot::Size()
{
    return ((PackageReadEntry *)m_second)->m_size;
}

extern PackedFileSlot g_packedFileSlots[4]; // 0x00795c80; initialization unresolved
DiagnosticFile *OpenNativeFile(const char *name, unsigned int options); // 0x006a8180

DiagnosticFile *AcquirePackedFile(void *first, void *second)
{
    for (unsigned int i = 0; i < 4; i++) {
        PackedFileSlot *file = &g_packedFileSlots[i];
        if (!file->IsUsed()) {
            PackedFileSlot *result = &g_packedFileSlots[i];
            result->m_first = first;
            result->m_second = second;
            result->m_position = 0;
            return (DiagnosticFile *)result;
        }
    }
    return 0;
}

DiagnosticFile *EFileManager::Open(const char *name, unsigned int options, int mode)
{
    if (!name)
        return 0;
    if (options & 0xffff80ff)
        return 0;
    if (mode == 0) {
        if (m_entryCount == 0)
            mode = 9;
        else
            mode = options & 0xb00 ? 4 : 1;
    }
    // Local spellings preserve RTM stack allocation, not original names:
    // resolvedPath/second are opaque resolver outputs; first is its path result.
    void *resolvedPath = 0;
    void *second = 0;
    const char *first = ResolvePath(name, mode, &resolvedPath, &second);
    if (resolvedPath && second) {
        return AcquirePackedFile(resolvedPath, second);
    } else {
        if (!first)
            return 0;
        return OpenNativeFile(first, options);
    }
}
