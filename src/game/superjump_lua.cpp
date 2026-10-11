// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

extern bool g_superJump;


LUA_FUNCTION(ToggleSuperJump)
{
    g_superJump = !g_superJump;
    return 0;
}
