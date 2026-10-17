// Script functions, file name made up.

#include "engine.h"
#include "luafunc.h"

struct EDialogBubble {
    void SetSelection(int index);
    void Exit(int choice);

    char pad0[0x2cc];
    int m_font;                     // 0x2cc
};

struct EJournal {
    void CompletedGoal(const char *name);
    void RemoveAllGoals();
};

struct EScrapbook {
    void RegisterEnvelope(const char *a, const char *b, const char *c);
};

struct EGameMenuList {
    void Add(const char *name, const char *command);
    void Remove();
};

struct EGameMenuPage {
    EJournal *GetJournal() { return m_journal; }
    void Open(int flags);
    int AddTitle(const char *title);
    void AddLevels(int count, const char *a, const char *b);
    void AddItem(const char *name, const char *command);
    void Clear();

    char pad0[0x154];
    int m_font;                     // 0x154
    char pad158[0x184 - 0x158];
    EJournal *m_journal;            // 0x184
    EScrapbook *m_scrapbook;        // 0x188
    EGameMenuList *m_gameMenu;      // 0x18c
};

struct EBunkhouseUI {
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void Deactivate();      // 0x14
    bool IsAnimating();
    void AnimComplete(int done);
    const char *GetBunkDisplayName(int index);

    char pad4[0x230 - 4];
    unsigned char m_showKeyboard;   // 0x230
    char pad231[0x649 - 0x231];
    unsigned char m_pauseControls;  // 0x649
};

struct EMainMenu {
    virtual void Vf00();
    virtual void Vf04();
    virtual void Vf08();
    virtual void Vf0c();
    virtual void Vf10();
    virtual void Deactivate();      // 0x14
    void DrawText(int index);
    void HideText(int index);
    void ProcessSelection(int index);
    void PurgeCredits();

    char pad4[0x2d4 - 4];
    bool m_pauseControlHandler;     // 0x2d4
    char pad2d5[0x2e0 - 0x2d5];
    bool m_showText;                // 0x2e0
};

struct ESlideshow {
    void Start(const char *name, int count, const char *a);
};

struct ECheer {
    bool IsDone() { return m_4 == 0 || m_0 == 0; }
    void Set(float time, int show, float delay);
    void NoPower(Vector2 pos, const char *text);
    void NewPower(Vector2 pos, const char *text, void *tex);
    void Remove();

    int m_0;
    int m_4;
};

struct EHUDCounterExtra;

struct EScavengerSheet {
    void Show(bool show);
    int GetCollectedFigmentCount();

    char pad0[0x175];
    unsigned char m_exitFromJournal;    // 0x175
};

struct EMainMenuPage {
    char pad0[0x130];
    bool m_startPressed;            // 0x130
};

struct ECredits {
    bool IsDone() { return m_done; }
    void Add(const char *a, const char *b);
    void Init(int flags);
    void Stop();

    char pad0[0x30];
    bool m_done;                    // 0x30
};

#include <string.h>

struct ESaylineManager {
    bool IsDone(EScriptObject *speaker);
    void Remove(bool all);
    EScriptObject *GetSpeaker();

    char pad0[0xc];
    bool m_resetPosition;           // 0x0c
    char padd[0x1c - 0xd];
    Vector2 m_position;             // 0x1c
};


struct EText3DManager {
    void AttachToBone(void *text, ETriggerBase *ent, const char *bone);
    void Remove3(void *text);
    void FadeOnHUD(void *text, bool in);
    void SetPriority(void *text, unsigned int priority);
    void Remove(void *text);
};

struct EHUDCounter {
    void Show(bool show);
    void Rankup(Vector2 pos, int rank);

    char pad0[0xc];
    int m_count;                    // 0x0c
};

struct EInputRecorder {
    void RecordUseSaveGame(const char *name);
    void PlaybackUseSaveGame(const char *name);
    void PlaybackFile(const char *name);
    void StopRecording();
    void StopPlayback();
    void Playback(const char *name, int flags);

