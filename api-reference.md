# PersWiFiManager API Reference

## constructor
[PersWiFiManager::PersWiFiManager](#PersWiFiManager)

## functions
[PersWiFiManager::attemptConnection](#attemptConnection)

[PersWiFiManager::begin](#begin)

[PersWiFiManager::getApSsid](#getApSsid)

[PersWiFiManager::handleWiFi](#handleWiFi)

[PersWiFiManager::onAp](#onAp)

[PersWiFiManager::onConnect](#onConnect)

[PersWiFiManager::setApCredentials](#setApCredentials)

[PersWiFiManager::setConnectNonBlock](#setConnectNonBlock)

[PersWiFiManager::setupWiFiHandlers](#setupWiFiHandlers)

[PersWiFiManager::startApMode](#startApMode)

---

## PersWiFiManager
Creates a new PersWiFiManager object

### Signatures
    PersWiFiManager(ESP8266WebServer& s, DNSServer& d)

### Args
    ESP8266WebServer& s An existing instance of the ESP8266WebServer class that will be used throughout the program
    DNSServer& d An existing instance of the DNSServer class that will be used throughout the program

### Notes
Declare the object globally, or at least in the same scope as your ESP8266WebServer and DNSServer


## attemptConnection
Attempts to auto-connect to last WiFi network used (or other if specified). Falls back to AP mode if unsuccessful.

### Signatures
    bool attemptConnection()
    bool attemptConnection(const String& ssid)
    bool attemptConnection(const String& ssid, const String& pass)

### Args
    const String& ssid (Optional) SSID to connect to. This overrides the default behavior of connecting to the last network used.
    const String& pass (Optional) password for the specified SSID. Length must be >= 8 characters

### Returns
    true if successfully autoconnected, false if switched to AP mode

### Notes
This function is already called inside begin, or when WiFi settings are changed, but this can be used for manual operation.


## begin
Begins automatic WiFi Configuration

### Signatures
    bool begin()
    bool begin(const String& ssid)
    bool begin(const String& ssid, const String& pass)

### Args
    const String& ssid (Optional) SSID to connect to. This overrides the default behavior of connecting to the last network used.
    const String& pass (Optional) password for the specified SSID. Length must be >= 8 characters

### Returns
    true if successfully autoconnected

### Notes

This sets up the server handlers for WiFi actions, then attempts to autoconnect. If no SSID is specified, it automatically attempts to connect to the last network used.


## getApSsid
API Reference / PersWiFiManager::getApSsid
Gets the AP mode SSID

### Signatures
    String getApSsid()

### Returns
    String containing the AP mode SSID

### Notes
The AP mode SSID is automatically set by the library, unless overridden by setApCredentials


## handleWiFi
Handles WiFi connecting/disconnecting in non-blocking mode

### Signatures
    void handleWiFi()

### Notes
This function does not need to be called in normal mode.

If the PersWiFiManager is set to non-blocking mode with setConnectNonBlock, this must be called in the main loop.


## onAp
Attach a handler function that runs every time AP mode is started

### Signatures
    void onAp(WiFiChangeHandlerFunction fn)

### Args
    WiFiChangeHandlerFunction fn A generic function pointer to an AP mode handler function

### Notes
This will run the handler function every time AP mode is started, when WiFi fails to connect or AP mode is manually triggered.

For successful connection handler, see onConnect

### Example Code

    void setup(){
	...

	//uses a lambda function as the argument
	persWM.onAp([]() {
		//this code runs every time AP mode is started
		DEBUG_PRINT("AP MODE");
		DEBUG_PRINT(persWM.getApSsid());
	});


## onConnect
Attach a handler function that runs every time WiFi connects

### Signatures
    void onConnect(WiFiChangeHandlerFunction fn)

### Args
    WiFiChangeHandlerFunction fn A generic function pointer to a wifi connection handler function

### Notes
This will run the handler function every time the WiFi settings are changed and then a successful connection is made.

For unsuccessful / AP mode connection handler, see onAp

### Example Code

    void setup(){
	...

	//uses a lambda function as the argument
	persWM.onConnect([]() {
		//this code runs every time wifi is connected
		DEBUG_PRINT("wifi connected");
		DEBUG_PRINT(WiFi.localIP());
		EasySSDP::begin(server);
	});


## setApCredentials
Sets the SSID (and password) to be used for AP mode

### Signatures
    void setApCredentials(const String& apSsid)
    void setApCredentials(const String& apSsid, const String& apPass)

### Args
    const String& apSsid SSID to be used for AP mode.
    const String& apPass (Optional) Password to be used for AP mode.

### Notes
Passwords less than 8 characters long will be ignored.


## setConnectNonBlock
Sets the PersWiFiManager connecting actions to non-blocking mode

### Signatures
    void setConnectNonBlock(bool b)

### Args
    bool b use true for non-blocking mode, false for regular mode

### Notes
In non-blocking mode, handleWiFi must be called in the main loop


## setupWiFiHandlers
Attaches WiFi settings and captive DNS handlers to web server and DNS Server

### Signatures
    void setupWiFiHandlers()

### Notes
This function is already called inside begin() and doesn’t normally need to be used, but is available for manual operation.


## startApMode
Allows manual starting of AP mode

### Signatures
    void startApMode()

### Notes
WiFi settings are controlled by the user via the web-interface, but this allows a manual program-induced switch to AP mode.

