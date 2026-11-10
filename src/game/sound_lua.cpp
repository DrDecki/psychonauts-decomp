// Script functions, file name made up.
// Partial native sound bindings; remaining bodies are marked TODO.

#include "engine.h"
#include "luafunc.h"
#include "classes.h"
#include <stdio.h>

void StrToLower(char *s);
#include "diagnostic.h"

void EEntity::StopSounds()
{
    if (m_soundInstance)
        g_game->m_sound->StopOnInstance(m_soundInstance);
}

// Provisional helper name. Observed at 0x00597d60 in the hash-gated Steam PE.
// Sound handles retain their raw Lua numeric bits; they are not float casts.
static void *GetSoundArg(lua_State *L, int idx)
{
    if (((idx & 0x80000000) ? -idx : idx) < 20) {
        if (lua_isrealnumber(L, idx)) {
            void *handle = lua_torawvalue(L, idx);
            return handle;
        } else if (lua_isstring(L, idx)) {
            const char *name = lua_tostring(L, idx);
            LuaCallSite(L);
            void *result = g_game->m_sound->FindSound(name);
            return result;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}


LUA_FUNCTION(LoadSound)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    if (!name)
        return 0;
    if (g_game->m_sound->IsAvailable()) {
        void *sound = g_game->m_sound->FindSound(name);
        lua_pushrawnumber(L, (int)sound);
    } else {
        lua_pushrawnumber(L, -1);
    }
    return 1;
}

LUA_FUNCTION(GetSoundName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *sound = GetSoundArg(L, 1);
    if ((unsigned int)sound <= 0) {
        NativeDiagnostic("ScriptVM: Invalid sound handle ", sound);
        return 0;
    }
    if (g_game->m_sound && g_game->m_sound->IsAvailable()) {
        const char *name = g_game->m_sound->GetSoundName(sound);
        if (name)
            lua_pushstring(L, name);
        else
            lua_pushstring(L, "Unknown");
    } else {
        lua_pushstring(L, "Unknown");
    }
    return 1;
}

LUA_FUNCTION_NOSTACK_TODO(PlaySoundWithBabble)

LUA_FUNCTION(StopBabbleForLine)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    EMeshEntity *ent = (EMeshEntity *)GetTriggerArg(L, 1, false);
    if (!ent)
        return 0;
    const char *who = GetStringArg(L, 2, false);
    const char *curLine = GetStringArg(L, 3, false);
    char myPath[512];
    myPath[0] = 0;
    if (ent == (EMeshEntity *)g_game->GetPlayerControl() && curLine[0] && curLine[1])
        _snprintf(myPath, 255, "Anims/%s/LipSync/%c%c/%s.pba", who, curLine[0], curLine[1], curLine);
    else
        _snprintf(myPath, 255, "Anims/%s/LipSync/%s.pba", who, curLine);
    ent->GetMesh()->StopBlendAnim(myPath, -1.0f);
    return 0;
}

LUA_FUNCTION_NOSTACK(PlaySound)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 6))
        return 0;
    EEntity *theEnt = GetEntityArg(L, 1, true, 0);
    void *pSound = GetSoundArg(L, 2);
    int nVolume = n >= 3 ? (int)GetNumberArg(L, 3, true) : 0;
    int iWait = n >= 4 ? (int)GetNumberArg(L, 4, true) : 0;
    int skippable = n >= 5 ? (int)GetNumberArg(L, 5, true) : 0;
    bool theOnce = n >= 6 ? GetNumberArg(L, 6, true) != 0 : false;
    if (!pSound)
        return 0;
    int state = g_game->m_gameState;
    if (state != 2 && state != 3 && state != 0) {
        LuaCallSite(L);
        DumpLuaStack(L, 0, 0);
    }
    void *curInstanceVal = 0;
    if (theEnt)
        curInstanceVal = theEnt->PlaySound(pSound, 0, theOnce);
    else
        curInstanceVal = g_game->m_sound->Play(pSound, 0, 0);
    if (g_game->m_sound->HasFlag(pSound, 0))
        g_game->StartMusicCue();
    if (g_game->m_sound->HasFlag(pSound, 6))
        g_game->StartStinger();
    if (iWait) {
        bool waiting = true;
        if (g_game->m_recorder && g_game->m_recorder->IsPlaying()) {
            while (waiting) {
                waiting = g_game->m_recorder->WaitForPlayback();
                if (waiting) {
                    if (!g_game->GetScriptVM()->Yield(L)) {
                        // whatever happened here is compiled out
                    }
                }
            }
            return 0;
        }
        bool first = true;
        while (g_game->m_sound && g_game->m_sound->IsPlaying(pSound)) {
            if (g_game->m_recorder && g_game->m_recorder->IsRecording())
                g_game->m_recorder->PauseRecording(true);
            if (!g_game->GetScriptVM()->Yield(L)) {
                if (g_game->m_recorder && g_game->m_recorder->IsRecording())
                    g_game->m_recorder->PauseRecording(false);
                return 0;
            }
            if (!first && skippable && (g_game->m_skipDialog || g_game->m_skipSounds)) {
                g_game->m_saylines->Remove(false);
                g_game->m_sound->Stop(pSound, 0, 0);
                g_game->m_skipDialog = false;
                break;
            }
            first = false;
        }
        if (g_game->m_recorder && g_game->m_recorder->IsRecording())
            g_game->m_recorder->PauseRecording(false);
    }
    lua_pushrawnumber(L, (int)curInstanceVal);
    return 1;
}

