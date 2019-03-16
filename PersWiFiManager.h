#ifndef PERSWIFIMANAGER_H
#define PERSWIFIMANAGER_H

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#else
#error "Unknown board class"
#endif
#include <DNSServer.h>

#define WIFI_CONNECT_TIMEOUT 30

class PersWiFiManager {

  public:

    typedef std::function<void(void)> WiFiChangeHandlerFunction;

#if defined(ESP8266)
    PersWiFiManager(ESP8266WebServer& s, DNSServer& d);
#elif defined(ESP32)
    PersWiFiManager(WebServer& s, DNSServer& d);
#endif

    bool attemptConnection(const String& ssid = "", const String& pass = "");

    void setupWiFiHandlers();

    bool begin(const String& ssid = "", const String& pass = "");

    void resetSettings();

    String getApSsid();

    String getSsid();

    void setApCredentials(const String& apSsid, const String& apPass = "");

    void setConnectNonBlock(bool b);

    void handleWiFi();

    void startApMode();

    void onConnect(WiFiChangeHandlerFunction fn);

    void onAp(WiFiChangeHandlerFunction fn);

  private:
#if defined(ESP8266)
    ESP8266WebServer * _server;
#elif defined(ESP32)
    WebServer * _server;
#endif
    DNSServer * _dnsServer;
    String _apSsid, _apPass;

    bool _connectNonBlock;
    unsigned long _connectStartTime;
    bool _freshConnectionAttempt;

    WiFiChangeHandlerFunction _connectHandler;
    WiFiChangeHandlerFunction _apHandler;

};//class

#endif

