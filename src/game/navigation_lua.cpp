// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class ENavMesh {
public:
    void DumpInfo();
};

extern bool g_groundNav;
extern bool g_straightNav;
extern bool g_navMeshSelected;
extern int g_selectedNavMesh;
extern int g_selectedNavGroup;


LUA_FUNCTION(SetGroundNav)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_groundNav = on;
    return 0;
}

LUA_FUNCTION_TODO(FindPathToEntity)

LUA_FUNCTION_TODO(FindPathToPosition)

LUA_FUNCTION_TODO(IsPositionOnPath)

LUA_FUNCTION_TODO(IsEntityOnPath)

LUA_FUNCTION_TODO(IsPathToEntitySolid)

LUA_FUNCTION_TODO(IsPathToPositionSolid)

LUA_FUNCTION_TODO(GetPathInfo)

LUA_FUNCTION_TODO(SlerpVectors)

LUA_FUNCTION_TODO(RegisterNavLink)

LUA_FUNCTION_TODO(ConnectNavLinks)

LUA_FUNCTION_TODO(DisconnectNavLink)

LUA_FUNCTION(SetNavGroup)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    unsigned int group = GetNumberArg(L, 2, false);
    if (group > 7)
        group = 7;
    ent->GetPhysics()->SetNavGroup(group);
    return 0;
}

LUA_FUNCTION(SelectNavMesh)
{
    int mesh;
    if (lua_gettop(L) < 1) {
        g_navMeshSelected = false;
        g_selectedNavMesh = 0xff;
        return 0;
    }
    mesh = (int)lua_tonumber(L, 1);
    if (mesh < 0) {
        g_navMeshSelected = false;
        g_selectedNavMesh = 0xff;
        return 0;
    }
    g_navMeshSelected = true;
    g_selectedNavMesh = mesh;
    return 0;
}

LUA_FUNCTION(SelectNavGroup)
{
    int group;
    if (lua_gettop(L) < 1) {
        g_selectedNavGroup = 0;
        return 0;
    }
    group = (int)lua_tonumber(L, 1);
    if (group < 0)
        group = 0;
    g_selectedNavGroup = group;
    return 0;
}

LUA_FUNCTION_TODO(RegisterNavObstacle)

LUA_FUNCTION_TODO(DeregisterNavObstacle)

LUA_FUNCTION_TODO(ListNavObstacles)

LUA_FUNCTION_TODO(RenderReachable)

LUA_FUNCTION(DumpNavMeshInfo)
{
    ELevel *level = g_game->GetLevelManager()->m_level;
    ENavMesh *mesh = level->GetNavMesh();
    if (mesh)
        mesh->DumpInfo();
    return 0;
}

LUA_FUNCTION(StraightNav)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool bOn2 = GetNumberArg(L, 1, false) != 0;
    g_straightNav = bOn2;
    return 0;
}
