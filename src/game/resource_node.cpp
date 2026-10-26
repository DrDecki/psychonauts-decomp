// Resource owner propagation; class and member names are provisional.
#include "engine.h"

struct ResourceLink {
    char pad0[8];
    void *m_owner;                    // +0x08
    unsigned int m_index : 8;
    unsigned int m_otherFlags : 24;
    char pad10[0x1c];
};

class ResourceNode {
public:
    void SetOwner(void *owner);

    char pad0[0x4c];
    TArray<ResourceNode> m_children;  // +0x4c
    TArray<ResourceLink> m_links;     // +0x58
    void *m_owner;                    // +0x64
    char pad68[0x18];
};

void ResourceNode::SetOwner(void *owner)
{
    m_owner = owner;
    for (unsigned int i = 0; i < m_children.m_count; i++) {
        ResourceNode *node = &m_children[i];
        node->SetOwner(owner);
        for (unsigned int j = 0; j < node->m_links.m_count; j++) {
            ResourceLink *link = &node->m_links[j];
            link->m_owner = owner;
            link->m_index = i;
        }
    }
}
