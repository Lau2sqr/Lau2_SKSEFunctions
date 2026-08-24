Scriptname Lau2_SKSEFunctions Hidden

; Registers akListener for the FactionRankChange event. akListener is
; typically a Quest (Self inside a Quest script). asScriptName must match
; the exact script name attached to akListener that implements:
;
;   Event OnFactionRankChanged(Actor akActor, Faction akFaction, Int aiNewRank)
;
; Returns false if akListener is None, asScriptName is empty, or the VM
; handle for akListener could not be resolved.
Bool Function RegisterForFactionRankChange(Form akListener, String asScriptName) Global Native

; Removes a previously registered listener. Returns false if no matching
; registration was found.
Bool Function UnregisterForFactionRankChange(Form akListener, String asScriptName) Global Native

; Registers akListener for the FactionRemoved event, fired when an actor is
; actually removed from a faction (not just a rank change). akListener's
; script must implement:
;
;   Event OnFactionRemoved(Actor akActor, Faction akFaction)
Bool Function RegisterForFactionRemoved(Form akListener, String asScriptName) Global Native

; Removes a previously registered FactionRemoved listener.
Bool Function UnregisterForFactionRemoved(Form akListener, String asScriptName) Global Native

; Adds akKeyword to akForm at runtime (e.g. NPCs, weapons, armor - any form
; that supports keywords). Returns false if akForm or akKeyword is None, or
; if akForm does not support keywords.
Bool Function AddKeyword(Form akForm, Keyword akKeyword) Global Native

; Removes akKeyword from akForm at runtime. Returns false if akForm or
; akKeyword is None, or if akForm does not support keywords.
Bool Function RemoveKeyword(Form akForm, Keyword akKeyword) Global Native

; Ref-variant of AddKeyword - resolves akRef's base object internally, so
; any ObjectReference (e.g. an Actor) can be passed directly.
Bool Function AddKeywordToRef(ObjectReference akRef, Keyword akKeyword) Global Native

; Ref-variant of RemoveKeyword. See AddKeywordToRef.
Bool Function RemoveKeywordFromRef(ObjectReference akRef, Keyword akKeyword) Global Native

; Registers akListener for the ItemCrafted event (all four workbenches).
; Listener script must implement:
;   Event OnItemCrafted(ObjectReference akBench, Location akLocation, Form akCreatedItem, Int aiWorkbenchType)
; aiWorkbenchType: 0=Smithing, 1=Tempering, 2=Enchanting, 3=Alchemy
Bool Function RegisterForItemCrafted(Form akListener, String asScriptName) Global Native
Bool Function UnregisterForItemCrafted(Form akListener, String asScriptName) Global Native

; Registers akListener for stage changes of akQuest specifically (not all
; quests in the game). Listener script must implement:
;   Event OnQuestStageChange(Quest akQuest, Int aiNewStage)
Bool Function RegisterForQuestStage(Form akListener, String asScriptName, Quest akQuest) Global Native
Bool Function UnregisterForQuestStage(Form akListener, String asScriptName, Quest akQuest) Global Native