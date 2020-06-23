Scriptname CBP Hidden

Int Function GetScriptVersion() global
	return 1
EndFunction

Int Function GetVersion() native global

; Load and apply configuration from file
Function Reload() native global

; Set and apply a config value
Bool Function Update(String section, String key, Float value) native global

; Save the current config values to file
Function Save() native global