LUA_FUNCTION(IsSoundPlaying)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *sound = GetSoundArg(L, 1);
    if (g_game->m_sound && g_game->m_sound->IsPlaying(sound))
        lua_pushnumber(L, 1.0f);
    else
        lua_pushnumber(L, 0.0f);
    return 1;
}

LUA_FUNCTION(StopSound)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    void *sound = GetSoundArg(L, 1);
    EEntity *ent = n >= 2 ? GetEntityArg(L, 2, true, 0) : 0;
    if (g_game->m_sound)
        g_game->m_sound->Stop(sound, ent ? ent->GetSoundInstance() : 0, 0);
    return 0;
}

LUA_FUNCTION(StopSoundsOnEntity)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EEntity *entity = GetEntityArg(L, 1, false, 0);
    if (!entity)
        return 0;
    if (g_game->m_sound)
        entity->StopSounds();
    return 0;
}

LUA_FUNCTION(StopSoundInstance)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    void *sound = GetSoundArg(L, 1);
    void *result = lua_torawvalue(L, 2);
    if (g_game->m_sound)
        g_game->m_sound->Stop(sound, 0, result);
    return 0;
}

LUA_FUNCTION(StopAllSounds)
{
    if (g_game->m_sound)
        g_game->m_sound->StopAll();
    return 0;
}

LUA_FUNCTION(SoundStopCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int category = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (category >= 0 && category < 256)
        g_game->m_sound->StopCategory(category);
    return 0;
}

LUA_FUNCTION(SoundMuteAll)
{
    return 0;
}

LUA_FUNCTION(SoundMute2D)
{
    return 0;
}

LUA_FUNCTION(SoundMute3D)
{
    return 0;
}

LUA_FUNCTION(SoundMuteNone)
{
    return 0;
}

LUA_FUNCTION(SoundVolumeCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    int category = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    float categoryVolume = GetNumberArg(L, 2, false);
    float time = n >= 3 ? GetNumberArg(L, 3, true) : 0.0f;
    if (category >= 0 && category < 256)
        g_game->m_sound->SetCategoryVolume(category, categoryVolume, time);
    return 0;
}

LUA_FUNCTION(SoundSetDucking)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool enable = GetNumberArg(L, 1, false) != 0;
    return 0;
}

LUA_FUNCTION(FXVolume)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float volume = GetNumberArg(L, 1, false);
    g_game->m_sound->SetFXVolume(volume, 0.0f);
    return 0;
}

LUA_FUNCTION(VoiceVolume)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float volume = GetNumberArg(L, 1, false);
    g_game->m_sound->SetVoiceVolume(volume, 0.0f);
    return 0;
}

LUA_FUNCTION(MusicVolume)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float volume = GetNumberArg(L, 1, false);
    g_game->m_sound->SetMusicVolume(volume, 0.0f);
    return 0;
}

LUA_FUNCTION(SoundMuteCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    int category = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    float time = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    if (category >= 0 && category < 256)
        g_game->m_sound->MuteCategory(category, time);
    return 0;
}

LUA_FUNCTION(SoundUnmuteCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    int category = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    float time = n >= 2 ? GetNumberArg(L, 2, true) : 0.0f;
    if (category >= 0 && category < 256)
        g_game->m_sound->UnmuteCategory(category);
    return 0;
}

LUA_FUNCTION(SoundPauseAll)
{
    g_game->m_sound->PauseAll();
    return 0;
}

LUA_FUNCTION(SoundResumeAll)
{
    g_game->m_sound->ResumeAll();
    return 0;
}

LUA_FUNCTION(SoundPauseCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int category = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (category >= 0 && category < 256)
        g_game->m_sound->PauseCategory(category);
    return 0;
}

LUA_FUNCTION(SoundPauseNonMenu)
{
    g_game->m_sound->PauseNonMenu();
    return 0;
}

