#include "PapyrusApi.h"

#include "CraftHook.h"
#include "FactionRankHook.h"
#include "RemoveFromFactionHook.h"
#include "QuestStageHook.h"

namespace Lau2_SKSEFunctions::PapyrusApi {

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

    // AddKeyword / RemoveKeyword / AddKeywordToRef / RemoveKeywordFromRef
    // Pattern based on powerofthree's Papyrus Extender (MIT License).

    bool AddKeyword(RE::StaticFunctionTag*, RE::TESForm* a_form, RE::BGSKeyword* a_keyword) {
        if (!a_form || !a_keyword) {
            return false;
        }
        auto keywordForm = a_form->As<RE::BGSKeywordForm>();
        if (!keywordForm) {
            return false;
        }
        return keywordForm->AddKeyword(a_keyword);
    }

    bool RemoveKeyword(RE::StaticFunctionTag*, RE::TESForm* a_form, RE::BGSKeyword* a_keyword) {
        if (!a_form || !a_keyword) {
            return false;
        }
        auto keywordForm = a_form->As<RE::BGSKeywordForm>();
        if (!keywordForm) {
            return false;
        }
        return keywordForm->RemoveKeyword(a_keyword);
    }

    bool AddKeywordToRef(RE::StaticFunctionTag*, RE::TESObjectREFR* a_ref, RE::BGSKeyword* a_keyword) {
        if (!a_ref || !a_keyword) {
            return false;
        }
        auto baseForm = a_ref->GetBaseObject();
        if (!baseForm) {
            return false;
        }
        auto keywordForm = baseForm->As<RE::BGSKeywordForm>();
        if (!keywordForm) {
            return false;
        }
        return keywordForm->AddKeyword(a_keyword);
    }

    bool RemoveKeywordFromRef(RE::StaticFunctionTag*, RE::TESObjectREFR* a_ref, RE::BGSKeyword* a_keyword) {
        if (!a_ref || !a_keyword) {
            return false;
        }
        auto baseForm = a_ref->GetBaseObject();
        if (!baseForm) {
            return false;
        }
        auto keywordForm = baseForm->As<RE::BGSKeywordForm>();
        if (!keywordForm) {
            return false;
        }
        return keywordForm->RemoveKeyword(a_keyword);
    }

    bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm) {
        if (!a_vm) {
            return false;
        }

        bool ok = true;
        ok &= FactionRankHook::RegisterNativeFunctions(a_vm);
        ok &= RemoveFromFactionHook::RegisterNativeFunctions(a_vm);
        ok &= CraftHook::RegisterNativeFunctions(a_vm);
        ok &= QuestStageHook::RegisterNativeFunctions(a_vm);

        a_vm->RegisterFunction("AddKeyword", "Lau2_SKSEFunctions", AddKeyword);
        a_vm->RegisterFunction("RemoveKeyword", "Lau2_SKSEFunctions", RemoveKeyword);
        a_vm->RegisterFunction("AddKeywordToRef", "Lau2_SKSEFunctions", AddKeywordToRef);
        a_vm->RegisterFunction("RemoveKeywordFromRef", "Lau2_SKSEFunctions", RemoveKeywordFromRef);

        spdlog::info("PapyrusApi: native functions registered ({}).", ok ? "ok" : "with errors");
        return ok;
    }

    void ClearAllListeners() {
        FactionRankHook::ClearListeners();
        RemoveFromFactionHook::ClearListeners();
        CraftHook::ClearListeners();
        QuestStageHook::ClearListeners();
    }
}