#pragma once

#include "PapyrusApi.h"

namespace CraftHook {
    // Hooks all four workbench craft call sites (Smithing/Tempering/
    // Enchanting/Alchemy) and fires OnItemCrafted for each.
    void Install();

    // Registers:
    //   bool RegisterForItemCrafted(Form akListener, String asScriptName)
    //   bool UnregisterForItemCrafted(Form akListener, String asScriptName)
    // Listener script must implement:
    //   Event OnItemCrafted(ObjectReference akBench, Location akLocation,
    //                        Form akCreatedItem, Int aiWorkbenchType)
    // aiWorkbenchType: 0=Smithing, 1=Tempering, 2=Enchanting, 3=Alchemy
    // (empirically verified per-version - see Install()).
    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm);

    void ClearListeners();
}
