#include <spdlog/sinks/basic_file_sink.h>

#include "FactionRankHook.h"
#include "PapyrusApi.h"
#include "RemoveFromFactionHook.h"
#include "LoadStateTracker.h"

// Sets up file-based logging for this plugin under
// Documents/My Games/Skyrim Special Edition/SKSE/Lau2_SKSEFunctions.log
void InitializeLogging() {
    auto path = SKSE::log::log_directory();
    if (!path) {
        return;
    }
    *path /= "Lau2_SKSEFunctions.log";

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%v");
}

// Registered listener VM handles only stay valid for the save/session they
// were created in. Without this, listeners registered before a load would
// keep dispatching to stale (possibly reused) handles after loading.
void OnSKSEMessage(SKSE::MessagingInterface::Message* a_msg) {
    switch (a_msg->type) {
        case SKSE::MessagingInterface::kNewGame:
        case SKSE::MessagingInterface::kPostLoadGame:
            FactionRankHook::PapyrusApi::ClearAllListeners();
            break;
        default:
            break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    InitializeLogging();

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(256);

    LoadStateTracker::Install();
    FactionRankHook::Install();
    RemoveFromFactionHook::Install();

    if (!SKSE::GetPapyrusInterface()->Register(FactionRankHook::PapyrusApi::RegisterFunctions)) {
        spdlog::error("Failed to register Papyrus functions - typed API will not be available.");
    }

    if (auto* messaging = SKSE::GetMessagingInterface()) {
        messaging->RegisterListener(OnSKSEMessage);
    } else {
        spdlog::error("Failed to get SKSE messaging interface - listener registry will not reset on load.");
    }

    spdlog::info("Lau2_SKSEFunctions plugin loaded (hooks: FactionRankChange, RemoveFromFaction).");

    return true;
}