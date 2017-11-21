---
title: PersWiFiManager::PersWiFiManager
description: Creates a new PersWiFiManager object
api-class: PersWiFiManager
api-group: constructors
---

## Signatures

- `PersWiFiManager(ESP8266WebServer& s, DNSServer& d)`

## Args

- `ESP8266WebServer& s` An existing instance of the ESP8266WebServer class that will be used throughout the program
- `DNSServer& d` An existing instance of the DNSServer class that will be used throughout the program

## Notes

Declare the object globally, or at least in the same scope as your `ESP8266WebServer` and `DNSServer`