    char pad0[0x12];
    bool m_showInfo;                // 0x12
    char pad13[0x19 - 0x13];
    bool m_recording;               // 0x19
    bool m_playing;                 // 0x1a
};

struct ELine2DManager {
    void Remove(void *line);
};


LUA_FUNCTION_NOSTACK_TODO(SayLine)

LUA_FUNCTION_TODO(PreloadSaylineHead)

static inline void SetSaylinePos(Vector2 pos)
{
    g_game->GetSaylines()->m_position = pos;
}

LUA_FUNCTION(SetSaylinePosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    Vector2 pos(32.0f, 325.0f);
    if (lua_type(L, 1) != LUA_TNIL)
        pos.x = lua_tonumber(L, 1);
    if (lua_type(L, 2) != LUA_TNIL)
        pos.y = lua_tonumber(L, 2);
    SetSaylinePos(pos);
    return 0;
}

LUA_FUNCTION(ResetSaylinePosition)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_saylines->m_resetPosition = true;
    return 0;
}

LUA_FUNCTION(IsSaylineDone)
{
    if (lua_gettop(L) < 1 || lua_gettop(L) > 1)
        return 0;
    const void *arg = lua_topointer(L, 1);
    EScriptObject *pSpeaker = GetScriptObjectArg(L, (int)arg, false, false);
    if (pSpeaker) {
        bool done = g_game->m_saylines->IsDone(pSpeaker);
        if (done)
            lua_pushint(L, 1);
        else
            lua_pushint(L, 0);
        return 1;
    }
    lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(SetMeshSayLineParams)

LUA_FUNCTION(RemoveSayline)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool all = n >= 1 ? GetNumberArg(L, 1, true) != 0 : true;
    g_game->m_saylines->Remove(all);
    return 0;
}

LUA_FUNCTION(IsLSOShowingDialog)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EScriptObject *so = GetScriptObjectArg(L, 1, false, false);
    if (!so)
        return 0;
    if (so && so == g_game->m_saylines->GetSpeaker())
        lua_pushnumber(L, 1.0f);
    else
        lua_pushnumber(L, 0.0f);
    return 1;
}

LUA_FUNCTION_TODO(SetPrintMessageParams)

LUA_FUNCTION(DeletePrintMessages)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    g_game->m_hud.DeletePrintMessages();
    return 0;
}

LUA_FUNCTION(IsPrintMessageDone)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    bool done = g_game->GetHUD()->IsPrintMessageDone();
    if (done)
        lua_pushint(L, 1);
    else
        lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION_NOSTACK_TODO(PrintMessage)

LUA_FUNCTION(PrintNote)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *str = lua_tostring(L, 1);
    g_game->m_hud.PrintNote(str);
    return 0;
}

LUA_FUNCTION(SetProximityHint)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 2))
        return 0;
    const char *str = n >= 1 ? GetStringArg(L, 1, true) : 0;
    bool flag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    g_game->m_hud.SetProximityHint(str, flag);
    return 0;
}

LUA_FUNCTION(SetCollectedItemFlyTime)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float time = GetNumberArg(L, 1, false);
    g_game->m_hudElements.m_cheering->Set(time, 1, 0.0f);
    return 0;
}

LUA_FUNCTION(SetCollectedItemText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 2))
        return 0;
    const char *s = GetStringArg(L, 1, false);
    bool flag = n >= 2 ? GetNumberArg(L, 2, true) != 0 : false;
    g_game->m_hud.SetCollectedItemText(s, flag);
    return 0;
}

LUA_FUNCTION_TODO(ClearCollectedItemText)

LUA_FUNCTION(HideFordCheering)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float time = GetNumberArg(L, 1, false);
    g_game->m_hudElements.m_cheering->Set(time, 1, 0.0f);
    return 0;
}

