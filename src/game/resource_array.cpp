// Storage transfer used by resource dispatch; names remain provisional.
#include "engine.h"

void SwapResourceArrays(EArrayBase *first, EArrayBase *second)
{
    void *oldData = first->m_data;
    first->m_data = second->m_data;
    second->m_data = oldData;

    unsigned int temporary = first->m_category;
    first->m_category = second->m_category;
    second->m_category = temporary;

    bool oldFlag = first->m_flag;
    first->m_flag = second->m_flag;
    second->m_flag = oldFlag;

    temporary = first->m_count;
    first->m_count = second->m_count;
    second->m_count = temporary;

    unsigned int oldCapacity = first->m_capacity;
    first->m_capacity = second->m_capacity;
    second->m_capacity = oldCapacity;
}
