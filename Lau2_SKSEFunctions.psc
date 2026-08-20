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
