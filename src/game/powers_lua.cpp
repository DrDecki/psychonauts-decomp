// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

#include "classes.h"
#include <math.h>

// tuning values the debug functions at the end of this file change
extern int g_LR;
extern bool g_LAmoeba;
extern bool g_LP;
extern bool g_tkcarry;
extern int g_HLP1;
extern int g_HLP2;
extern float g_LA[3];
extern float g_LF[3];

class EPsiBlastTarget {
public:
    static EClassInfo s_class;
};

static const float kTKTurnSpeed = 80.0f;
static const float kTKEpsilon = 0.00001f;
void ScriptError(const char *fmt, ...);


LUA_FUNCTION(InterruptPowers)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : false;
    ResetPlayerInput(all);
    return 0;
}

LUA_FUNCTION(EnablePower)
{
    if (lua_gettop(L) < 1 || lua_gettop(L) > 2)
        return 0;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    int power = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    bool bQuiet = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (power > -1 && power <= 8) {
        if (power == 8) {
            for (int i = 0; i < g_game->m_player->GetNumPowers(); i++)
                g_game->m_player->EnablePower(i, false);
        } else
            g_game->m_player->EnablePower(power, bQuiet);
        bool curAll = true;
        for (int j = 0; j < g_game->m_player->GetNumPowers(); j++) {
            if (!g_game->GetPlayer()->IsPowerEnabled(j)) {
                curAll = false;
                break;
            }
        }
        if (curAll)
            g_game->UnlockAchievement("ACH_ALL_POWERS");
    }
    return 0;
}

LUA_FUNCTION(DisablePower)
{
    if (lua_gettop(L) < 1 || lua_gettop(L) > 1)
        return 0;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int power = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (power > -1 && power <= 8) {
        if (power == 8) {
            for (int i = 0; i < g_game->m_player->GetNumPowers(); i++)
                g_game->m_player->DisablePower(i);
        } else
            g_game->m_player->DisablePower(power);
    }
    return 0;
}

LUA_FUNCTION(CoolDownPower)
{
    if (lua_gettop(L) < 2 || lua_gettop(L) > 2)
        return 0;
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int power = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    bool on = GetNumberArg(L, 2, false) != 0;
    if (power > -1 && power < 8)
        g_game->GetPlayer()->SetPowerCoolDown(power, on);
    return 0;
}

LUA_FUNCTION(IsPowerEnabled)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int power = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (g_game->GetPlayer()->IsPowerEnabled(power))
        lua_pushnumber(L, 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION(MapPowerToButton)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int power = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    int button = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (power == -1) {
        g_game->GetPlayer()->MapButton(button, power);
        g_game->m_hudElements.m_powers->ClearButton(button);
        lua_pushnumber(L, 1);
    } else if (power >= 0 && power < 8 && (g_game->GetPlayer()->IsPowerEnabled(power) || g_game->m_player->CanMapPower(power))) {
        g_game->GetPlayer()->MapButton(button, power);
        g_game->m_hudElements.m_powers->SetButton(button);
        lua_pushnumber(L, 1);
    } else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION(GetMappedPower)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int button = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    int nPower = g_game->m_player->m_buttons[button].m_power;
    lua_pushint(L, nPower);
    return 1;
}

LUA_FUNCTION(LaunchPsiBlast)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 8, 10))
        return 0;
    EEntity *pOwnerValue = GetEntityArg(L, 1, true, 0);
    Vector3 aPosVal(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    Vector3 dir(GetNumberArg(L, 5, false), GetNumberArg(L, 6, false), GetNumberArg(L, 7, false));
    float aSpeedObj = GetNumberArg(L, 8, false);
    bool homing = n >= 9 ? GetNumberArg(L, 9, true) != 0 : false;
    float delay = n >= 10 ? GetNumberArg(L, 10, true) : 0.0f;
    EPsiBlastManager *blasts = g_game->m_psiBlasts;
    int curId = blasts->Launch(aPosVal, dir, pOwnerValue ? (int)pOwnerValue : -1, aSpeedObj, homing, delay);
    lua_pushint(L, curId);
    return 1;
}

LUA_FUNCTION(ActivateLevitation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int frames = n >= 1 ? (int)GetNumberArg(L, 1, true) : -1;
    g_game->m_player->ActivateLevitation(frames);
    return 0;
}

LUA_FUNCTION(GetPlayerPsiBlastCharge)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    float charge = g_game->m_psiBlasts->GetCharge(-1);
    lua_pushnumber(L, charge);
    return 1;
}

