#include "CraftHook.h"

namespace CraftHook {

    namespace {
        // Story event struct, verified via po3's PapyrusExtenderSSE (MIT).
        struct StoryItemCraft {
            RE::ObjectRefHandle objectHandle;  // 00
            RE::BGSLocation* location;         // 08
            RE::TESForm* form;                 // 10 - base form, not instance
        };
        static_assert(sizeof(StoryItemCraft) == 0x18);

        using CraftFunc_t = StoryItemCraft* (*)(StoryItemCraft*, RE::TESObjectREFR*, RE::BGSLocation*, RE::TESForm*);

        enum class WorkbenchType : std::uint8_t { kSmithing = 0, kTempering = 1, kEnchanting = 2, kAlchemy = 3 };

       Lau2_SKSEFunctions::PapyrusApi::ListenerRegistry<RE::TESObjectREFR*, RE::BGSLocation*, RE::TESForm*, std::int32_t> g_listeners;

        bool RegisterForItemCrafted(RE::StaticFunctionTag*, RE::TESForm* a_listener, RE::BSFixedString a_scriptName) {
            return g_listeners.Register(a_listener, a_scriptName);
        }

        bool UnregisterForItemCrafted(RE::StaticFunctionTag*, RE::TESForm* a_listener, RE::BSFixedString a_scriptName) {
            return g_listeners.Unregister(a_listener, a_scriptName);
        }

        void HandleCraftedItem(WorkbenchType a_type, RE::TESObjectREFR* a_bench, RE::BGSLocation* a_loc,
                               RE::TESForm* a_form) {
            if (a_form) {
                g_listeners.Dispatch("OnItemCrafted", a_bench, a_loc, a_form, static_cast<std::int32_t>(a_type));
            }

            // Smithing-only quality tagging (placeholder, WIP).
            if (a_type != WorkbenchType::kSmithing) {
                return;
            }

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player || !a_form) {
                return;
            }

            spdlog::trace("CraftHook: smithing craft event, base form {:X}", a_form->GetFormID());
        }

        // One instantiation per call site - each needs its own _Original.
        template <std::size_t N>
        struct Hook {
            static inline REL::Relocation<CraftFunc_t> _Original;

            static StoryItemCraft* Thunk(StoryItemCraft* a_event, RE::TESObjectREFR* a_bench, RE::BGSLocation* a_loc,
                                         RE::TESForm* a_form) {
                spdlog::trace("CraftHook: Thunk<{}> fired", N);
                HandleCraftedItem(static_cast<WorkbenchType>(N), a_bench, a_loc, a_form);
                return _Original(a_event, a_bench, a_loc, a_form);
            }

            static void Install(std::uint32_t a_idSE, std::uint32_t a_idAE, std::ptrdiff_t a_offsetSE,
                                std::ptrdiff_t a_offsetAE) {
                const auto offset = REL::Module::IsAE() ? a_offsetAE : a_offsetSE;
                REL::Relocation<std::uintptr_t> target{RELOCATION_ID(a_idSE, a_idAE), offset};

                auto& trampoline = SKSE::GetTrampoline();
                _Original = trampoline.write_call<5>(target.address(), Thunk);

                spdlog::info("CraftHook[{}] installed at {:X}", N, target.address());
            }
        };
    }

    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm) {
        a_vm->RegisterFunction("RegisterForItemCrafted", "Lau2_SKSEFunctions", RegisterForItemCrafted);
        a_vm->RegisterFunction("UnregisterForItemCrafted", "Lau2_SKSEFunctions", UnregisterForItemCrafted);
        return true;
    }

    void ClearListeners() { g_listeners.Clear(); }

    void Install() {
        // IDs/offsets from po3's PapyrusExtenderSSE (MIT), Smithing/
        // Tempering swapped vs. po3's labels - verified via Thunk<N>
        // logging on AE 1.6.1170 / SE 1.5.97.
        Hook<0>::Install(50476, 51369, 0x11E, 0x227);  // Smithing
        Hook<1>::Install(50477, 51370, 0x17D, 0x1B3);  // Tempering
        Hook<2>::Install(50450, 51355, 0x2FC, 0x2FA);  // Enchanting
        Hook<3>::Install(50449, 51354, 0x29E, 0x296);  // Alchemy
    }
}