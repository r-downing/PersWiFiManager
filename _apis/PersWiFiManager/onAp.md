---
title: PersWiFiManager::onAp
description: Attach a handler function that runs every time AP mode is started
api-class: PersWiFiManager

---

## Signatures

- `void onAp(WiFiChangeHandlerFunction fn)`

## Args

- `WiFiChangeHandlerFunction fn` A generic function pointer to an AP mode handler function


## Notes

This will run the handler function every time AP mode is started, when WiFi fails to connect or AP mode is manually triggered.

For successful connection handler, see [`onConnect`]({{site.baseurl}}/api/PersWiFiManager/onConnect)

## Example Code

```cpp
void setup(){
	...

	//uses a lambda function as the argument
	persWM.onAp([]() {
		//this code runs every time AP mode is started
		DEBUG_PRINT("AP MODE");
		DEBUG_PRINT(persWM.getApSsid());
	});

```