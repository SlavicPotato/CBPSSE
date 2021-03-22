Scriptname CBP Hidden

Int Function GetScriptVersion() global
    return 4
EndFunction

Int Function GetVersion() native global

Function UpdateAllActors() native global
Function ResetAllActors() native global

; Set and apply config value globally
Bool Function SetGlobalConfig(String section, String key, Float value) native global

; Set and apply config value to actor
Bool Function SetActorConfig(Actor actor, String section, String key, Float value) native global

Function OpenUI(Bool bOpen) native global