LUA_FUNCTION(RestoreFordCheering)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_hudElements.m_cheering->Set(2000.0f, 0, 1000.0f);
    return 0;
}

LUA_FUNCTION(Text3DAttachToBone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    void *szText = lua_torawvalue(L, 1);
    ETriggerBase *e = GetTriggerArg(L, 2, false);
    if (!e)
        return 0;
    const char *szBone = GetStringArg(L, 3, false);
    g_game->m_text3d->AttachToBone(szText, e, szBone);
    return 0;
}

LUA_FUNCTION_TODO(DisplayText3)

LUA_FUNCTION_TODO(AdjustColor3)

LUA_FUNCTION_TODO(AdjustPos3)

LUA_FUNCTION_TODO(AdjustScale3)

LUA_FUNCTION(RemoveText3)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *text = lua_torawvalue(L, 1);
    g_game->m_text3d->Remove3(text);
    return 0;
}

LUA_FUNCTION(GetLocalizedText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *nId = GetStringArg(L, 1, false);
    lua_pushstring(L, g_game->GetLocalizedText(nId));
    return 1;
}

LUA_FUNCTION_TODO(DrawStringWorldSpace)

LUA_FUNCTION_TODO(DisplayText)

LUA_FUNCTION(TextFadeOnHUD)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    void *s = lua_torawvalue(L, 1);
    bool in = GetNumberArg(L, 2, false) != 0;
    g_game->m_text3d->FadeOnHUD(s, in);
    return 0;
}

LUA_FUNCTION(SetTextPriority)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    void *str = lua_torawvalue(L, 1);
    unsigned int nPriority = GetNumberArg(L, 2, false);
    g_game->m_text3d->SetPriority(str, nPriority);
    return 0;
}

LUA_FUNCTION(RemoveText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    unsigned int text = (unsigned int)lua_torawvalue(L, 1);
    if (text < 0)
        ;
    else
        g_game->m_text3d->Remove((void *)text);
    return 0;
}

LUA_FUNCTION_TODO(AdjustTextLine)

LUA_FUNCTION(ShowHealth)
{
    g_game->m_hudElements.Show(1, 1);
    return 0;
}

LUA_FUNCTION_TODO(HideHUD)

LUA_FUNCTION_TODO(CollectiblePickedUp)

LUA_FUNCTION(UI_SetNumCollectibles)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *type = GetStringArg(L, 1, false);
    int cnt = (int)GetNumberArg(L, 2, false);
    if (!_stricmp(type, "arrowhead"))
        g_game->GetArrowheadCounter()->m_count = cnt;
    if (!_stricmp(type, "PSICards"))
        g_game->GetPsiCardCounter()->m_count = cnt;
    return 0;
}

LUA_FUNCTION_TODO(UI_AdjustCollectible)

LUA_FUNCTION_TODO(UI_AddFigment)

LUA_FUNCTION(ShowRecPlayInfo)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool show = GetNumberArg(L, 1, false) != 0;
    g_game->GetRecorder()->m_showInfo = show;
    return 0;
}

LUA_FUNCTION_TODO(RecordControllerInput)

LUA_FUNCTION_TODO(AttractModeRecordInput)

LUA_FUNCTION(AttractModeRecordInputUseSaveGame)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    g_game->m_recorder->RecordUseSaveGame(name);
    return 0;
}

LUA_FUNCTION(AttractModePlaybackInputUseSaveGame)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    g_game->m_recorder->PlaybackUseSaveGame(name);
    return 0;
}

LUA_FUNCTION(AttractModePlaybackInput)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    g_game->m_recorder->PlaybackFile(name);
    return 0;
}

LUA_FUNCTION(StopRecordingInput)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    bool recording = g_game->m_recorder->m_recording;
    if (recording)
        g_game->m_recorder->StopRecording();
    return 0;
}

LUA_FUNCTION(StopPlaybackInput)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    bool bPlaying = g_game->m_recorder->m_playing;
    if (bPlaying)
        g_game->m_recorder->StopPlayback();
    return 0;
}

