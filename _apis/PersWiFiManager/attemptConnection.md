---
title: PersWiFiManager::attemptConnection
description: Attempts to auto-connect to last WiFi network used (or other if specified). Falls back to AP mode if unsuccessful.
api-class: PersWiFiManager
---



## Signatures

- `bool attemptConnection()`
- `bool attemptConnection(const String& ssid)`
- `bool attemptConnection(const String& ssid, const String& pass)`


## Args

- `const String& ssid` (Optional) SSID to connect to. This overrides the default behavior of connecting to the last network used.
- `const String& pass` (Optional) password for the specified SSID. Length  must be >= 8 characters

## Returns

- `true` if successfully autoconnected, false if switched to AP mode

## Notes

This function is already called inside [`begin`]({{site.baseurl}}/api/PersWiFiManager/begin/), or when WiFi settings are changed, but this can be used for manual operation.