LUA_FUNCTION(GetPsiBlastCharge)
{
    if (lua_gettop(L) < 1 || lua_gettop(L) > 1)
        return 0;
    int nId = (int)lua_topointer(L, 1);
    EScriptObject *so = GetScriptObjectArg(L, nId, false, false);
    EEntity *ent = so->GetEntity();
    if (!ent->IsType(4)) {
        ScriptError("GetPsiBlastCharge: Entity is not a psi blast");
        return 0;
    }
    EPsiBlast *blast = (EPsiBlast *)ent;
    float charge = blast->GetCharge();
    lua_pushnumber(L, charge);
    return 1;
}

LUA_FUNCTION(DetonatePsiBlast)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, true, 0);
    Vector3 newPosVal(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    EPsiBlastManager *blasts = g_game->m_psiBlasts;
    blasts->Detonate(newPosVal, ent ? (int)ent : -1);
    return 0;
}

LUA_FUNCTION(FirePsiBlast)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 3))
        return 0;
    EEntity *curTarget = n >= 1 ? GetEntityArg(L, 1, true, 0) : 0;
    int chain = n >= 3 ? (int)GetNumberArg(L, 3, true) : 3;
    if (curTarget && curTarget->IsA(&EPsiBlastTarget::s_class)) {
        float range = 2000.0f;
        EEntity *ignore[10];
        int super = 0;
        unsigned int count = lua_getn(L, 2);
        int numIgnore = 0;
        count = Min(count, 10);
        for (unsigned int i = 1; i <= count; i++) {
            lua_rawgeti(L, 2, i);
            EScriptObject *so = GetScriptObjectArg(L, -1, true, true);
            if (so && so->m_entity) {
                ignore[numIgnore] = so->m_entity;
                numIgnore++;
            }
        }
        GetScriptManager()->GetNumber(g_game->GetPlayerControl()->GetScriptObject(), "PsiBlastRange", &range);
        g_game->GetPlayerControl()->GetScriptValue("bSuperChainBlast", &super);
        g_game->m_psiBlasts->Fire(curTarget, range, ignore, numIgnore, chain - count);
    } else {
        EPlayer *player = g_game->m_player;
        if (player)
            player->CancelPsiBlast();
    }
    return 0;
}

LUA_FUNCTION(SetPsiBlastColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    Color theColor = GetColorArg(L, 2, false);
    EPsiBlastManager *pBlasts = g_game->m_psiBlasts;
    pBlasts->SetColor(index, theColor);
    return 0;
}

LUA_FUNCTION(SetPsiBlastTexture)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    void **myTextureInfo = (void **)GetResourceArg(L, 2);
    EPsiBlastManager *blasts = g_game->m_psiBlasts;
    blasts->SetTexture(index, *myTextureInfo);
    return 0;
}

LUA_FUNCTION(SetPsiBlastAmmo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int ammo = (int)GetNumberArg(L, 1, false);
    EPsiBlastManager *blasts = g_game->m_psiBlasts;
    blasts->SetAmmo(ammo);
    return 0;
}

LUA_FUNCTION(AdjustPsiBlastAmmo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int myAmount = (int)GetNumberArg(L, 1, false);
    EPsiBlastManager *blasts = g_game->m_psiBlasts;
    blasts->AdjustAmmo(myAmount);
    return 0;
}

LUA_FUNCTION(GetPsiBlastAmmo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EPsiBlastManager *blasts = g_game->m_psiBlasts;
    int ammo = blasts->m_ammo;
    lua_pushint(L, ammo);
    return 1;
}

LUA_FUNCTION(FirestartingPunchBalloon)
{
    g_game->m_firestarting->PunchBalloon();
    return 0;
}

LUA_FUNCTION(FirestartingSlamGround)
{
    g_game->m_firestarting->SlamGround();
    return 0;
}

LUA_FUNCTION(FirestartingIgnited)
{
    g_game->m_player->FirestartingIgnited();
    return 0;
}

LUA_FUNCTION(FirestartingDetonated)
{
    g_game->m_player->FirestartingDetonated();
    return 0;
}

LUA_FUNCTION(StartFireBlast)
{
    if (lua_gettop(L) < 3 || lua_gettop(L) > 3)
        return 0;
    EZeroVector3 pos;
    pos.x = lua_tonumber(L, 1);
    pos.y = lua_tonumber(L, 2);
    pos.z = lua_tonumber(L, 3);
    g_game->m_firestarting->StartFireBlast(pos, 1);
    return 0;
}