LUA_FUNCTION(IsPlaybackDone)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    bool playing = g_game->m_recorder->m_playing;
    int done = !playing;
    lua_pushint(L, done);
    return 0;
}

LUA_FUNCTION(IsPlaybackMode)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    bool playing = g_game->m_recorder->m_playing;
    int mode = playing != 0;
    lua_pushint(L, mode);
    return 0;
}

LUA_FUNCTION(IsRecordMode)
{
    if (lua_gettop(L) < 0 || lua_gettop(L) > 0)
        return 0;
    bool recording = g_game->m_recorder->m_recording;
    int mode = recording != 0;
    lua_pushint(L, mode);
    return 0;
}

LUA_FUNCTION(PlaybackControllerInput)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    const char *name = n >= 1 ? GetStringArg(L, 1, true) : 0;
    if (name)
        g_game->m_recorder->Playback(name, 0);
    else
        g_game->m_recorder->Playback("Demo", 0);
    return 0;
}

LUA_FUNCTION_TODO(Add2DLine)

LUA_FUNCTION(Remove2DLine)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *pLine = lua_torawvalue(L, 1);
    g_game->m_lines2d->Remove(pLine);
    return 0;
}

LUA_FUNCTION_TODO(Draw2DLine)

LUA_FUNCTION_TODO(AddSprite)

LUA_FUNCTION_TODO(AddSpriteHandle)

LUA_FUNCTION_TODO(SpriteAdjust)

LUA_FUNCTION(SpriteColor)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 5, 5))
        return 0;
    void *sprite = lua_torawvalue(L, 1);
    Color colorNum = GetColorArg(L, 2, false);
    g_game->m_overlay->SetSpriteColor(sprite, colorNum);
    return 0;
}

LUA_FUNCTION(SpriteFadeOnHUD)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 3))
        return 0;
    void *pSprite = lua_torawvalue(L, 1);
    bool in = GetNumberArg(L, 2, false) != 0;
    int nFrames1 = n >= 3 ? (int)GetNumberArg(L, 3, true) : 0;
    g_game->m_overlay->FadeSprite(pSprite, in, nFrames1);
    return 0;
}

LUA_FUNCTION(RemoveSprite)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    void *sprite = lua_torawvalue(L, 1);
    g_game->m_overlay->RemoveSprite(sprite);
    return 0;
}

LUA_FUNCTION(CustomAutosave)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    g_game->m_hudElements.CustomAutosave(name);
    return 0;
}

LUA_FUNCTION(Autosave)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_hudElements.Autosave("Autosave");
    return 0;
}

LUA_FUNCTION_TODO(RegisterCollectible)

LUA_FUNCTION_TODO(DialogChoiceAddOatmealTextures)

LUA_FUNCTION_NOSTACK_TODO(DialogChoiceBubble)

LUA_FUNCTION_TODO(DialogChoiceSetLineColor)

LUA_FUNCTION(SetDialogBubbleCurSelection)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    g_game->m_dialogBubble->SetSelection(index);
    return 0;
}

LUA_FUNCTION(ExitDialogChoiceBubble)
{
    g_game->m_dialogBubble->Exit(-1);
    return 0;
}

LUA_FUNCTION(SetDialogBubbleFont)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int font = (int)GetNumberArg(L, 1, false);
    g_game->GetDialogBubble()->m_font = font;
    return 0;
}

LUA_FUNCTION_TODO(SetDialogBubbleFontColors)

LUA_FUNCTION_TODO(SetDialogBubbleFontScale)

LUA_FUNCTION_TODO(JournalAddGoal)

LUA_FUNCTION(JournalCompletedGoal)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("InGameMenu");
    if (page)
        page->GetJournal()->CompletedGoal(name);
    return 0;
}

LUA_FUNCTION(JournalRemoveAllGoals)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("InGameMenu");
    if (page)
        page->GetJournal()->RemoveAllGoals();
    return 0;
}

