// File string allocation helpers; names other than StrDup are provisional.
#include "engine.h"

char *StrDup(const char *s, int category)
{
    if (!s)
        return 0;
    int len = strlen(s);
    char *p = (char *)MemAlloc(len + 1, category);
    memcpy(p, s, len + 1);
    return p;
}

char *CopyFileName(const char *s, int category)
{
    return StrDup(s, category);
}

void FreeFileName(char *block, int category, int flags)
{
    MemFree(block);
}
