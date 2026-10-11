// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

class ETrampolineEntity {
public:
    void OnBeginLevel();
    void OnEntered(ETriggerBase *trigger);
    void SetBouncer(EEntity *bouncer);

    static EClassInfo s_class;
};


LUA_FUNCTION(Trampoline_onBeginLevel)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETrampolineEntity *trampoline = (ETrampolineEntity *)GetEntityArg(L, 1, false, &ETrampolineEntity::s_class);
    if (!trampoline)
        return 0;
    trampoline->OnBeginLevel();
    return 0;
}

LUA_FUNCTION(Trampoline_onEntered)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETrampolineEntity *trampoline = (ETrampolineEntity *)GetEntityArg(L, 1, false, &ETrampolineEntity::s_class);
    if (!trampoline)
        return 0;
    ETriggerBase *trg = GetTriggerArg(L, 2, false);
    if (!trg)
        return 0;
    trampoline->OnEntered(trg);
    return 0;
}

LUA_FUNCTION(Trampoline_onExited)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETrampolineEntity *trampoline = (ETrampolineEntity *)GetEntityArg(L, 1, false, &ETrampolineEntity::s_class);
    if (!trampoline)
        return 0;
    ETriggerBase *trg = GetTriggerArg(L, 2, false);
    if (!trg)
        return 0;
    return 0;
}

LUA_FUNCTION(Trampoline_ClearBouncer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETrampolineEntity *trampoline = (ETrampolineEntity *)GetEntityArg(L, 1, false, &ETrampolineEntity::s_class);
    if (!trampoline)
        return 0;
    trampoline->SetBouncer(0);
    return 0;
}