LUA_FUNCTION_TODO(JournalAddScrapbookItem)

LUA_FUNCTION_TODO(JournalAddEmoBagItem)

LUA_FUNCTION_TODO(JournalAddMovie)

LUA_FUNCTION(RegisterScrapbookEnvelope)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    const char *fAngle = GetStringArg(L, 1, false);
    const char *b = GetStringArg(L, 2, false);
    const char *c = GetStringArg(L, 3, false);
    EGameMenuPage *pPageArg = (EGameMenuPage *)g_game->m_menus->GetPage("InGameMenu");
    if (pPageArg) {
        EScrapbook *pBookCur = pPageArg->m_scrapbook;
        if (pBookCur)
            pBookCur->RegisterEnvelope(fAngle, b, c);
    }
    return 0;
}

LUA_FUNCTION(ShowQuickPickMenu)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    bool show = n >= 1 ? GetNumberArg(L, 1, true) != 0 : true;
    if (show)
        g_game->m_menus->ShowQuickPickMenu(0);
    else
        g_game->GetMenus()->HideQuickPickMenu();
    return 0;
}

LUA_FUNCTION(ShowJournal)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int page = (int)GetNumberArg(L, 1, false);
    if (g_game->m_journalEnabled && !GetScriptManager()->IsBusy()) {
        EGameMenuPage *menu = (EGameMenuPage *)g_game->m_menus->GetPage("InGameMenu");
        if (menu) {
            menu->Open(1);
            g_game->m_menus->ToggleInGameMenu();
        }
    }
    return 0;
}

LUA_FUNCTION(ExitJournal)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_menus->ExitJournal();
    return 0;
}

LUA_FUNCTION(JournalAddGameMenuItem)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    const char *szCommand1 = GetStringArg(L, 2, false);
    EGameMenuPage *pPage = (EGameMenuPage *)g_game->m_menus->GetPage("InGameMenu");
    if (pPage) {
        EGameMenuList *pList2 = pPage->m_gameMenu;
        if (pList2)
            pList2->Add(name, szCommand1);
    }
    return 0;
}

LUA_FUNCTION(JournalRemoveGameMenuItem)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("InGameMenu");
    if (page) {
        EGameMenuList *list = page->m_gameMenu;
        if (list)
            list->Remove();
    }
    return 0;
}

LUA_FUNCTION(BunkhouseUIAnimComplete)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int val = n >= 1 ? (int)GetNumberArg(L, 1, true) : 1;
    unsigned char done = val == 1;
    if (done) {
        if (g_game->m_bunkhouse->IsAnimating())
            g_game->m_bunkhouse->AnimComplete(1);
    } else {
        g_game->m_bunkhouse->AnimComplete(0);
    }
    return 0;
}

LUA_FUNCTION(BunkhouseUIShowKeyboardUI)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int amount = n >= 1 ? (int)GetNumberArg(L, 1, true) : 1;
    unsigned char show = amount == 1;
    g_game->m_bunkhouse->m_showKeyboard = show;
    return 0;
}

LUA_FUNCTION(BunkhouseUIPauseControls)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 1))
        return 0;
    int val = n >= 1 ? (int)GetNumberArg(L, 1, true) : 1;
    unsigned char pause = val == 1;
    g_game->GetBunkhouse()->m_pauseControls = pause;
    return 0;
}

LUA_FUNCTION(BunkhouseUIGetBunkDisplayName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    const char *szName = g_game->m_bunkhouse->GetBunkDisplayName(index - 1);
    lua_pushstring(L, szName);
    return 1;
}

LUA_FUNCTION_TODO(InitKeypad)

LUA_FUNCTION(ExitKeypad)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int code = 0;
    EHUDElements *hud = &g_game->m_hudElements;
    hud->m_keypad->Exit();
    return 0;
}

