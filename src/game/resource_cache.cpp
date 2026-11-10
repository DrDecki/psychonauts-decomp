// Resource-cache lookup view; names and complete ownership remain provisional.
#include "engine.h"

struct ResourceCacheHandle {
    union {
        unsigned int m_raw;
        struct {
            unsigned int m_index : 31;
            unsigned int m_flag : 1;
        };
    };
};

void FreeFileName(char *block, int category, int flags);

struct ResourceCacheEntry {
    void operator delete(void *block) { _aligned_free(block); }
    ~ResourceCacheEntry() { FreeFileName(m_name, 0x1a, 0); m_name = 0; }
    char *m_name;
    void *m_value;
    unsigned int m_next;
};

unsigned int ResourceCacheHash(const unsigned char *name);

// Observed sparse-array storage followed by hash buckets; names are provisional.
class ResourceCache {
public:
    ResourceCacheHandle *Find(const char *name, unsigned int *bucket);
    void *Lookup(const char *name, void *fallback);

    bool TryGet(const char *name, void **output);
    void *Remove(const char *name, void *fallback);
    void Unlink(unsigned int handle, unsigned int bucket);
    void Release(unsigned int handle)
    {
        ResourceCacheEntry *pEntry;
        unsigned int index = handle & 0x7fffffff;
        pEntry = &m_entries[index];
        pEntry->~ResourceCacheEntry();
        *(unsigned int *)pEntry = ((m_freeHead & 0x7fffffff) << 1) | 1;
        m_freeHead = index;
        --m_live;
    }

    unsigned int m_category : 5;
    unsigned int m_count : 27;
    unsigned int m_storageFlag : 1;
    unsigned int m_capacity : 31;
    ResourceCacheEntry *m_entries; // +0x08; 12-byte entries
    unsigned int m_live;
    unsigned int m_freeHead;
    unsigned short m_generation;
    ResourceCacheHandle *m_buckets; // +0x18
    unsigned int m_bucketCount;
};

void *ResourceCache::Lookup(const char *name, void *fallback)
{
    ResourceCacheHandle *handle = Find(name, 0);
    if (!handle) {
        return fallback;
    } else {
        return m_entries[handle->m_index].m_value;
    }
}

unsigned int ResourceCacheHash(const unsigned char *name)
{
    unsigned int hash = 0;
    for (const unsigned char *cursor = name; *cursor; ++cursor)
        hash = (hash << 5) - hash + *cursor;
    return hash;
}

ResourceCacheHandle *ResourceCache::Find(const char *name, unsigned int *bucketOutput)
{
    ResourceCacheHandle *entryLink;
    unsigned int hashIndex = ResourceCacheHash((const unsigned char *)name) % m_bucketCount;
    if (bucketOutput)
        *bucketOutput = hashIndex;
    entryLink = &m_buckets[hashIndex];
    while (entryLink->m_raw != 0xffffffff) {
        ResourceCacheEntry *entry = &m_entries[entryLink->m_index];
        if (strcmp(entry->m_name, name) == 0)
            return entryLink;
        entryLink = (ResourceCacheHandle *)&entry->m_next;
    }
    return 0;
}

bool ResourceCache::TryGet(const char *name, void **output)
{
    ResourceCacheHandle *handle = Find(name, 0);
    if (!handle) {
        return false;
    } else {
        if (output)
            *output = m_entries[handle->m_index].m_value;
        return true;
    }
}

void *ResourceCache::Remove(const char *name, void *fallback)
{
    ResourceCacheHandle *handle = Find(name, 0);
    if (!handle) {
        return fallback;
    } else {
        void *result;
        ResourceCacheEntry *data;
        unsigned int index = handle->m_raw;
        data = &m_entries[index & 0x7fffffff];
        handle->m_raw = data->m_next;
        result = data->m_value;
        Release(index);
        return result;
    }
}

void ResourceCache::Unlink(unsigned int handle, unsigned int bucket)
{
    ResourceCacheHandle *link = &m_buckets[bucket];
    while (link->m_raw != handle)
        link = (ResourceCacheHandle *)&m_entries[link->m_index].m_next;
    link->m_raw = m_entries[handle & 0x7fffffff].m_next;
    Release(handle);
}
