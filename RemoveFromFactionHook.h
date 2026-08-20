#pragma once

#include "PapyrusApi.h"

namespace RemoveFromFactionHook {
    // Installs a hook on Skyrim's internal RemoveFromFaction function so that
    // Papyrus scripts can react when an actor is actually removed from a
    // faction (as opposed to a rank change - see FactionRankHook).
    //
    // NOTE: does nothing until a valid offset is filled in below - see the
    // TODO in RemoveFromFactionHook.cpp. Safe to call/ship in this state.
    void Install();

    // Registers this module's native Papyrus functions:
    //   bool RegisterForFactionRemoved(Form akListener, String asScriptName)
    //   bool UnregisterForFactionRemoved(Form akListener, String asScriptName)
    // Called from PapyrusApi::RegisterFunctions.
    //
    // akListener's script must implement:
    //   Event OnFactionRemoved(Actor akActor, Faction akFaction)
    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm);

    // Clears this module's registered listeners. Called from
    // PapyrusApi::ClearAllListeners (new game / load game).
    void ClearListeners();
}
