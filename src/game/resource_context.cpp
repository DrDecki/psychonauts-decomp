// Resource-dispatch lifetime views; names are provisional.
#include "engine.h"

class ContextPool {
public:
    void Release();
};

class ContextListBase {
public:
    ContextListBase(int mode);
    ~ContextListBase();
    void Clear();
    unsigned int m_count;
    void *m_pool;
    unsigned int m_growth;
    void *m_cursor;
    void *m_last;
    void *m_first;
};

class ContextList : public ContextListBase {
public:
    ContextList() : ContextListBase(1) {}
};

ContextListBase::ContextListBase(int mode)
{
    // Original 0x006a5039 stores 10 to a local; keep it for stack/code matching.
    int growth = 10;
    m_count = 0;
    m_first = 0;
    m_last = 0;
    m_cursor = 0;
    m_pool = 0;
    m_growth = 10;
}

ContextListBase::~ContextListBase()
{
    Clear();
}

void ContextListBase::Clear()
{
    m_count = 0;
    m_first = 0;
    m_last = 0;
    m_cursor = 0;
    ((ContextPool *)m_pool)->Release();
    m_pool = 0;
}

struct ContextAssociation {
    void operator delete(void *block) { _aligned_free(block); }
    unsigned int m_value;
    struct Entry {
        ~Entry();
        char data[12];
    } m_entry;
};

template <> void TArray<ContextAssociation>::Clear()
{
    if (__is_class(ContextAssociation)) {
        for (unsigned int i = 0; i < m_count; i++)
            (*this)[i].~ContextAssociation();
    }
    Reserve(sizeof(ContextAssociation), 0);
}

struct ContextBounds {
    ContextBounds();
    float data[6];
};

void FreeFileName(char *block, int category, int flags);
char *CopyFileName(const char *name, int category);

class ResourceContext {
public:
    ResourceContext(void *manager, void *allocator);
    ~ResourceContext();
    void Cleanup();
    void SetName(const char *name);

    void *m_allocator;                 // +0x00
    ContextList m_children;            // +0x04
    ContextList m_results;             // +0x1c
    TArray<ContextAssociation> m_associations; // +0x34
    bool m_flag40;
    char *m_name;                      // +0x44
    bool m_flag48;
    ContextBounds m_bounds;            // +0x4c
    void *m_manager;                   // +0x64
    void *m_unknown68;
    ContextList m_pending;             // +0x6c
};

ResourceContext::~ResourceContext()
{
    Cleanup();
    FreeFileName(m_name, 0x1a, 1);
}

// Original-behavior lifetime hazard: an input aliasing m_name is freed before
// the copy reads it. Preserve the original sequence; callers must avoid aliases.
void ResourceContext::SetName(const char *name)
{
    if (m_name)
        FreeFileName(m_name, 0x1a, 1);
    m_name = CopyFileName(name, 0x1a);
}
