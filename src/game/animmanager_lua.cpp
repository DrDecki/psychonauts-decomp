// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class EAnimManager {
public:
    void SetCacheSize(unsigned int bytes);
    void PurgeCache(bool all);
    void SqueezeCache(bool all);
};

extern EAnimManager *g_animManager;


LUA_FUNCTION(AnimManager_SqueezeCache)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_animManager->SqueezeCache(all);
    return 0;
}

LUA_FUNCTION(AnimManager_PurgeCache)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    g_animManager->PurgeCache(all);
    return 0;
}

LUA_FUNCTION(AnimManager_SetCacheSize)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int kb = GetNumberArg(L, 1, false);
    if (kb > 128 && kb < 65536)
        g_animManager->SetCacheSize(kb * 1024);
    return 0;
}
