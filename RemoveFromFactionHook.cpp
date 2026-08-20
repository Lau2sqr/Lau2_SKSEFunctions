#include "RemoveFromFactionHook.h"

#include <cstring>

namespace RemoveFromFactionHook {

    namespace {
        FactionRankHook::PapyrusApi::ListenerRegistry<RE::Actor*, RE::TESFaction*> g_listeners;

        bool RegisterForFactionRemoved(RE::StaticFunctionTag*, RE::TESForm* a_listener,
                                       RE::BSFixedString a_scriptName) {
            return g_listeners.Register(a_listener, a_scriptName);
        }

        bool UnregisterForFactionRemoved(RE::StaticFunctionTag*, RE::TESForm* a_listener,
                                         RE::BSFixedString a_scriptName) {
            return g_listeners.Unregister(a_listener, a_scriptName);
        }
    }

    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm) {
        a_vm->RegisterFunction("RegisterForFactionRemoved", "Lau2_SKSEFunctions", RegisterForFactionRemoved);
        a_vm->RegisterFunction("UnregisterForFactionRemoved", "Lau2_SKSEFunctions", UnregisterForFactionRemoved);
        return true;
    }

    void ClearListeners() { g_listeners.Clear(); }

    // Signature confirmed via Ghidra (AE 1.6.1170): this is the thunk that
    // Skyrim's native function table calls directly for RemoveFromFaction
    // (single caller, unlike the shared helper it tail-jumps to). Standard
    // engine-native calling convention: (vm, stackID, actor, faction) in
    // RCX, RDX, R8, R9. The thunk itself reassigns R9->RDX, R8->RCX before
    // jumping onward, which is exactly what our copied trampoline bytes
    // below replicate.
    using RemoveFromFaction_t = void(void*, std::uint32_t, RE::Actor*, RE::TESFaction*);

    REL::Relocation<RemoveFromFaction_t> _RemoveFromFaction;

    void RemoveFromFaction_Hook(void* a_vm, std::uint32_t a_stackID, RE::Actor* a_actor, RE::TESFaction* a_faction) {
        _RemoveFromFaction(a_vm, a_stackID, a_actor, a_faction);
        if (a_actor && a_faction) {
            g_listeners.Dispatch("OnFactionRemoved", a_actor, a_faction);
        }
    }

    void Install() {
        // Thunk body: MOV RDX,R9 (3B) + MOV RCX,R8 (3B) + JMP rel32 (5B).
        // Clean instruction boundary at 6 bytes (after both MOVs), which is
        // >=5 so a jmp fits there without cutting an instruction in half.
        constexpr std::size_t patchSize = 6;

        // Address Library IDs for the RemoveFromFaction thunk (found via
        // versiondb.h/versionlibdb.h reverse lookup: SE 54161, AE 54958).
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(54161, 54958)};
        const auto srcAddr = target.address();

        auto& trampoline = SKSE::GetTrampoline();

        // Manual trampoline: copy the untouched original bytes, append a jmp
        // back to the real thunk body (srcAddr + patchSize).
        auto* mem = static_cast<std::uint8_t*>(trampoline.allocate(patchSize + 14));
        std::memcpy(mem, reinterpret_cast<void*>(srcAddr), patchSize);

        mem[patchSize + 0] = 0xFF;
        mem[patchSize + 1] = 0x25;
        *reinterpret_cast<std::int32_t*>(mem + patchSize + 2) = 0;
        *reinterpret_cast<std::uint64_t*>(mem + patchSize + 6) = static_cast<std::uint64_t>(srcAddr + patchSize);

        _RemoveFromFaction = reinterpret_cast<std::uintptr_t>(mem);
        trampoline.write_branch<5>(srcAddr, RemoveFromFaction_Hook);

        spdlog::info("RemoveFromFactionHook installed at {:X}", srcAddr);
    }
}