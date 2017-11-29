---
title: PersWiFiManager::onConnect
description: Attach a handler function that runs every time WiFi connects
api-class: PersWiFiManager

---

## Signatures

- `void onConnect(WiFiChangeHandlerFunction fn)`

## Args

- `WiFiChangeHandlerFunction fn` A generic function pointer to a wifi connection handler function


## Notes

This will run the handler function every time the WiFi settings are changed and then a successful connection is made.

## Example Code

```cpp
void setup(){
	...

	//uses a lambda function as the argument
	persWM.onConnect([]() {
		//this code runs every time wifi is connected
		DEBUG_PRINT("wifi connected");
		DEBUG_PRINT(WiFi.localIP());
		EasySSDP::begin(server);
	});

```