LUA_FUNCTION(TelekinesisPunchBalloon)
{
    g_game->GetTelekinesis()->GetClaw()->PunchBalloon();
    return 0;
}

LUA_FUNCTION(TelekinesisOpenClaw)
{
    g_game->GetTelekinesis()->GetClaw()->OpenClaw();
    return 0;
}

LUA_FUNCTION(TelekinesisHideBubble)
{
    g_game->GetTelekinesis()->GetClaw()->HideBubble();
    return 0;
}

LUA_FUNCTION(SetTKSource)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 3)
        return 0;
    if (lua_gettop(L) < 3) {
        g_game->GetTelekinesis()->GetClaw()->UseDefaultSource();
    } else {
        EZeroVector3 thePos;
        thePos.x = lua_tonumber(L, 1);
        thePos.y = lua_tonumber(L, 2);
        thePos.z = lua_tonumber(L, 3);
        g_game->GetTelekinesis()->GetClaw()->SetSource(thePos);
        int nPrevious = g_game->GetCurrentControlHandlerId();
        g_game->SetControlHandler(8, 0);
        ((ETKControlHandler *)g_game->GetActionItems())->m_holding = false;
        ((ETKControlHandler *)g_game->GetActionItems())->m_previousHandler = nPrevious;
        g_game->GetTelekinesis()->GetClaw()->PunchBalloon();
    }
    return 0;
}

LUA_FUNCTION(RemoveTKTarget)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, false, 0);
    if (!ent)
        return 0;
    g_game->GetTelekinesis()->GetClaw()->RemoveTarget(ent, true);
    return 0;
}

LUA_FUNCTION(InvisibilityHopThroughLasso)
{
    g_game->m_invisibility->HopThroughLasso();
    return 0;
}

LUA_FUNCTION(SetEntityInvisibility)
{
    if (lua_gettop(L) < 2 || lua_gettop(L) > 3)
        return 0;
    int nId = (int)lua_topointer(L, 1);
    float fAmount = lua_tonumber(L, 2);
    float secs = -1.0f;
    if (lua_gettop(L) == 3)
        secs = lua_tonumber(L, 3);
    EScriptObject *so = GetScriptObjectArg(L, nId, false, false);
    if (!so)
        return 0;
    EEntity *theEnt = so->GetEntity();
    if (theEnt) {
        g_game->m_invisibility->SetTime(theEnt, secs);
        theEnt->SetInvisibility(fAmount);
    }
    return 0;
}

LUA_FUNCTION(SetPlayerInvisibility)
{
    if (lua_gettop(L) < 1 || lua_gettop(L) > 2)
        return 0;
    float fAmount = lua_tonumber(L, 1);
    float secs = -1.0f;
    if (lua_gettop(L) == 2)
        secs = lua_tonumber(L, 2);
    if (secs > 0)
        g_game->m_invisibility->SetTime(g_game->GetPlayerControl(), secs);
    g_game->GetPlayerControl()->SetInvisibility(fAmount);
    return 0;
}

LUA_FUNCTION(GetPlayerInvisibility)
{
    lua_pushnumber(L, g_game->GetPlayerControl()->GetInvisibility());
    return 1;
}

LUA_FUNCTION(GetEntityInvisibility)
{
    int nId = (int)lua_topointer(L, 1);
    EScriptObject *so = GetScriptObjectArg(L, nId, false, false);
    if (!so)
        return 0;
    EEntity *ent = so->GetEntity();
    if (!ent)
        return 0;
    lua_pushnumber(L, ent->GetInvisibility());
    return 1;
}

LUA_FUNCTION(GetDartInvisibility)
{
    lua_pushnumber(L, g_game->GetPlayerControl()->GetInvisibility());
    return 1;
}

LUA_FUNCTION(GetThoughtBubbleState)
{
    int state = 0;
    if (g_game->GetThoughtBubble()->GetPhysics()->HasFlag(1))
        state = 1;
    lua_pushint(L, state);
    return 1;
}

LUA_FUNCTION(OpenThoughtBubble)
{
    EActionItems *items = g_game->m_actionItems;
    bool ok = items->OpenThoughtBubble(g_game->m_player->GetBubbleItem(), 0);
    lua_pushint(L, ok != 0);
    return 1;
}

