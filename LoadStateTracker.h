#pragma once

namespace LoadStateTracker {
    // True once the Loading Menu has closed after game/save load. False
    // while a loading screen is active. Used to suppress event dispatch
    // during initial actor/world setup (e.g. SetFactionRank being called
    // for every NPC's starting factions), which is not a "real" runtime
    // change and would otherwise flood the Papyrus VM call queue.
    bool IsGameFullyLoaded();

    // Registers the UI event sink. Call once from SKSEPluginLoad.
    void Install();
}