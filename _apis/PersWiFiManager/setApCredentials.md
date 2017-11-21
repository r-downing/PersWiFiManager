---
title: PersWiFiManager::setApCredentials
description: Sets the SSID (and password) to be used for AP mode
api-class: PersWiFiManager

---



## Signatures

- `void setApCredentials(const String& apSsid)`
- `void setApCredentials(const String& apSsid, const String& apPass)`

## Args

- `const String& apSsid` SSID to be used for AP mode.
- `const String& apPass` (Optional) Password to be used for AP mode.

## Notes

Passwords less than 8 characters long will be ignored.