LUA_FUNCTION(KeypadAddNumber)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int nNumber3 = (int)GetNumberArg(L, 1, false);
    EHUDElements *hud = &g_game->m_hudElements;
    hud->m_keypad->AddNumber(nNumber3);
    return 0;
}

LUA_FUNCTION(AddPolishedMenuTitle)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *szTitle = GetStringArg(L, 1, false);
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("OfficialLevelMenu");
    if (!page)
        return 0;
    int nId = page->AddTitle(szTitle);
    lua_pushint(L, nId);
    return 1;
}

LUA_FUNCTION(AddLevelsToPolishedMenu)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    int total = (int)GetNumberArg(L, 1, false);
    const char *szA = GetStringArg(L, 2, false);
    const char *enable = GetStringArg(L, 3, false);
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("OfficialLevelMenu");
    if (!page)
        return 0;
    page->AddLevels(total, szA, enable);
    return 0;
}

LUA_FUNCTION(AddQuickMenuItem)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *szName = GetStringArg(L, 1, false);
    const char *szCommand2 = GetStringArg(L, 2, false);
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("QuickMenu");
    if (!page)
        return 0;
    page->AddItem(szName, szCommand2);
    return 0;
}

LUA_FUNCTION(ClearAllQuickMenuItems)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("QuickMenu");
    if (!page)
        return 0;
    page->Clear();
    return 0;
}

LUA_FUNCTION_TODO(SetQuickMenuTextColor)

LUA_FUNCTION(SetQuickMenuFont)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    EGameMenuPage *page = (EGameMenuPage *)g_game->m_menus->GetPage("QuickMenu");
    if (!page)
        return 0;
    int font = (int)GetNumberArg(L, 1, false);
    if (font > 4 || font < 1)
        font = 1;
    page->m_font = font;
    return 0;
}

LUA_FUNCTION_TODO(AddTutorialBox)

LUA_FUNCTION(InitMap)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    g_game->m_menus->InitMap(name);
    return 0;
}

LUA_FUNCTION(ShowMap)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_menus->ShowMap();
    return 0;
}

LUA_FUNCTION_TODO(AddMapLabel)

LUA_FUNCTION_TODO(CollectibleToJournalDisplay)

LUA_FUNCTION(HUDRankup)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float val = GetNumberArg(L, 1, false);
    float fY = GetNumberArg(L, 2, false);
    int nRank = (int)GetNumberArg(L, 3, false);
    g_game->m_hudElements.m_rank->Rankup(Vector2(val, fY), nRank);
    return 0;
}

LUA_FUNCTION(ActivateBunkControlHandler)
{
    g_game->SetControlHandler(12, 0);
    return 0;
}

LUA_FUNCTION(DeactivateBunkControlHandler)
{
    g_game->m_bunkhouse->Deactivate();
    g_game->SetControlHandler(0, 0);
    return 0;
}

LUA_FUNCTION(ActivateMainMenuControlHandler)
{
    g_game->SetControlHandler(18, 0);
    return 0;
}

LUA_FUNCTION(DeactivateMainMenuControlHandler)
{
    g_game->m_mainMenu->Deactivate();
    g_game->SetControlHandler(0, 0);
    return 0;
}

LUA_FUNCTION(ActivateFigmentControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->PushControlHandler(19);
    return 0;
}

LUA_FUNCTION(ActivateSlideshowControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    int c = (int)GetNumberArg(L, 2, false);
    const char *music = GetStringArg(L, 3, false);
    g_game->PushControlHandler(17);
    g_game->m_slideshow->Start(name, c, music);
    return 0;
}

LUA_FUNCTION(DeactivateSlideshowControlHandler)
{
    g_game->PushControlHandler(-1);
    return 0;
}

LUA_FUNCTION_TODO(InitPlayerMood)

LUA_FUNCTION(GetPlayerMaxMood)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION(SetPlayerMaxMood)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float mood = GetNumberArg(L, 1, false);
    return 0;
}