LUA_FUNCTION(SoundResumeCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int category = (unsigned int)GetNumberArg(L, 1, false);
    if (lua_type(L, 1) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (category >= 0 && category <= 11)
        g_game->m_sound->ResumeCategory(category);
    return 0;
}

LUA_FUNCTION(SoundResumeNonMenu)
{
    g_game->m_sound->ResumeNonMenu();
    return 0;
}

LUA_FUNCTION(SoundSetTriggerVariable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    unsigned int variable = (unsigned int)GetNumberArg(L, 1, false);
    // Local spelling selects the observed RTM stack layout; name not recovered.
    float time = GetNumberArg(L, 2, false);
    if (variable >= 0 && variable < 32)
        g_game->m_sound->SetTriggerVariable(variable, time, 0);
    return 0;
}

LUA_FUNCTION(SoundGetTriggerVariable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int variable = (unsigned int)GetNumberArg(L, 1, false);
    if (variable >= 0 && variable < 32) {
        float value = g_game->m_sound->GetTriggerVariable(variable);
        lua_pushnumber(L, value);
        return 1;
    }
    return 0;
}

LUA_FUNCTION(SoundSetParameterControl)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    void *sound = GetSoundArg(L, 1);
    void *instance = lua_torawvalue(L, 2);
    const char *name = GetStringArg(L, 3, false);
    float time = GetNumberArg(L, 4, false);
    if (name)
        g_game->m_sound->SetParameterControl(name, time, instance, sound);
    return 0;
}

LUA_FUNCTION(SoundSetDistanceFactor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float factor = GetNumberArg(L, 1, false);
    g_game->m_sound->SetDistanceFactor(factor);
    return 0;
}

LUA_FUNCTION(SoundSetRolloffFactor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float factor = GetNumberArg(L, 1, false);
    g_game->m_sound->SetRolloffFactor(factor);
    return 0;
}

LUA_FUNCTION(SoundIsDolbyDigital)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    if (g_game->m_sound->IsDolbyDigital())
        lua_pushnumber(L, 1.0f);
    else
        lua_pushnumber(L, 0.0f);
    return 1;
}

LUA_FUNCTION(SoundSetCameraRelative)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    bool cameraRelative = GetNumberArg(L, 1, false) != 0;
    bool second = n >= 2 ? GetNumberArg(L, 2, true) != 0 : true;
    g_game->SetSoundCameraRelative(cameraRelative, second);
    return 0;
}

LUA_FUNCTION(SoundSetEcho)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *entity = GetEntityArg(L, 1, false, 0);
    if (!entity)
        return 0;
    bool useEcho = GetNumberArg(L, 2, false) != 0;
    entity->m_echo = useEcho;
    void *sound = entity->m_soundInstance;
    if (sound) {
        void *instance = entity->m_soundInstance;
        g_game->m_sound->SetEcho(instance, useEcho);
    }
    return 0;
}

LUA_FUNCTION(IsEntityPlayingCategory)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    EEntity *ent = GetEntityArg(L, 1, true, 0);
    int nCat = (unsigned int)GetNumberArg(L, 2, false);
    if (lua_type(L, 2) == LUA_TNIL) {
        LuaCallSite(L);
        return 0;
    }
    if (ent) {
        bool playing = g_game->m_sound->IsEntityPlayingCategory(ent, nCat);
        lua_pushnumber(L, playing ? 1.0f : 0.0f);
    } else {
        bool anyPlaying = g_game->m_sound->IsCategoryPlaying(0, nCat);
        lua_pushnumber(L, anyPlaying ? 1.0f : 0.0f);
    }
    return 1;
}

LUA_FUNCTION(LoadSoundData)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool night = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    bool curAll = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    if (!name || !*name)
        g_game->LoadSoundData(0, night, false);
    else {
        char path[300];
        strncpy(path, name, 299);
        path[299] = 0;
        StrToLower(path);
        g_game->LoadSoundData(path, night, curAll);
    }
    return 0;
}

LUA_FUNCTION(PurgeSoundData)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    const char *name = n >= 1 ? GetStringArg(L, 1, true) : 0;
    bool all = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    if (name) {
        char path[300];
        strncpy(path, name, 300);
        path[299] = 0;
        g_game->m_sound->PurgeSoundData(path, "_night", all);
    } else
        g_game->m_sound->PurgeSoundData(0, "_night", all);
    return 0;
}

LUA_FUNCTION(AnalyzeSounds)
{
    return 0;
}

LUA_FUNCTION(PlayPCSound)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    bool loop = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    bool wait = n >= 3 ? GetNumberArg(L, 3, true) != 0 : false;
    void *mySound = g_game->m_sound->FindSound(name);
    g_game->m_sound->Play(mySound, 0, 0);
    return 0;
}

LUA_FUNCTION(GetCueLength)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *sound = GetSoundArg(L, 1);
    int result = g_game->m_sound->GetCueLength(sound);
    // The original release binding discards this result and pushes nothing.
    return 0;
}

LUA_FUNCTION(Set2DAttenuation)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int nValue = (int)GetNumberArg(L, 1, false);
    g_game->m_sound->Set2DAttenuation(nValue);
    return 0;
}

LUA_FUNCTION(SoundEnableDebugStreams)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool on = GetNumberArg(L, 1, false) != 0;
    return 0;
}
