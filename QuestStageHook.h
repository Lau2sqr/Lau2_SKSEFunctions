#pragma once

#include "PapyrusApi.h"

namespace QuestStageHook {
    // Installs a native TESQuestStageEvent sink via ScriptEventSourceHolder -
    // this is an event CommonLibSSE-NG already exposes internally, so no
    // trampoline/offset hook is needed (unlike FactionRankHook/CraftHook).
    void Install();

    // Registers:
    //   bool RegisterForQuestStage(Form akListener, String asScriptName, Quest akQuest)
    //   bool UnregisterForQuestStage(Form akListener, String asScriptName, Quest akQuest)
    // Listener script must implement:
    //   Event OnQuestStageChange(Quest akQuest, Int aiNewStage)
    // Dispatch is filtered internally - a listener only receives events for
    // the specific akQuest it registered for, not for every quest in the game.
    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm);

    void ClearListeners();
}