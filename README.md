# PersWiFiManager
Persistent WiFiManager Arduino library for ESP8266-based microcontrollers

- [PersWiFiManager](#perswifimanager)
- [About](#about)
	- [How it Works](#how-it-works)
		- [Example '/wifi/list' output](#example-wifilist-output)
	- [Screenshots](#screenshots)
		- [Main WiFi Setup Page](#main-wifi-setup-page)
		- [WiFi Scan Function](#wifi-scan-function)
		- [WPS Setup](#wps-setup)
	- [How to Use](#how-to-use)
- [Installation](#installation)
	- [Via Arduino IDE Library Manager](#via-arduino-ide-library-manager)
	- [Via ZIP File](#via-zip-file)
- [Documentation](#documentation)
	- [PersWiFiManager Functions](#perswifimanager-functions)
		- [PersWiFiManager::PersWiFiManager](#perswifimanagerperswifimanager)
		- [PersWiFiManager::attemptConnection](#perswifimanagerattemptconnection)
		- [PersWiFiManager::setupWiFiHandlers](#perswifimanagersetupwifihandlers)
		- [PersWiFiManager::begin](#perswifimanagerbegin)
		- [PersWiFiManager::getApSsid](#perswifimanagergetapssid)
		- [PersWiFiManager::setApCredentials](#perswifimanagersetapcredentials)
- [Examples](#examples)
	- [Basic REST API](#basic-rest-api)
		- [Screenshot](#screenshot)
- [To Do](#to-do)

# About

This Persistent WiFi Manager provides a WiFi Settings web interface for ESP8266-based microcontrollers. It can autoconnect to the last used network, and then if unsuccessful, switch to AP mode with a captive DNS server. This action is non-blocking; the program will proceed to the main loop after, regardless of WiFi connection status. In AP mode, any device (phone, computer, etc) can connect to this network to configure the wifi connection, or access the device as a webserver. 

This library and UI was inspired by tzapu's [WiFiManager library](https://github.com/tzapu/WiFiManager). The main difference is that it allows the program to continue functioning normally, even in AP mode. It is also more memory efficient, as it does not have to dynamically build the page, and can serve it from SPIFFS rather than PROGMEM. 

## How it Works

When creating an instance of a PersWiFiManager object, it takes arguments of an already-declared ESP8266WiFiManager and DNSServer, and attaches the wifi settings handlers to them. This way, they can continue to be used in the main program.

The WiFi setup page is static and served from SPIFFS (or PROGMEM). It has a form for manual SSID/password input. If the user wants to scan for WiFi networks, it makes a request to the server, which responds with a CSV-style list of available WiFi networks (sorted by signal, with duplicates removed). I chose CSV over JSON because, even with the ArduinoJSON library or proper manual escaping, I still encountered a few wireless networks with bad characters that would invalidate the entire json string. The page uses javascript to parse the list and display it. The interface also has a WPS setup button, and a Reboot button.

### Example '/wifi/list' output

Each line represents a network in the format: `signal,encrypted,SSID`

```
100,1,DAY LAN
94,1,FIGHTER OF THE NIGHT LAN
50,0,linksys
50,0,xfinitywifi
48,1,XFINITY
40,1,DIRECT-8C0A0D0F
```

Commas in the SSID are ok; the javascript in the wifi.htm page splits the lines by commas but re-merges anything after the second column. It also uses the DOM `createTextNode` method to display the network names, so any other special symbols are displayed properly as well.

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

For a quick and easy way to serve files from SPIFFS, see my [SPIFFSReadServer](https://github.com/r-downing/SPIFFSReadServer#spiffsreadserver) library.


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
**Sketch** -> **Include Library** -> **Manage Libraries...** -> search for "PersWiFiManager"
![Arduino Library Manager screen](https://i.imgur.com/IdLr2VV.png)

## Via ZIP File
[Download zip file](https://github.com/r-downing/PersWiFiManager/archive/master.zip) and extract to *Arduino/libraries* folder

# Documentation
## PersWiFiManager Functions

### PersWiFiManager::PersWiFiManager
> Creates a new PersWiFiManager object
```cpp
PersWiFiManager(ESP8266WebServer& s, DNSServer& d)
```
- `s` and `d` are the existing web server and dns server object, passed by reference

### PersWiFiManager::attemptConnection
> Attempts to connect to wifi. This is already called in `begin`, but this function can be used as a delayed alternative with `setupWiFiHandlers`
```cpp
bool attemptConnection(const String& ssid = "", const String& pass = "")
```
- `ssid` and `pass` are optional arguments, the desired network and password to connect to

### PersWiFiManager::setupWiFiHandlers
> Sets up built-in wifi command handlers. This is already called in `begin`, but this can be used as a delayed alternative, calling `setupWiFiHandlers` at a later time
```cpp
void setupWiFiHandlers()
```

### PersWiFiManager::begin
> Begins WiFi Manager operation. Sets up handlers and attempts auto-connection, switches to AP mode if unsuccessful
```cpp
bool begin(const String& ssid = "", const String& pass = "")
```
- `ssid` and `pass` are optional arguments, the desired network and password to connect to first
- `return` true if autoconnected, false if switched to AP mode

### PersWiFiManager::getApSsid
> Get AP mode SSID
```cpp
String getApSsid()
```
- `return` the AP ssid, either automatically set, or overridden in `setApCredentials`

### PersWiFiManager::setApCredentials
> Sets AP mode credentials to be used if wifi can't connect. Should be called before `begin`
```cpp
void setApCredentials(const String& apSsid, const String& apPass = "")
```
- `apSsid` and (optional) `apPass` are the network credentials for AP mode


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

  //optional set credentials for AP mode if wifi connect fails
  persWM.setApCredentials("WIFI SETUP"); 
  //persWM.setApCredentials("WIFI SETUP", "PASSWORD");

  //begin autoconnect
  persWM.begin();

  //or try a specific network first
  //persWM.begin("mynetwork"); //open
  //persWM.begin("mynetwork", "mypassword"); //encrypted

  //program then proceeds from this point, with or without wifi
```

The program tries to connect to wifi, then the REST API just works, with or without local wifi. You can switch wifi networks at any time.

### Screenshot
![](https://i.imgur.com/RScftNg.png)

# To Do
- [ ] static IP option
- [ ] non-blocking connection/reconnection
	- This may require a continuously called function (something like *handleWiFi*) which would be a little bit of a fundamental change, so to preserve compatibility/simplicity, I may want to make this feature optional (default turned off)

----

[Home](http://ryandowning.net)




