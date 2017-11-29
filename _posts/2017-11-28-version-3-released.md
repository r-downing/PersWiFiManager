---
title: Version 3 Released! New Features Detailed
---

# New Features in Version 3

## Non-Blocking Connection and Disconnection

Set the PersWiFiManager's connecting actions to non-blocking mode with [setConnectNonBlock]({{site.baseurl}}/api/PersWiFiManager/setConnectNonBlock).

Then just call [handleWiFi]({{site.baseurl}}/api/PersWiFiManager/handleWiFi) in the main loop, instead of having your program wait for WiFi to finish connecting.

## WiFi Connection / AP Handlers

Set custom code or functions to run when wifi settings change with [onConnect]({{site.baseurl}}/api/PersWiFiManager/onConnect) and [onAp]({{site.baseurl}}/api/PersWiFiManager/onAp).

## Manual AP mode control

Manually enter AP mode in the program via the [startApMode]({{site.baseurl}}/api/PersWiFiManager/startApMode) function, or a new button in the web interface.