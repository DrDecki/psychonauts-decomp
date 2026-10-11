// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class EMMGravityEntity : public EEntity {
public:
    void Register(EEntity *ent);
    void Unregister(EEntity *ent);
    void DoWork();

    static EClassInfo s_class;
};


LUA_FUNCTION(MMGravityEntity_Register)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMMGravityEntity *gravityEnt = (EMMGravityEntity *)GetEntityArg(L, 1, false, &EMMGravityEntity::s_class);
    if (!gravityEnt)
        return 0;
    EEntity *other = GetEntityArg(L, 2, false, 0);
    if (!other)
        return 0;
    gravityEnt->Register(other);
    return 0;
}

LUA_FUNCTION(MMGravityEntity_Unregister)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EMMGravityEntity *gravityEnt = (EMMGravityEntity *)GetEntityArg(L, 1, false, &EMMGravityEntity::s_class);
    if (!gravityEnt)
        return 0;
    EEntity *other = GetEntityArg(L, 2, false, 0);
    if (!other)
        return 0;
    gravityEnt->Unregister(other);
    return 0;
}

LUA_FUNCTION(MMGravityEntity_DoWork)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EMMGravityEntity *gravityEnt = (EMMGravityEntity *)GetEntityArg(L, 1, false, &EMMGravityEntity::s_class);
    if (!gravityEnt)
        return 0;
    gravityEnt->DoWork();
    return 0;
}