LUA_FUNCTION(AdjustPlayerMood)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    float mood = GetNumberArg(L, 1, false);
    return 0;
}

LUA_FUNCTION(GetPlayerMood)
{
    lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(SetCamInvertPitch)

LUA_FUNCTION_TODO(SetCamInvertYaw)

LUA_FUNCTION_TODO(SetCamFirstPersonInvertPitch)

LUA_FUNCTION(RankupCheerNoPower)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 3, 3))
        return 0;
    float val = GetNumberArg(L, 1, false);
    float fY = GetNumberArg(L, 2, false);
    const char *s = GetStringArg(L, 3, false);
    g_game->m_hudElements.m_rankupCheer->NoPower(Vector2(val, fY), s);
    return 0;
}

LUA_FUNCTION(IsRankupCheerDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    bool done = g_game->m_hudElements.m_rankupCheer->IsDone();
    if (done)
        lua_pushnumber(L, 1.0f);
    else
        lua_pushnumber(L, 0.0f);
    return 1;
}

LUA_FUNCTION(RankupCheerNewPower)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 4, 4))
        return 0;
    float val = GetNumberArg(L, 1, false);
    float fY = GetNumberArg(L, 2, false);
    const char *s = GetStringArg(L, 3, false);
    EResource *pTex2 = (EResource *)GetResourceArg(L, 4);
    g_game->m_hudElements.m_cheering->NewPower(Vector2(val, fY), s, *pTex2->m_data);
    return 0;
}

LUA_FUNCTION(RemoveRankupCheer)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_hudElements.m_cheering->Remove();
    return 0;
}

LUA_FUNCTION(DrawMainMenuText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    index--;
    g_game->m_mainMenu->DrawText(index);
    return 0;
}

LUA_FUNCTION(HideMainMenuText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    index--;
    g_game->m_mainMenu->HideText(index);
    return 0;
}

LUA_FUNCTION(ShowHUDCollectible)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    int nType2 = (int)GetNumberArg(L, 1, false);
    bool show = GetNumberArg(L, 2, false) != 0;
    if (nType2 == 1)
        g_game->m_hudElements.m_rank->Show(show);
    else if (nType2 == 2)
        g_game->m_hudElements.m_arrowheads->Show(show);
    return 0;
}

LUA_FUNCTION(IsSubtitlesEnabled)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int on = g_game->m_subtitles != 0;
    lua_pushint(L, on);
    return 1;
}

LUA_FUNCTION(MainMenuProcessSelection)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    int index = (int)GetNumberArg(L, 1, false);
    g_game->m_mainMenu->ProcessSelection(index - 1);
    return 0;
}

LUA_FUNCTION(PauseMenuControlHandler)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool pause = GetNumberArg(L, 1, false) != 0;
    g_game->m_mainMenu->m_pauseControlHandler = pause;
    return 0;
}

LUA_FUNCTION(ShowMainMenuText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool show = GetNumberArg(L, 1, false) != 0;
    g_game->GetMainMenu()->m_showText = show;
    return 0;
}

LUA_FUNCTION(ShowScavengerHuntSheet)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool show = GetNumberArg(L, 1, false) != 0;
    if (!g_game->m_e5)
        g_game->m_menus->GetScavengerSheet()->Show(show);
    return 0;
}

LUA_FUNCTION(ClearRazTable)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->GetSave()->ClearRazTable();
    g_game->m_hudElements.m_rankupCheer->Remove();
    g_game->m_hudElements.m_cheering->Remove();
    return 0;
}

LUA_FUNCTION_TODO(AddCompletedGoalDisplay)

LUA_FUNCTION(AddNewGoalDisplay)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    const char *name = GetStringArg(L, 1, false);
    if (g_game->m_goalDisplay)
        g_game->m_hudElements.AddNewGoal(name);
    return 0;
}

