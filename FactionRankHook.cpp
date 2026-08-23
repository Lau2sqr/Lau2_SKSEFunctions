#include "FactionRankHook.h"
#include "LoadStateTracker.h"
#include <cstring>

namespace FactionRankHook {

    namespace {
        // This module's listener registry - owns everything needed to let
        // Papyrus scripts subscribe to the typed FactionRankChange event.
        Lau2_SKSEFunctions::PapyrusApi::ListenerRegistry<RE::Actor*, RE::TESFaction*, std::int32_t> g_listeners;

        bool RegisterForFactionRankChange(RE::StaticFunctionTag*, RE::TESForm* a_listener,
                                          RE::BSFixedString a_scriptName) {
            return g_listeners.Register(a_listener, a_scriptName);
        }

        bool UnregisterForFactionRankChange(RE::StaticFunctionTag*, RE::TESForm* a_listener,
                                            RE::BSFixedString a_scriptName) {
            return g_listeners.Unregister(a_listener, a_scriptName);
        }
    }

    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm) {
        a_vm->RegisterFunction("RegisterForFactionRankChange", "Lau2_SKSEFunctions", RegisterForFactionRankChange);
        a_vm->RegisterFunction("UnregisterForFactionRankChange", "Lau2_SKSEFunctions", UnregisterForFactionRankChange);
        return true;
    }

    void ClearListeners() { g_listeners.Clear(); }

    // Signature of the internal engine function that actually sets a faction rank.
    // Both Papyrus Actor.AddToFaction() and Actor.SetFactionRank() route through this.
    using SetFactionRank_t = void(RE::Actor*, RE::TESFaction*, std::int8_t);

    // Holds the address of the original function, reached via our manual trampoline.
    REL::Relocation<SetFactionRank_t> _SetFactionRank;

   void SetFactionRank_Hook(RE::Actor* a_actor, RE::TESFaction* a_faction, std::int8_t a_rank) {
        _SetFactionRank(a_actor, a_faction, a_rank);
        if (a_actor && a_faction && LoadStateTracker::IsGameFullyLoaded()) {
            g_listeners.Dispatch("OnFactionRankChanged", a_actor, a_faction, static_cast<std::int32_t>(a_rank));
        }
    }

    void Install() {
        // The function starts with "test rdx,rdx" (3 bytes) followed by a 6-byte
        // conditional jump. Both instructions must be preserved intact, so the
        // patch/copy size has to cover all 9 bytes rather than a plain 5-byte jmp.
        constexpr std::size_t patchSize = 9;

        // Address Library IDs (found via versiondb.h/versionlibdb.h reverse
        // lookup: SE 36669, AE 37677). Using RELOCATION_ID instead of raw
        // offsets means this now requires "Address Library for SKSE Plugins"
        // as a hard runtime dependency - CommonLibSSE-NG itself reports a
        // clear error (and stops loading) if it's missing or the ID isn't
        // found for the running version, rather than us silently skipping
        // hook installation.
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(36669, 37677)};
        const auto srcAddr = target.address();

        auto& trampoline = SKSE::GetTrampoline();

        // SKSE's write_branch() assumes the patched bytes already form a call/jmp
        // instruction, which isn't the case at an arbitrary function entry point.
        // Instead, build our own trampoline: copy the untouched original bytes,
        // append a jmp back to the real function body (srcAddr + patchSize).
        auto* mem = static_cast<std::uint8_t*>(trampoline.allocate(patchSize + 14));
        std::memcpy(mem, reinterpret_cast<void*>(srcAddr), patchSize);

        mem[patchSize + 0] = 0xFF;
        mem[patchSize + 1] = 0x25;
        *reinterpret_cast<std::int32_t*>(mem + patchSize + 2) = 0;
        *reinterpret_cast<std::uint64_t*>(mem + patchSize + 6) = static_cast<std::uint64_t>(srcAddr + patchSize);

        _SetFactionRank = reinterpret_cast<std::uintptr_t>(mem);

        // Now redirect the function entry itself to our hook.
        trampoline.write_branch<5>(srcAddr, SetFactionRank_Hook);

        spdlog::info("FactionRankHook installed at {:X}", srcAddr);
    }
}