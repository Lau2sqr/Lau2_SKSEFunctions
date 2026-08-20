# Lau2_SKSEFunctions

A native SKSE plugin for Skyrim Special Edition / Anniversary Edition that exposes engine-internal events to Papyrus which are not otherwise accessible — in the style of PO3's Papyrus Extender and Dylbill's Papyrus Functions.

Currently, this plugin adds two typed, native Papyrus events related to faction membership (**faction rank changes** and **faction removal**), plus a small set of keyword utility functions.

## Features

- `OnFactionRankChanged` — fires whenever an actor's rank in a faction changes (covers both `Actor.AddToFaction()` and `Actor.SetFactionRank()`, since both route through the same internal engine function)
- `OnFactionRemoved` — fires whenever an actor is removed from a faction via `Actor.RemoveFromFaction()`
- `AddKeyword` / `RemoveKeyword` — add or remove a keyword on a base Form (e.g. an NPC's `TESNPC` record)
- `AddKeywordToRef` / `RemoveKeywordFromRef` — same, but for an `ObjectReference` directly (resolves the reference's base object internally)
- Fully typed native API — no FormID-string workarounds, real `Actor`/`Faction` objects passed directly to Papyrus
- Listener registry automatically cleaned up on new game / load game, so no dangling or duplicate registrations across save loads

## Requirements

- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) — **hard dependency**. Without it, or on an unsupported game version, the plugin will fail to load (CommonLibSSE-NG reports this clearly; it will not crash your game).

## Supported Game Versions

| Version | Status |
|---|---|
| AE 1.6.1170 | Verified in-game |
| SE 1.5.97 | Verified in-game |
| Other AE builds (1.6.640, 1.6.1130, etc.) | **Not supported** (intentionally — not enough demand to justify additional offset research) |
| VR | Not supported |

## Papyrus API

Add `Lau2_SKSEFunctions` as a native script (the `.psc`/`.pex` is included in the download) and call these functions from any script.

### Faction Rank Changed

```papyrus
Event OnInit()
    Lau2_SKSEFunctions.RegisterForFactionRankChange(Self, "OnFactionRankChanged")
EndEvent

Event OnPlayerLoadGame()
    Lau2_SKSEFunctions.RegisterForFactionRankChange(Self, "OnFactionRankChanged")
EndEvent

Event OnFactionRankChanged(Actor akActor, Faction akFaction, Int aiNewRank)
    ; your logic here
EndEvent
```

To stop listening: `Lau2_SKSEFunctions.UnregisterForFactionRankChange(Self)`

### Faction Removed

```papyrus
Event OnInit()
    Lau2_SKSEFunctions.RegisterForFactionRemoved(Self, "OnFactionRemoved")
EndEvent

Event OnPlayerLoadGame()
    Lau2_SKSEFunctions.RegisterForFactionRemoved(Self, "OnFactionRemoved")
EndEvent

Event OnFactionRemoved(Actor akActor, Faction akFaction)
    ; your logic here
EndEvent
```

To stop listening: `Lau2_SKSEFunctions.UnregisterForFactionRemoved(Self)`

### Notes

- Re-registering in `OnPlayerLoadGame()` is required — listener handles are tied to the VM session and become invalid across save loads.
- Both events fire for the player and for NPCs, including companion/follower recruitment and dismissal (which is implemented internally as a rank change, not add/remove).

### Keywords

```papyrus
Bool Function AddKeyword(Form akForm, Keyword akKeyword) Global Native
Bool Function RemoveKeyword(Form akForm, Keyword akKeyword) Global Native
Bool Function AddKeywordToRef(ObjectReference akRef, Keyword akKeyword) Global Native
Bool Function RemoveKeywordFromRef(ObjectReference akRef, Keyword akKeyword) Global Native
```

Example:

```papyrus
Lau2_SKSEFunctions.AddKeywordToRef(akSpouseRef, MySpouseKeyword)
```

`AddKeyword`/`RemoveKeyword` expect a base Form (e.g. `akActor.GetLeveledActorBase()`) — passing an `ObjectReference`/`Actor` directly will return `false`, since keywords live on the base object, not the reference. Use the `...ToRef`/`...FromRef` variants instead if you're working with a reference.

Unlike the two faction events above, these four functions don't hook anything — they're thin wrappers around CommonLibSSE-NG's own `BGSKeywordForm::AddKeyword()`/`RemoveKeyword()`. That means they're **automatically version-independent**: no `RELOCATION_ID`, no per-version verification needed.

## Installation

Install with your mod manager of choice (MO2, Vortex) like any other SKSE plugin. Make sure Address Library is installed for your game version.

## Known Limitations

- Only the two game versions listed above are supported.
- No VR support.

## Credits

- [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) by meh321
- Ghidra, used for reverse-engineering the internal engine functions this plugin hooks
- Keyword function pattern based on [powerofthree's Papyrus Extender](https://github.com/powerof3/PapyrusExtenderSSE) (MIT License)

## Source

https://github.com/Lau2sqr/Lau2_SKSEFunctions

## License

MIT — see `LICENSE` for details. Free to use in any mod, credit appreciated but not required.