LUA_FUNCTION(IsNewGoalDisplayDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    if (!g_game->m_goalDisplay)
        lua_pushnumber(L, 1.0f);
    else if (g_game->m_hudElements.IsNewGoalDone())
        lua_pushnumber(L, 1.0f);
    else
        lua_pushnumber(L, 0.0f);
    return 1;
}

LUA_FUNCTION(IsCompletedGoalDisplayDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    if (!g_game->m_goalDisplay)
        lua_pushnumber(L, 1.0f);
    else if (g_game->m_hudElements.IsCompletedGoalDone())
        lua_pushint(L, 1);
    else
        lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION(DoesContinueExist)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    bool exists = g_game->m_save->m_continueExists;
    if (exists)
        lua_pushint(L, 1);
    else
        lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION(DoesAnyGameProfileExist)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    bool exists = g_game->m_save->m_anyProfile;
    if (exists)
        lua_pushint(L, 1);
    else
        lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(AddCreditsTextToMesh)

LUA_FUNCTION_TODO(SetCreditsText)

LUA_FUNCTION(PurgeCreditsText)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->m_mainMenu->PurgeCredits();
    return 0;
}

LUA_FUNCTION(IsMainMenuStartPressed)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    EMainMenuPage *page = g_game->m_menus->m_mainPage;
    if (page->m_startPressed)
        lua_pushint(L, 1);
    else
        lua_pushint(L, 0);
    return 1;
}

LUA_FUNCTION_TODO(GetStringDims)

LUA_FUNCTION(AddEndingCredits)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 2, 2))
        return 0;
    const char *fAngle = GetStringArg(L, 1, false);
    const char *b = GetStringArg(L, 2, false);
    g_game->GetCredits()->Add(fAngle, b);
    return 0;
}

LUA_FUNCTION(InitEndingCredits)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->GetCredits()->Init(1);
    return 0;
}

LUA_FUNCTION(IsEndCreditsDone)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int done = g_game->GetCredits()->IsDone() != 0;
    lua_pushint(L, done);
    return 1;
}

LUA_FUNCTION(IsPCBuild)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    lua_pushint(L, 1);
    return 1;
}

LUA_FUNCTION(StopEndingCredits)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->GetCredits()->Stop();
    return 0;
}

LUA_FUNCTION(ShowDemoEndScreen)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 1, 1))
        return 0;
    bool show = GetNumberArg(L, 1, false) != 0;
    g_game->m_hudElements.ShowDemoEnd(show, 0);
    return 0;
}

LUA_FUNCTION_TODO(AddFontTexturedMesh)

LUA_FUNCTION(RemoveAllFontTexturedMeshes)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    g_game->RemoveAllFontTexturedMeshes();
    return 0;
}

LUA_FUNCTION(GetCurrentProfileName)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    const char *name = g_game->GetSave()->GetCurrentProfileName();
    lua_pushstring(L, name);
    return 1;
}

LUA_FUNCTION(GetGameTimePlayedSecs)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    float secs = g_game->GetGameTimePlayed();
    lua_pushnumber(L, secs);
    return 1;
}

LUA_FUNCTION(GetCollectedFigmentCount)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int num = g_game->m_menus->GetScavengerSheet()->GetCollectedFigmentCount();
    lua_pushnumber(L, num);
    return 1;
}

LUA_FUNCTION(IsExitFromJournal)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    int exitFromJournal = g_game->m_menus->GetScavengerSheet()->m_exitFromJournal != 0;
    lua_pushnumber(L, exitFromJournal);
    return 1;
}

LUA_FUNCTION(GetLanguage)
{
    int n = lua_gettop(L);
    if (!CheckArgs(L, 0, 0))
        return 0;
    switch (g_game->GetLanguage()) {
    case 1:
        lua_pushnumber(L, 2.0f);
        break;
    case 2:
        lua_pushnumber(L, 3.0f);
        break;
    default:
        lua_pushnumber(L, 1.0f);
        break;
    }
    return 1;
}
