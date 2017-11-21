---
title: PersWiFiManager::begin
description: Begins automatic WiFi Configuration
api-class: PersWiFiManager

---



## Signatures

- `bool begin()`
- `bool begin(const String& ssid)`
- `bool begin(const String& ssid, const String& pass)`

## Args

- `const String& ssid` (Optional) SSID to connect to. This overrides the default behavior of connecting to the last network used.
- `const String& pass` (Optional) password for the specified SSID. Length  must be >= 8 characters

## Returns

- `true` if successfully autoconnected

## Notes

This sets up the server handlers for WiFi actions, then attempts to autoconnect. If no SSID is specified, it automatically attempts to connect to the last network used.