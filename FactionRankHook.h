#pragma once

#include "PapyrusApi.h"

namespace FactionRankHook {
    // Installs a hook on Skyrim's internal SetFactionRank function so that
    // Papyrus scripts can react to faction rank changes via the typed
    // FactionRankChange event, instead of polling with OnLocationChange or
    // similar.
    void Install();

    // Registers this module's native Papyrus functions:
    //   bool RegisterForFactionRankChange(Form akListener, String asScriptName)
    //   bool UnregisterForFactionRankChange(Form akListener, String asScriptName)
    // Called from PapyrusApi::RegisterFunctions - see PapyrusApi.h for the
    // single central registration entry point.
    //
    // akListener's script must implement:
    //   Event OnFactionRankChanged(Actor akActor, Faction akFaction, Int aiNewRank)
    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm);

    // Clears this module's registered listeners. Called from
    // PapyrusApi::ClearAllListeners (new game / load game).
    void ClearListeners();
}
