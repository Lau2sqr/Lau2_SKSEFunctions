#include "QuestStageHook.h"

namespace QuestStageHook {

    namespace {
        struct Entry {
            RE::VMHandle handle;
            RE::BSFixedString scriptName;
            RE::FormID questFormID;
        };

        std::vector<Entry> g_listeners;
        std::mutex g_mutex;

        bool RegisterForQuestStage(RE::StaticFunctionTag*, RE::TESForm* a_listener, RE::BSFixedString a_scriptName,
                                   RE::TESQuest* a_quest) {
            if (!a_listener) {
                spdlog::warn("QuestStageHook::RegisterForQuestStage: akListener is None.");
                return false;
            }
            if (a_scriptName.empty()) {
                spdlog::warn("QuestStageHook::RegisterForQuestStage: asScriptName is empty.");
                return false;
            }
            if (!a_quest) {
                spdlog::warn("QuestStageHook::RegisterForQuestStage: akQuest is None.");
                return false;
            }

            const auto handle = Lau2_SKSEFunctions::PapyrusApi::Detail::GetHandleForListener(a_listener);
            if (handle == static_cast<RE::VMHandle>(0)) {
                spdlog::warn("QuestStageHook::RegisterForQuestStage: could not resolve VM handle for {:X}.",
                             a_listener->GetFormID());
                return false;
            }

            const auto questID = a_quest->GetFormID();

            std::scoped_lock lock(g_mutex);
            for (const auto& entry : g_listeners) {
                if (entry.handle == handle && entry.scriptName == a_scriptName && entry.questFormID == questID) {
                    return true;
                }
            }
            g_listeners.emplace_back(Entry{handle, a_scriptName, questID});
            spdlog::info("QuestStageHook: listener registered: {} ({:X}) for quest {:X}", a_scriptName.c_str(),
                         a_listener->GetFormID(), questID);
            return true;
        }

        bool UnregisterForQuestStage(RE::StaticFunctionTag*, RE::TESForm* a_listener, RE::BSFixedString a_scriptName,
                                     RE::TESQuest* a_quest) {
            if (!a_listener || !a_quest) {
                return false;
            }
            const auto handle = Lau2_SKSEFunctions::PapyrusApi::Detail::GetHandleForListener(a_listener);
            const auto questID = a_quest->GetFormID();

            std::scoped_lock lock(g_mutex);
            const auto before = g_listeners.size();
            std::erase_if(g_listeners, [&](const Entry& e) {
                return e.handle == handle && e.scriptName == a_scriptName && e.questFormID == questID;
            });
            return g_listeners.size() != before;
        }

        class QuestStageWatcher : public RE::BSTEventSink<RE::TESQuestStageEvent> {
        public:
            static QuestStageWatcher* GetSingleton() {
                static QuestStageWatcher singleton;
                return &singleton;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::TESQuestStageEvent* a_event,
                                                  RE::BSTEventSource<RE::TESQuestStageEvent>*) override {
                if (!a_event) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                std::scoped_lock lock(g_mutex);
                if (g_listeners.empty()) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                // Skip the lookup entirely if no one is even registered for this quest.
                const bool anyMatch =
                    std::ranges::any_of(g_listeners, [&](const Entry& e) { return e.questFormID == a_event->formID; });
                if (!anyMatch) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
                auto* questForm = RE::TESForm::LookupByID<RE::TESQuest>(a_event->formID);
                if (!vm || !questForm) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                for (const auto& entry : g_listeners) {
                    if (entry.questFormID != a_event->formID) {
                        continue;
                    }

                    auto* funcArgs = RE::MakeFunctionArguments<RE::TESQuest*, std::int32_t>(
                        std::move(questForm), static_cast<std::int32_t>(a_event->stage));

                    RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
                    const bool dispatched = vm->DispatchMethodCall(entry.handle, entry.scriptName, "OnQuestStageChange",
                                                                   funcArgs, callback);

                    spdlog::trace("QuestStageHook::Dispatch: '{}' -> quest {:X} stage {} ({}).",
                                 entry.scriptName.c_str(), a_event->formID, a_event->stage,
                                 dispatched ? "ok" : "failed");
                }

                return RE::BSEventNotifyControl::kContinue;
            }

            QuestStageWatcher(const QuestStageWatcher&) = delete;
            QuestStageWatcher(QuestStageWatcher&&) = delete;
            QuestStageWatcher& operator=(const QuestStageWatcher&) = delete;
            QuestStageWatcher& operator=(QuestStageWatcher&&) = delete;

        private:
            QuestStageWatcher() = default;
        };
    }

    bool RegisterNativeFunctions(RE::BSScript::IVirtualMachine* a_vm) {
        a_vm->RegisterFunction("RegisterForQuestStage", "Lau2_SKSEFunctions", RegisterForQuestStage);
        a_vm->RegisterFunction("UnregisterForQuestStage", "Lau2_SKSEFunctions", UnregisterForQuestStage);
        return true;
    }

    void ClearListeners() {
        std::scoped_lock lock(g_mutex);
        if (!g_listeners.empty()) {
            spdlog::info("QuestStageHook: clearing {} listener(s) (new game / load game).", g_listeners.size());
        }
        g_listeners.clear();
    }

    void Install() {
        if (auto* holder = RE::ScriptEventSourceHolder::GetSingleton()) {
            holder->AddEventSink<RE::TESQuestStageEvent>(QuestStageWatcher::GetSingleton());
            spdlog::info("QuestStageHook installed (native TESQuestStageEvent sink, no trampoline required).");
        } else {
            spdlog::error("QuestStageHook: failed to get ScriptEventSourceHolder singleton - hook will not work.");
        }
    }
}