LUA_FUNCTION(SetClairvoyanceTarget)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 3))
        return 0;
    EEntity *ent = n >= 1 ? GetEntityArg(L, 1, true, 0) : 0;
    bool activate = n >= 2 ? GetNumberArg(L, 2, true) != 0 : true;
    bool bFade = n >= 3 ? GetNumberArg(L, 3, true) != 0 : true;
    if (ent && ((EEntityVirtuals *)ent)->HasMesh()) {
        g_game->SetClairvoyanceTarget(ent, bFade);
        if (activate && g_game->GetCurrentControlHandlerId() == 1) {
            g_game->m_clairvoyanceOn = true;
            g_game->m_clairvoyanceOff = false;
            g_game->m_player->StartClairvoyance();
        }
    } else {
        g_game->GetCurrentControlHandlerId();
        g_game->SetClairvoyanceTarget(0, bFade);
    }
    return 0;
}

LUA_FUNCTION(SetClairvoyanceFilter)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int iFilter = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    g_game->SetClairvoyanceFilter(iFilter);
    return 0;
}

LUA_FUNCTION(SetClairvoyanceMask)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void **theTexture = (void **)GetResourceArg(L, 1);
    g_game->SetClairvoyanceMask(theTexture ? *theTexture : 0);
    return 0;
}

LUA_FUNCTION(SetClairvoyanceNightvision)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    if (on)
        g_game->SetClairvoyanceFilter(1);
    else
        g_game->SetClairvoyanceFilter(0);
    return 0;
}

LUA_FUNCTION_TODO(CreateTKClaw)

LUA_FUNCTION(CloseTKClaw)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETKClaw *claw = (ETKClaw *)GetEntityArg(L, 1, false, &ETKClaw::s_class);
    if (!claw)
        return 0;
    int state = claw->m_state;
    if (state == 5) {
        if (g_game->m_sound)
            g_game->m_sound->Play(g_game->m_clawCloseSound, 0, 0);
        claw->SetState(3);
    }
    return 0;
}

LUA_FUNCTION(OpenTKClaw)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETKClaw *claw = (ETKClaw *)GetEntityArg(L, 1, false, &ETKClaw::s_class);
    if (!claw)
        return 0;
    int state = claw->m_state;
    if (state == 6) {
        if (g_game->m_sound)
            g_game->m_sound->Play(g_game->m_clawOpenSound, 0, 0);
        claw->SetState(5);
        claw->SetTarget(0);
    }
    return 0;
}

LUA_FUNCTION(SetTKClawTarget)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    g_game->GetTelekinesis()->GetClaw()->SetMoving(false);
    g_game->GetTelekinesis()->GetClaw()->SetTarget(ent);
    return 0;
}

LUA_FUNCTION(GetTKClawTarget)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EEntity *target = g_game->GetTelekinesis()->GetClaw()->m_target;
    PushEntity(L, target, false);
    return 1;
}

LUA_FUNCTION(SetTKClawSnapType)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETKClaw *claw = (ETKClaw *)GetEntityArg(L, 1, false, &ETKClaw::s_class);
    if (!claw)
        return 0;
    const char *type = GetStringArg(L, 2, false);
    claw->SetSnapType(type);
    return 0;
}

LUA_FUNCTION(MoveTKClawTowardPosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 5))
        return 0;
    ETKClaw *claw = (ETKClaw *)GetEntityArg(L, 1, false, &ETKClaw::s_class);
    if (!claw)
        return 0;
    Vector3 myPos(GetNumberArg(L, 2, false), GetNumberArg(L, 3, false), GetNumberArg(L, 4, false));
    float myAngleVal = n >= 5 ? GetNumberArg(L, 5, true) : 0.0f;
    float theCurrent = claw->m_angle;
    if (myAngleVal < 0)
        myAngleVal = theCurrent;
    while (myAngleVal > 360.0)
        myAngleVal -= 360.0;
    float turn = myAngleVal - theCurrent;
    if (turn < 0)
        turn += 360.0;
    float aSecs = g_game->m_frameSecs;
    turn = Min(turn, kTKTurnSpeed * aSecs) / aSecs;
    bool bArrivedVal = claw->MoveToward(myPos, turn);
    if (bArrivedVal) {
        if (fabs(myAngleVal - claw->GetAngle()) < kTKEpsilon && claw->GetVelocity().LengthSq() < kTKEpsilon)
            lua_pushnumber(L, 2);
        else
            lua_pushnumber(L, 1);
    } else
        lua_pushnumber(L, 0);
    return 1;
}

LUA_FUNCTION(EnableTKClawRendering)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    ETKClaw *claw = (ETKClaw *)GetEntityArg(L, 1, false, &ETKClaw::s_class);
    if (!claw)
        return 0;
    bool on = GetNumberArg(L, 2, false) != 0;
    claw->m_render = on;
    return 0;
}

