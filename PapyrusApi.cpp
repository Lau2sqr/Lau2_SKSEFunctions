#include "PapyrusApi.h"

#include "FactionRankHook.h"
#include "RemoveFromFactionHook.h"

namespace FactionRankHook::PapyrusApi {

    namespace Detail {
        RE::VMHandle GetHandleForListener(RE::TESForm* a_form) {
            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            if (!vm || !a_form) {
                return static_cast<RE::VMHandle>(0);
            }
            auto* policy = vm->GetObjectHandlePolicy();
            if (!policy) {
                return static_cast<RE::VMHandle>(0);
            }
            return policy->GetHandleForObject(a_form->GetFormType(), a_form);
        }
    }

    bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm) {
        if (!a_vm) {
            return false;
        }

        bool ok = true;
        // Each hook module registers its own native functions here under
        // the shared "Lau2_SKSEFunctions" script class. Adding a new hook
        // module means adding one line here - nothing else in this file
        // changes.
        ok &= FactionRankHook::RegisterNativeFunctions(a_vm);
        ok &= RemoveFromFactionHook::RegisterNativeFunctions(a_vm);

        spdlog::info("PapyrusApi: native functions registered ({}).", ok ? "ok" : "with errors");
        return ok;
    }

    void ClearAllListeners() {
        FactionRankHook::ClearListeners();
        RemoveFromFactionHook::ClearListeners();
    }
}
