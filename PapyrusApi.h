#pragma once

#include <mutex>
#include <utility>
#include <vector>

namespace Lau2_SKSEFunctions::PapyrusApi {

    namespace Detail {
        RE::VMHandle GetHandleForListener(RE::TESForm* a_form);
    }

    // Generic listener registry for typed native -> Papyrus event dispatch.
    // Each hook module owns one instance (FactionRankHook, RemoveFromFactionHook,
    // CraftHook), wires Register()/Unregister() to its native functions, and
    // calls Dispatch() from its C++ hook.
    template <typename... Args>
    class ListenerRegistry {
    public:
        bool Register(RE::TESForm* a_listener, RE::BSFixedString a_scriptName) {
            if (!a_listener) {
                spdlog::warn("ListenerRegistry::Register: akListener is None.");
                return false;
            }
            if (a_scriptName.empty()) {
                spdlog::warn("ListenerRegistry::Register: asScriptName is empty.");
                return false;
            }

            const auto handle = Detail::GetHandleForListener(a_listener);
            if (handle == static_cast<RE::VMHandle>(0)) {
                spdlog::warn("ListenerRegistry::Register: could not resolve VM handle for {:X}.",
                             a_listener->GetFormID());
                return false;
            }

            std::scoped_lock lock(_mutex);
            for (const auto& entry : _listeners) {
                if (entry.handle == handle && entry.scriptName == a_scriptName) {
                    return true;
                }
            }
            _listeners.emplace_back(Entry{handle, a_scriptName});
            spdlog::info("Listener registered: {} ({:X})", a_scriptName.c_str(), a_listener->GetFormID());
            return true;
        }

        bool Unregister(RE::TESForm* a_listener, RE::BSFixedString a_scriptName) {
            if (!a_listener) {
                return false;
            }
            const auto handle = Detail::GetHandleForListener(a_listener);

            std::scoped_lock lock(_mutex);
            const auto before = _listeners.size();
            std::erase_if(_listeners,
                          [&](const Entry& e) { return e.handle == handle && e.scriptName == a_scriptName; });
            return _listeners.size() != before;
        }

        void Dispatch(const char* a_eventName, Args... a_args) {
            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            if (!vm) {
                return;
            }

            std::scoped_lock lock(_mutex);
            if (_listeners.empty()) {
                return;
            }

            for (const auto& entry : _listeners) {
                auto* funcArgs = RE::MakeFunctionArguments<Args...>(std::move(a_args)...);

                RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                const bool dispatched =
                    vm->DispatchMethodCall(entry.handle, entry.scriptName, a_eventName, funcArgs, callback);

                spdlog::trace("ListenerRegistry::Dispatch: '{}' -> '{}' ({}).", a_eventName, entry.scriptName.c_str(),
                              dispatched ? "ok" : "failed");
            }
        }

        void Clear() {
            std::scoped_lock lock(_mutex);
            if (!_listeners.empty()) {
                spdlog::info("ListenerRegistry: clearing {} listener(s) (new game / load game).", _listeners.size());
            }
            _listeners.clear();
        }

    private:
        struct Entry {
            RE::VMHandle handle;
            RE::BSFixedString scriptName;
        };

        std::vector<Entry> _listeners;
        std::mutex _mutex;
    };

    // Central Papyrus registration hub - every hook module registers its
    // native functions here, called once from SKSEPluginLoad via:
    //   SKSE::GetPapyrusInterface()->Register(Lau2_SKSEFunctions::PapyrusApi::RegisterFunctions);
    bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm);

    // Clears every hook module's listener registry. Wired to SKSE's
    // kNewGame/kPostLoadGame messages in plugin.cpp.
    void ClearAllListeners();
}