LUA_FUNCTION(GetTKClawApparentDestination)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    Vector3 pos = g_game->GetTelekinesis()->GetClaw()->m_apparentDestination;
    PushVector(L, pos);
    return 3;
}

LUA_FUNCTION(SetThoughtBubbleSource)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    ETriggerBase *ent = GetTriggerArg(L, 1, true);
    bool activate = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    g_game->GetPlayer()->m_bubbleSource = ent;
    g_game->GetPsiBlasts()->m_source = ent;
    if (activate && g_game->GetCurrentControlHandlerId() != 1)
        g_game->SetControlHandler(1, 0);
    return 0;
}

LUA_FUNCTION(Levitation_InhibitJump)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float secs = GetNumberArg(L, 1, false);
    ((ELevitationControlHandler *)g_game->GetControlHandler(3))->InhibitJump(secs);
    return 0;
}

LUA_FUNCTION(SetPlayerConfused)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_game->m_player->SetConfused(on);
    return 0;
}

LUA_FUNCTION(SetTKOldWay)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ETKControlHandler *pHandler = (ETKControlHandler *)g_game->GetControlHandler(8);
    pHandler->m_oldWay = on;
    return 0;
}

LUA_FUNCTION(tkold)
{
    ETKControlHandler *handler = (ETKControlHandler *)g_game->GetControlHandler(8);
    handler->m_oldWay = true;
    return 0;
}

LUA_FUNCTION(tknew)
{
    ETKControlHandler *handler = (ETKControlHandler *)g_game->GetControlHandler(8);
    handler->m_oldWay = false;
    return 0;
}

LUA_FUNCTION(SetTKRequireHeldButton)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ETKControlHandler *pHandler = (ETKControlHandler *)g_game->GetControlHandler(8);
    pHandler->m_requireHeldButton = on;
    return 0;
}

LUA_FUNCTION(SetTKRequireTarget)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    ETKControlHandler *pHandler = (ETKControlHandler *)g_game->GetControlHandler(8);
    pHandler->m_requireTarget = on;
    return 0;
}

LUA_FUNCTION(tkcarry)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_tkcarry = on;
    return 0;
}

LUA_FUNCTION(newpb)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_game->GetPsiBlasts()->m_new = on;
    g_game->GetPlayer()->SetPowerEnabled(15, on);
    g_game->GetPlayer()->SetPowerEnabled(31, on);
    return 0;
}

LUA_FUNCTION(BubbleSelectOnRelease)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_game->GetPlayer()->m_bubbleSelectOnRelease = on;
    return 0;
}

LUA_FUNCTION(BubbleUseLeftStick)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_game->GetPlayer()->m_bubbleUseLeftStick = on;
    return 0;
}

LUA_FUNCTION(LP)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_LP = on;
    return 0;
}

LUA_FUNCTION(LAmoeba)
{
    int iN = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    g_LAmoeba = on;
    return 0;
}

LUA_FUNCTION(LR)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int nValue = (int)GetNumberArg(L, 1, false);
    g_LR = nValue;
    return 0;
}

LUA_FUNCTION(IS)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 4))
        return 0;
    float myAVal = GetNumberArg(L, 1, false);
    float b = n >= 2 ? GetNumberArg(L, 2, true) : -1.0f;
    float c = n >= 3 ? GetNumberArg(L, 3, true) : -1.0f;
    float theD = n >= 4 ? GetNumberArg(L, 4, true) : -1.0f;
    g_game->m_player->SetInvisibilityShader(myAVal, b, c, theD);
    return 0;
}

LUA_FUNCTION(LF)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float myAVal = GetNumberArg(L, 1, false);
    float b = GetNumberArg(L, 2, false);
    float c = GetNumberArg(L, 3, false);
    g_LF[0] = myAVal;
    g_LF[1] = b;
    g_LF[2] = c;
    return 0;
}

LUA_FUNCTION(LA)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float myAVal = GetNumberArg(L, 1, false);
    float b = GetNumberArg(L, 2, false);
    float c = GetNumberArg(L, 3, false);
    g_LA[0] = myAVal;
    g_LA[1] = b;
    g_LA[2] = c;
    return 0;
}

LUA_FUNCTION(FMI)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float theValue = GetNumberArg(L, 1, false);
    g_game->GetPlayer()->m_fmi = theValue;
    return 0;
}

LUA_FUNCTION(HLP)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int iA = (int)GetNumberArg(L, 1, false);
    int b = (int)GetNumberArg(L, 2, false);
    g_HLP1 = iA;
    g_HLP2 = b;
    return 0;
}
