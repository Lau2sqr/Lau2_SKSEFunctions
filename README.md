# Lau2_SKSEFunctions

A native SKSE plugin for Skyrim Special Edition / Anniversary Edition that exposes engine-internal events to Papyrus which are not otherwise accessible — in the style of PO3's Papyrus Extender and Dylbill's Papyrus Functions.

Currently, this plugin adds four typed, native Papyrus events related to faction membership (**faction rank changes** and **faction removal**), crafting (**item crafted**), and quest progression (**quest stage changed**), plus a small set of keyword utility functions.

## Features

- `OnFactionRankChanged` — fires whenever an actor's rank in a faction changes (covers both `Actor.AddToFaction()` and `Actor.SetFactionRank()`, since both route through the same internal engine function)
- `OnFactionRemoved` — fires whenever an actor is removed from a faction via `Actor.RemoveFromFaction()`
- `OnItemCrafted` — fires whenever an item is crafted at a Smithing, Tempering, Enchanting, or Alchemy workbench (including custom `COBJ` recipes on repurposed furniture, e.g. cooking pots)
- `OnQuestStageChange` — fires whenever a specific quest you're watching changes stage, filtered per-quest so you only receive events for the quest you registered for
- `AddKeyword` / `RemoveKeyword` — add or remove a keyword on a base Form (e.g. an NPC's `TESNPC` record)
- `AddKeywordToRef` / `RemoveKeywordFromRef` — same, but for an `ObjectReference` directly (resolves the reference's base object internally)
- Fully typed native API — no FormID-string workarounds, real `Actor`/`Faction`/`Quest`/`Form` objects passed directly to Papyrus
- Listener registry automatically cleaned up on new game / load game, so no dangling or duplicate registrations across save loads

## Requirements

- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) — **hard dependency**. Without it, or on an unsupported game version, the plugin will fail to load (CommonLibSSE-NG reports this clearly; it will not crash your game).

## Supported Game Versions

| Version | Status |
|---|---|
| AE 1.7.99 | Verified in-game |
| AE 1.6.1170 | Verified in-game |
| SE 1.5.97 | Verified in-game |
| Other AE builds (1.6.640, 1.6.1130, etc.) | **Not supported** (intentionally — not enough demand to justify additional offset research) |
| VR | Not supported |

## Papyrus API

Add `Lau2_SKSEFunctions` as a native script (the `.psc`/`.pex` is included in the download) and call these functions from any script.

**Important:** for all four events below, the second argument to `RegisterFor...` is the **exact class name of the script attached to `akListener`** (e.g. the name of the script you're writing this in) — not the event name. The plugin always calls its own fixed event name (`OnFactionRankChanged`, `OnFactionRemoved`, `OnItemCrafted`, `OnQuestStageChange`) on that script.

### Faction Rank Changed

```papyrus
Scriptname MyQuestScript extends Quest

Event OnInit()
    Lau2_SKSEFunctions.RegisterForFactionRankChange(Self, "MyQuestScript")
EndEvent

Event OnPlayerLoadGame()
    Lau2_SKSEFunctions.RegisterForFactionRankChange(Self, "MyQuestScript")
EndEvent

Event OnFactionRankChanged(Actor akActor, Faction akFaction, Int aiNewRank)
    ; your logic here
EndEvent
```

To stop listening: `Lau2_SKSEFunctions.UnregisterForFactionRankChange(Self, "MyQuestScript")`

### Faction Removed

```papyrus
Scriptname MyQuestScript extends Quest

Event OnInit()
    Lau2_SKSEFunctions.RegisterForFactionRemoved(Self, "MyQuestScript")
EndEvent

Event OnPlayerLoadGame()
    Lau2_SKSEFunctions.RegisterForFactionRemoved(Self, "MyQuestScript")
EndEvent

Event OnFactionRemoved(Actor akActor, Faction akFaction)
    ; your logic here
EndEvent
```

To stop listening: `Lau2_SKSEFunctions.UnregisterForFactionRemoved(Self, "MyQuestScript")`

### Item Crafted

```papyrus
Scriptname MyQuestScript extends Quest

Event OnInit()
    Lau2_SKSEFunctions.RegisterForItemCrafted(Self, "MyQuestScript")
EndEvent

Event OnPlayerLoadGame()
    Lau2_SKSEFunctions.RegisterForItemCrafted(Self, "MyQuestScript")
EndEvent

Event OnItemCrafted(ObjectReference akBench, Location akLocation, Form akCreatedItem, Int aiWorkbenchType)
    ; aiWorkbenchType: 0 = Smithing, 1 = Tempering, 2 = Enchanting, 3 = Alchemy
    ; your logic here
EndEvent
```

To stop listening: `Lau2_SKSEFunctions.UnregisterForItemCrafted(Self, "MyQuestScript")`

`akCreatedItem` is the crafted item's **base Form**, not the specific inventory instance — if your mod needs to distinguish between individually crafted copies of the same item, you'll need to resolve the instance yourself.

`aiWorkbenchType` reflects the internal crafting menu class used, which does not always match the physical furniture's real-world appearance — e.g. a custom `COBJ` recipe placed on a cooking pot may report `0` (Smithing) rather than `3` (Alchemy), depending on which menu class the recipe is wired to. If your mod relies on a specific workbench, verify the reported `aiWorkbenchType` empirically for your recipe rather than assuming it based on the furniture type.

### Quest Stage Changed

```papyrus
Scriptname MyQuestScript extends Quest

Quest Property QuestToWatch Auto

Event OnInit()
    Lau2_SKSEFunctions.RegisterForQuestStage(Self, "MyQuestScript", QuestToWatch)
EndEvent

Event OnPlayerLoadGame()
    Lau2_SKSEFunctions.RegisterForQuestStage(Self, "MyQuestScript", QuestToWatch)
EndEvent

Event OnQuestStageChange(Quest akQuest, Int aiNewStage)
    ; your logic here
EndEvent
```

To stop listening: `Lau2_SKSEFunctions.UnregisterForQuestStage(Self, "MyQuestScript", QuestToWatch)`

Unlike the other three events, this one is **filtered per registration** — you pass the specific `Quest` you want to watch, and your script only receives `OnQuestStageChange` calls for that quest, not for every quest stage change in the entire game. You can register for multiple quests from the same script by calling `RegisterForQuestStage` once per quest.

This is the native replacement for PO3 Papyrus Extender's `RegisterForQuestStage`/`OnQuestStageChange` (`PO3_Events_Form`).

### Notes

- Re-registering in `OnPlayerLoadGame()` is required for all four events — listener handles are tied to the VM session and become invalid across save loads.
- `OnFactionRankChanged`/`OnFactionRemoved` fire for the player and for NPCs, including companion/follower recruitment and dismissal (which is implemented internally as a rank change, not add/remove).
- `OnFactionRankChanged` is automatically suppressed while the game is loading (New Game or Load Game) — the engine itself calls `SetFactionRank` for essentially every NPC with starting factions during initial world setup, which would otherwise flood your listener with thousands of irrelevant calls in a few seconds. The underlying engine behavior is unaffected either way; only the Papyrus event is held back until loading finishes. `OnFactionRemoved`, `OnItemCrafted`, and `OnQuestStageChange` are not gated this way, since none of them happen in bulk during loading.
- If a script attached to an `extends Quest` script doesn't reliably receive `OnPlayerLoadGame`, that's a known base-game Papyrus quirk unrelated to this plugin — `OnPlayerLoadGame` is an Actor event and is only guaranteed to fire on a `ReferenceAlias` filled with the player (or a Magic Effect on the player), not on a bare Quest script. Route your re-registration through such an alias if you're seeing this.

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

**Note:** all four keyword functions — including the `...ToRef`/`...FromRef` variants — ultimately write the keyword to the **base Form**, not to an individual instance. There is no engine-level way to attach a keyword to a single item instance while leaving other copies of the same base Form unaffected.

Unlike the faction/crafting events above, the keyword functions don't hook anything — they're thin wrappers around CommonLibSSE-NG's own `BGSKeywordForm::AddKeyword()`/`RemoveKeyword()`. That means they're **automatically version-independent**: no `RELOCATION_ID`, no per-version verification needed. `OnQuestStageChange` shares this property, for a different reason: it subscribes to a native CommonLibSSE-NG event source rather than patching an internal engine function, so it also needs no `RELOCATION_ID` or per-version offset work.

## Installation

Install with your mod manager of choice (MO2, Vortex) like any other SKSE plugin. Make sure Address Library is installed for your game version.

## Known Limitations

- Only the game versions listed above are supported.
- No VR support.
- Keywords (see above) always apply to the base Form, never to a single item instance.

## Credits

- [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) (1.5.97 / 1.6.1170 builds)
- [CommonLibSSE-NG (alandtse fork)](https://github.com/alandtse/CommonLibSSE-NG) (1.7.99 build — GPL-3.0-or-later)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) by meh321
- Ghidra, used for reverse-engineering the internal engine functions this plugin hooks
- Hook/event patterns based on [powerofthree's Papyrus Extender](https://github.com/powerof3/PapyrusExtenderSSE) (MIT License)

## Source

https://github.com/Lau2sqr/Lau2_SKSEFunctions

## License

GPL-3.0-or-later — see `LICENSE` for details.

Earlier releases of this plugin were MIT-licensed. As of the 1.7.99-compatible build, this plugin links against a GPL-3.0-or-later-licensed fork of CommonLibSSE-NG, and the resulting combined work is therefore distributed under GPL-3.0-or-later. Calling this plugin's Papyrus functions from your own mod's scripts does not, by itself, require your mod to be GPL-licensed — this only applies to code that directly links against this plugin's own source.