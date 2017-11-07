# PersWiFiManager
Persistent WiFiManager Arduino library for ESP8266-based microcontrollers

- [PersWiFiManager](#perswifimanager)
- [About](#about)
	- [How it Works](#how-it-works)
	- [Screenshots](#screenshots)
		- [Main WiFi Setup Page](#main-wifi-setup-page)
		- [WiFi Scan Function](#wifi-scan-function)
		- [WPS Setup](#wps-setup)
	- [How to Use](#how-to-use)
- [Installation](#installation)
	- [Via Arduino IDE Library Manager](#via-arduino-ide-library-manager)
	- [Via ZIP File](#via-zip-file)
- [Examples](#examples)
	- [Basic REST API](#basic-rest-api)
- [To Do](#to-do)

# About

This Persistent WiFi Manager provides a WiFi Settings web interface for ESP8266-based microcontrollers. It can autoconnect to the last used network, and then if unsuccessful, switch to AP mode with a captive DNS server. This action is non-blocking; the program will proceed to the main loop after, regardless of WiFi connection status. In AP mode, any device (phone, computer, etc) can connect to this network to configure the wifi connection, or access the device as a webserver. 

This library and UI was inspired by tzapu's [WiFiManager library](https://github.com/tzapu/WiFiManager). The main difference is that it allows the program to continue functioning normally, even in AP mode. It is also more memory efficient, as it does not have to dynamically build the page, and can serve it from SPIFFS rather than PROGMEM. 

## How it Works

When creating an instance of a PersWiFiManager object, it takes arguments of an already-declared ESP8266WiFiManager and DNSServer, and attaches the wifi settings handlers to them. This way, they can continue to be used in the main program.

The WiFi setup page is static and served from SPIFFS (or PROGMEM). It has a form for manual SSID/password input. If the user wants to scan for WiFi networks, it makes a request to the server, which sends a CSV-style list of available WiFi networks (sorted by signal, with duplicates removed). The page uses javascript to parse the list and display it. The interface also has aWPS setup button, and a Reboot button.

## Screenshots
### Main WiFi Setup Page
<img src="https://i.imgur.com/aLT68mU.png" width="300">

### WiFi Scan Function
<img src="https://i.imgur.com/IzY25Gr.png" width="300">

### WPS Setup
<img src="https://i.imgur.com/1mwIEXD.png" width="300">

## How to Use

The library can be installed and included like a normal Arduino library. The *wifi.htm* can be uploaded to SPIFFS using the [Arduino ESP8266 filesystem uploader](https://github.com/esp8266/arduino-esp8266fs-plugin). Alternatively, the library can be modified to serve the page from PROGMEM instead, by adding the following line to *PersWiFiManager.cpp*

`#define WIFI_HTM_PROGMEM`


The PersWiFiManager object should be declared in the same scope as the servers, which are passed as arguements in the constructor.

```cpp
ESP8266WebServer server(80);
DNSServer dnsServer;
PersWiFiManager persWM(server, dnsServer);
```

Then in setup, the `begin()` function should be called.
```cpp
void setup() {

  SPIFFS.begin();
  persWM.begin();
  ...
```

Since the captive DNS server redirects to the root page, the user should make sure their index page or `"/"` handler provides a link to `"/wifi.htm"`

Another option is to have a custom user interface that can make use of the same `"/wifi/..."` server handlers.

# Installation
## Via Arduino IDE Library Manager
Coming soon...

## Via ZIP File
[Download zip file](https://github.com/r-downing/PersWiFiManager/archive/master.zip) and extract to *Arduino/libraries* folder

# Examples
## Basic REST API

[Full example code](https://github.com/r-downing/PersWiFiManager/tree/master/examples/basic_rest_api)

This is a simple, fully-funcioning REST API example that uses the PersWiFiManager to setup and handle the wifi connection.

All that is needed is to create a PersWiFiObject and pass webserver + dnsServer as arguments

```cpp

//includes
#include <PersWiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266SSDP.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>

....

//server objects
ESP8266WebServer server(80);
DNSServer dnsServer;
PersWiFiManager persWM(server, dnsServer);

```

Then in setup, call `PersWiFiManager::begin()`

```cpp
void setup() {
  ...

  //allows serving of files from SPIFFS
  SPIFFS.begin();
  persWM.begin();
```

The program tries to connect to wifi, then the REST API just works, with or without local wifi. You can switch wifi networks at any time.

### Screenshot
![](https://i.imgur.com/RScftNg.png)

# To Do
- [ ] get in library manager
- [ ] AP mode encryption option
- [ ] static IP option
- [ ] Finish doc page
- [ ] Examples




