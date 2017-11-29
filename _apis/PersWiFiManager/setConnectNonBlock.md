---
title: PersWiFiManager::setConnectNonBlock
description: Sets the PersWiFiManager connecting actions to non-blocking mode
api-class: PersWiFiManager

---



## Signatures

- `void setConnectNonBlock(bool b)`

## Args

- `bool b` use `true` for non-blocking mode, `false` for regular mode

## Notes

In non-blocking mode, [`handleWiFi`]({{site.baseurl}}/api/PersWiFiManager/handleWiFi) must be called in the main loop