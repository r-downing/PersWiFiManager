---
title: PersWiFiManager::handleWiFi
description: Handles WiFi connecting/disconnecting in non-blocking mode
api-class: PersWiFiManager

---

## Signatures

- `void handleWiFi()`

## Notes

This function does not need to be called in normal mode. 

If the PersWiFiManager is set to non-blocking mode with [`setConnectNonBlock`]({{site.baseurl}}/api/PersWiFiManager/setConnectNonBlock), this must be called in the main loop.
