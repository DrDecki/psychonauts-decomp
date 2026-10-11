// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"


LUA_FUNCTION(EnableWater)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool enable = GetNumberArg(L, 1, false) != 0;
    g_game->m_waterEnabled = enable;
    return 0;
}

LUA_FUNCTION(DisplayReflectionTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int index = n >= 1 ? (int)GetNumberArg(L, 1, true) : 0;
    EResource *pTex2;
    if (index)
        pTex2 = g_game->m_resources->GetById(index);
    else
        return 0;
    if (pTex2) {
        void *pHandle1 = pTex2->m_handle;
        Vector2 pos(0.0f, 0.0f);
        int ret = g_game->m_overlay->Display(pHandle1, pos, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0, 1, g_white);
        lua_pushint(L, ret);
    } else {
        return 0;
    }
    return 1;
}

LUA_FUNCTION(SetReflectionScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevelManager()->m_level;
    pLevel->m_reflectionScale = fScale * 0.002f;
    return 0;
}

LUA_FUNCTION(SetRefractionScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevelManager()->m_level;
    pLevel->m_refractionScale = fScale * 0.002f;
    return 0;
}

LUA_FUNCTION(SetReflectionBumpScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevelManager()->m_level;
    pLevel->m_reflectionBumpScale = fScale * 0.05f;
    return 0;
}

LUA_FUNCTION(SetRefractionBumpScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevelManager()->m_level;
    pLevel->m_refractionBumpScale = fScale * 0.05f;
    return 0;
}

LUA_FUNCTION(SetFresnelBumpScale)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float fScale = GetNumberArg(L, 1, false);
    ELevel *pLevel = g_game->GetLevelManager()->m_level;
    pLevel->m_fresnelBumpScale = fScale * 0.5f;
    return 0;
}

LUA_FUNCTION(SetWaterRefractionTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EResource *tex = (EResource *)GetResourceArg(L, 1);
    if (tex) {
        void *handle = tex->m_handle;
        EWater *water = g_game->m_water;
        water->m_refractionTexture = handle;
    }
    return 0;
}

LUA_FUNCTION(SetWaterBumpTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EResource *tex = (EResource *)GetResourceArg(L, 1);
    if (tex) {
        void *handle = tex->m_handle;
        EWater *water = g_game->m_water;
        water->m_bumpTexture = handle;
    }
    return 0;
}

LUA_FUNCTION(SetWaterFresnelTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EResource *tex = (EResource *)GetResourceArg(L, 1);
    if (tex) {
        void *handle = tex->m_handle;
        EWater *water = g_game->m_water;
        water->m_fresnelTexture = handle;
    }
    return 0;
}

LUA_FUNCTION_TODO(SetupWaterTextures)

LUA_FUNCTION_TODO(TestTableArray)
