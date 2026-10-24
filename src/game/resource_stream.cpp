// File-backed resource input; stream class names are confirmed by RTTI.
#include "diagnostic.h"
#include "engine.h"
#include <string.h>

class EStream {
public:
    EStream() : m_flags(0), m_error(false) {}
    virtual ~EStream() {}
    void operator delete(void *block) { _aligned_free(block); }
    virtual const char *GetName() { return 0; }
    virtual void Read(void *data, unsigned int size) = 0;

    bool IsReading() { return m_flags == 0; }
    bool IsWriting() { return m_flags == 1; }
    void ReadString(char *data, int capacity);
    void ReadString2b(char *data, int capacity);
    char *ReadAllocatedString(int category);
    template <class T> void Transfer(T *data, int count)
    {
        Read(data, sizeof(T) * count);
    }

    unsigned int m_flags;          // +0x04
    unsigned int m_unknown8;
    bool m_error;                  // +0x0c
};

void EStream::ReadString(char *data, int capacity)
{
    int length;
    if (IsWriting()) {
        int size = strlen(data);
        length = size + 1;
    }
    Read(&length, 4);
    if (length && length < capacity) {
        Transfer(data, length);
    } else {
        Transfer((char *)0, length);
        if (IsReading())
            *data = 0;
    }
}

void EStream::ReadString2b(char *data, int capacity)
{
    unsigned short length;
    if (IsWriting()) {
        int size = strlen(data);
        length = size + 1;
    }
    Read(&length, 2);
    if (length && length < capacity) {
        Transfer(data, length);
    } else {
        Transfer((char *)0, length);
        if (IsReading())
            *data = 0;
    }
}

class EFileStream : public EStream {
public:
    EFileStream(DiagnosticFile *file, int mode) : m_file(file), m_owned(mode == 1) {}
    ~EFileStream();
    const char *GetName();

    DiagnosticFile *m_file;         // +0x10
    bool m_owned;                  // +0x14
};

class EFileReader : public EFileStream {
public:
    EFileReader(DiagnosticFile *file, int mode) : EFileStream(file, mode) {}
    void Read(void *data, unsigned int size);
    void ReadBytes(void *data, unsigned int size);
};

const char *EFileStream::GetName()
{
    const char *name = m_file->m_name;
    return name;
}

EFileStream::~EFileStream()
{
    if (m_owned) {
        if (m_file)
            m_file->Close();
        m_file = 0;
    }
}

void EFileReader::Read(void *data, unsigned int size)
{
    ReadBytes(data, size);
}

void EFileReader::ReadBytes(void *data, unsigned int size)
{
    if (data) {
        if (m_file->Read(data, size) != size)
            m_error = true;
    } else {
        m_file->Seek(size, 1);
    }
}

class MeshResourceReader {
public:
    void *LoadFile(const char *name, int flags);
    void *ReadResource(EFileReader *reader, int flags);
};

void *MeshResourceReader::LoadFile(const char *name, int flags)
{
    DiagnosticFile *file = g_diagnosticFileFactory->Open(name, 0x400, 0);
    if (!file)
        return 0;
    EFileReader reader(file, 1);
    return ReadResource(&reader, flags);
}

// Pool headers and list names are provisional; offsets come from native calls.
struct ResourceListPool {
    ResourceListPool *m_next;
    char opaque4[4];
    void Release();
};

ResourceListPool *AllocateResourceListPool(ResourceListPool **head,
                                         unsigned int count, unsigned int stride)
{
    void *pool = _aligned_malloc(count * stride + 8, 16);
    ResourceListPool *memory = (ResourceListPool *)pool;
    memory->m_next = *head;
    *head = memory;
    return memory;
}

void ResourceListPool::Release()
{
    ResourceListPool *pool = this;
    while (pool) {
        ResourceListPool *block = pool;
        ResourceListPool *next = pool->m_next;
        MemFree(block);
        pool = next;
    }
}

struct ResourceListNode {
    ResourceListNode *m_prev;
    ResourceListNode *m_next;
    void *m_data;
};

class ResourceList {
public:
    void AppendList(const ResourceList *source);
    void PrependList(const ResourceList *source);
    void RefillNodes();
    ResourceListNode *InsertBefore(ResourceListNode *position, void *data);
    void RemoveNode(ResourceListNode *position);
    void *PopFront();
    ResourceListNode *GetFromBack(unsigned int index);
    void ReleaseNode(ResourceListNode *node);
    void Clear();
    ResourceListNode *AllocateNode(ResourceListNode *next, ResourceListNode *prev);
    ResourceListNode *PushBack(void *data);
    ResourceListNode *PushFront(void *data);
    void *PopBack();
    ResourceListNode *InsertAfter(ResourceListNode *position, void *data);

    unsigned int m_count;
    ResourceListPool *m_pool;      // +0x04
    unsigned int m_growth;         // +0x08
    ResourceListNode *m_free;      // +0x0c
    ResourceListNode *m_last;       // +0x10
    ResourceListNode *m_first;      // +0x14
};

