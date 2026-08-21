#include "LoadStateTracker.h"

namespace LoadStateTracker {

    namespace {
        std::atomic_bool g_fullyLoaded = false;

        class MenuWatcher : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
        public:
            static MenuWatcher* GetSingleton() {
                static MenuWatcher singleton;
                return &singleton;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                                  RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
                if (a_event && a_event->menuName == RE::LoadingMenu::MENU_NAME) {
                    g_fullyLoaded = !a_event->opening;
                    spdlog::debug("LoadStateTracker: Loading Menu {} - fullyLoaded = {}.",
                                  a_event->opening ? "opened" : "closed", g_fullyLoaded.load());
                }
                return RE::BSEventNotifyControl::kContinue;
            }

            MenuWatcher(const MenuWatcher&) = delete;
            MenuWatcher(MenuWatcher&&) = delete;
            MenuWatcher& operator=(const MenuWatcher&) = delete;
            MenuWatcher& operator=(MenuWatcher&&) = delete;

        private:
            MenuWatcher() = default;
        };
    }

    bool IsGameFullyLoaded() { return g_fullyLoaded; }

    void Install() {
        if (auto* ui = RE::UI::GetSingleton()) {
            ui->AddEventSink(MenuWatcher::GetSingleton());
            spdlog::info("LoadStateTracker installed.");
        } else {
            spdlog::error("LoadStateTracker: failed to get UI singleton - dispatch gating will not work.");
        }
    }
}