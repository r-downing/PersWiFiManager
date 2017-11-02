#include "PersWiFiManager.h"

#ifdef WIFI_HTM_PROGMEM
const char wifi_htm[] PROGMEM = R"=====(<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no"/><title>ESP WiFi</title><script>function g(i){return document.getElementById(i);};function p(t,l){if(confirm(t)) window.location=l;};function E(s){return document.createElement(s)};var S="setAttribute",A="appendChild",H="innerHTML",X,wl;function scan(){if(X) return;X=new XMLHttpRequest(),wl=document.getElementById('wl');wl[H]="Scanning...";X.onreadystatechange=function(){if (this.readyState==4&&this.status==200){X=0;wl[H]="";this.responseText.split("\n").forEach(function (e){let t=e.split(","), s=t.slice(2).join(',');var d=E('div'),i=E('a'),c=E('a');i[S]('class','s'); c[S]('class','q');i.onclick=function(){g('s').value=s;g('p').focus();};i[A](document.createTextNode(s));c[H]=t[0]+"%"+(parseInt(t[1])?"\uD83D\uDD12":"\u26A0");wl[A](i); wl[A](c);wl[A](document.createElement('br'));});}};X.open("GET","wifi/list",true);X.send();};</script><style>input{padding:5px;font-size:1em;width:95%;}body{text-align:center;font-family:verdana;background-color:black;color:white;}a{color:#1fa3ec;}button{border:0;border-radius:0.3em;background-color:#1fa3ec;color:#fff;line-height:2.4em;font-size:1.2em;width:100%;display:block;}.q{float:right;}.s{display:inline-block;width:14em;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;}#wl{line-height:1.5em;}</style></head><body><div style='text-align:left;display:inline-block;width:320px;padding:5px'><button onclick="scan()">&#x21bb; Scan</button><p id='wl'></p><form method='post' action='/wifi/connect'><input id='s' name='n' length=32 placeholder='SSID'><br><input id='p' name='p' length=64 type='password' placeholder='password'><br><br><button type='submit'>Connect</button></form><br><br><button onclick="p('Start WPS?','/wifi/wps')">WPS Setup</button><br><button onclick="p('Reboot device?','/wifi/rst')">Reboot</button><br><a href="javascript:history.back()">Back</a> |<a href="/">Home</a></div></body></html>)=====";
#endif

PersWiFiManager::PersWiFiManager(ESP8266WebServer& s, DNSServer& d, const String& apSSID) {
  _server = &s;
  _dnsServer = &d;
  _apSSID = apSSID;
}

void PersWiFiManager::attemptConnection(const String& ssid, const String& pass) {
  IPAddress apIP(192, 168, 1, 1);
  //attempt to connect to wifi
  WiFi.mode(WIFI_STA);
  if (ssid.length()) {
    if (pass.length()) WiFi.begin(ssid.c_str(), pass.c_str());
    else WiFi.begin(ssid.c_str());
  } else {
    WiFi.begin();
  }
  unsigned long connectTime = millis();
  //while ((millis() - connectTime) < 1000 * WIFI_CONNECT_TIMEOUT && WiFi.status() != WL_CONNECTED)
  while (WiFi.status() != WL_CONNECT_FAILED && WiFi.status() != WL_CONNECTED && (millis() - connectTime) < 1000 * WIFI_CONNECT_TIMEOUT)
    delay(10);
  if (WiFi.status() != WL_CONNECTED) { //if timed out, switch to AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(_apSSID.length() ? _apSSID.c_str() : "ESP8266");
  } //if
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  _dnsServer->start((byte)53, "*", apIP); //used for captive portal in AP mode
}

void PersWiFiManager::attemptConnection() {
  PersWiFiManager::attemptConnection("", "");
}

void PersWiFiManager::setupWiFiHandlers() {
  _server->on("/wifi/list", [&] () {
    //scan for wifi networks
    int n = WiFi.scanNetworks();

    //build array of indices
    int ix[n];
    for (int i = 0; i < n; i++) ix[i] = i;

    //sort by signal strength
    for (int i = 0; i < n; i++) for (int j = 1; j < n - i; j++) if (WiFi.RSSI(ix[j]) > WiFi.RSSI(ix[j - 1])) std::swap(ix[j], ix[j - 1]);
    //remove duplicates
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (WiFi.SSID(ix[i]).equals(WiFi.SSID(ix[j])) && WiFi.encryptionType(ix[i]) == WiFi.encryptionType(ix[j])) ix[j] = -1;

    //build plain text string of wifi info
    //format [signal%]:[encrypted 0 or 1]:SSID
    String s = "";
    s.reserve(2050);
    for (int i = 0; i < n && s.length() < 2000; i++) { //check s.length to limit memory usage
      if (ix[i] != -1) {
        s += String(i ? "\n" : "") + ((constrain(WiFi.RSSI(ix[i]), -100, -50) + 100) * 2) + ","
             + ((WiFi.encryptionType(ix[i]) == ENC_TYPE_NONE) ? 0 : 1) + "," + WiFi.SSID(ix[i]);
      }
    }

    //send string to client
    _server->send(200, "text/plain", s);
  }); //_server->on /wifi/list

  _server->on("/wifi/wps", [&]() {
    _server->send(200, "text/html", "attempting WPS");
    WiFi.mode(WIFI_STA);
    WiFi.beginWPSConfig();
    delay(100);
    if (WiFi.status() != WL_CONNECTED) {
      attemptConnection("", "");
    }
  }); //_server->on /wifi/wps

  _server->on("/wifi/connect", [&]() {
    _server->send(200, "text/html", "connecting...");
    attemptConnection(_server->arg("n"), _server->arg("p"));
  }); //_server->on /wifi/connect

  _server->on("/wifi/rst", [&]() {
    _server->send(200, "text/html", "Rebooting...");
    delay(100);
    ESP.restart();
  });

#ifdef WIFI_HTM_PROGMEM
  _server->on("/wifi.htm", [&]() {
    _server->send(200, "text/html", wifi_htm);
  });
#endif

}//setupWiFiHandlers

void PersWiFiManager::begin() {
  attemptConnection();
  setupWiFiHandlers();
}//begin