void *ResourceList::PopFront()
{
    ResourceListNode *first = m_first;
    void *data = first->m_data;
    m_first = first->m_next;
    if (m_first)
        m_first->m_prev = 0;
    else
        m_last = 0;
    ReleaseNode(first);
    return data;
}

ResourceListNode *ResourceList::GetFromBack(unsigned int index)
{
    if (index >= m_count)
        return 0;
    ResourceListNode *node = m_last;
    while (index--)
        node = node->m_prev;
    return node;
}

ResourceListNode *ResourceList::AllocateNode(ResourceListNode *next, ResourceListNode *prev)
{
    if (!m_free) {
        ResourceListPool *pool = AllocateResourceListPool(&m_pool, m_growth, sizeof(ResourceListNode));
        ResourceListNode *freeNode = (ResourceListNode *)(pool + 1);
        freeNode += m_growth - 1;
        for (int i = m_growth - 1; i >= 0; --i, --freeNode) {
            freeNode->m_prev = m_free;
            m_free = freeNode;
        }
    }
    ResourceListNode *node = m_free;
    m_free = m_free->m_prev;
    node->m_next = next;
    node->m_prev = prev;
    ++m_count;
    node->m_data = 0;
    return node;
}

void ResourceList::ReleaseNode(ResourceListNode *node)
{
    node->m_prev = m_free;
    m_free = node;
    --m_count;
    if (!m_count)
        Clear();
}

ResourceListNode *ResourceList::PushBack(void *data)
{
    ResourceListNode *node = AllocateNode(0, m_last);
    node->m_data = data;
    if (m_last)
        m_last->m_next = node;
    else
        m_first = node;
    m_last = node;
    return node;
}

ResourceListNode *ResourceList::PushFront(void *data)
{
    ResourceListNode *node = AllocateNode(m_first, 0);
    node->m_data = data;
    if (m_first)
        m_first->m_prev = node;
    else
        m_last = node;
    m_first = node;
    return node;
}

void *ResourceList::PopBack()
{
    ResourceListNode *last = m_last;
    void *data = last->m_data;
    m_last = last->m_prev;
    if (m_last)
        m_last->m_next = 0;
    else
        m_first = 0;
    ReleaseNode(last);
    return data;
}

ResourceListNode *ResourceList::InsertAfter(ResourceListNode *position, void *data)
{
    if (!position) {
        return PushBack(data);
    }
    ResourceListNode *previous = position;
    ResourceListNode *node = AllocateNode(previous->m_next, previous);
    node->m_data = data;
    if (previous->m_next)
        previous->m_next->m_prev = node;
    else
        m_last = node;
    previous->m_next = node;
    return node;
}

ResourceListNode *ResourceList::InsertBefore(ResourceListNode *position, void *data)
{
    if (!position) {
        return PushFront(data);
    }
    ResourceListNode *previous = position;
    ResourceListNode *node = AllocateNode(previous, previous->m_prev);
    node->m_data = data;
    if (previous->m_prev)
        previous->m_prev->m_next = node;
    else
        m_first = node;
    previous->m_prev = node;
    return node;
}

void ResourceList::RemoveNode(ResourceListNode *position)
{
    ResourceListNode *node = position;
    if (node == m_last)
        m_last = node->m_prev;
    else
        node->m_next->m_prev = node->m_prev;
    if (node == m_first)
        m_first = node->m_next;
    else
        node->m_prev->m_next = node->m_next;
    ReleaseNode(node);
}

// Copy payload pointers, preserving source order and pre-insertion traversal.
void ResourceList::AppendList(const ResourceList *source)
{
    ResourceListNode *node = source->m_first;
    ResourceListNode *position = node;
    while (position) {
        ResourceListNode *current = position;
        position = position->m_next;
        PushBack(current->m_data);
    }
}

void ResourceList::PrependList(const ResourceList *source)
{
    ResourceListNode *node = source->m_last;
    ResourceListNode *position = node;
    while (position) {
        ResourceListNode *current = position;
        position = position->m_prev;
        PushFront(current->m_data);
    }
}

void ResourceList::RefillNodes()
{
    if (!m_free) {
        ResourceListPool *pool = AllocateResourceListPool(&m_pool, m_growth, sizeof(ResourceListNode));
        ResourceListNode *freeNode = (ResourceListNode *)(pool + 1);
        freeNode += m_growth - 1;
        for (int i = m_growth - 1; i >= 0; --i, --freeNode) {
            freeNode->m_prev = m_free;
            m_free = freeNode;
        }
    }
}

// The category is forwarded into MemAlloc, whose RTM inline body ignores it.
char *EStream::ReadAllocatedString(int category)
{
    int length;
    char *data;
    Read(&length, 4);
    if (length <= 1) {
        Transfer((char *)0, length);
        return 0;
    } else {
        data = (char *)MemAlloc(length, category);
        Transfer(data, length);
        return data;
    }
}
