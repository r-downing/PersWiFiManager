/* PersWiFiManager
   version 3.0.1
   https://r-downing.github.io/PersWiFiManager/
*/

#include "PersWiFiManager.h"

#if defined(ESP32)
#include <esp_wifi.h>
#endif

#ifdef WIFI_HTM_PROGMEM
const char wifi_htm[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no"/>
    <title>ESP WiFi</title>
    <script>
      function g(i){
        return document.getElementById(i);
      };
      function p(t,l){
        if(confirm(t)) window.location=l;
      };
      function E(s){
        return document.createElement(s)
      };
      var S="setAttribute",A="appendChild",H="innerHTML",X,wl;
      function scan(){
        if(X) return;
        X=new XMLHttpRequest(),wl=document.getElementById('wl');
        wl[H]="Scanning...";
        X.onreadystatechange=function(){
          if (this.readyState==4&&this.status==200){
            X=0;wl[H]="";
            this.responseText.split("\n").forEach(function (e){
              let t=e.split(","), s=t.slice(2).join(',');
              var d=E('div'),i=E('a'),c=E('a');
              i[S]('class','s');
              c[S]('class','q');
              i.onclick=function(){
                g('s').value=s;
                g('p').focus();
              };
              i[A](document.createTextNode(s));
              c[H]=t[0]+"%"+(parseInt(t[1])?"\uD83D\uDD12":"\u26A0");
              wl[A](i); 
              wl[A](c);
              wl[A](document.createElement('br'));
            });
          }
        };
        X.open("GET","wifi/list",true);
        X.send();
      };
    </script>
    <style>
      input{
        padding:5px;font-size:1em;width:95%;
      }
      body{
        text-align:center;font-family:verdana;background-color:black;color:white;
      }
      a{
        color:#1fa3ec;
      }
      button{
        border:0;border-radius:0.3em;background-color:#1fa3ec;color:#fff;
        line-height:2.4em;font-size:1.2em;width:100%;display:block;
      }
      .q{
        float:right;
      }
      .s{
        display:inline-block;width:14em;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;
      }
      #wl{
        line-height:1.5em;
      }
    </style>
  </head>
  <body>
    <div style='text-align:left;display:inline-block;width:320px;padding:5px'>
      <button onclick="scan()">&#x21bb; Scan</button>
      <p id='wl'></p>
      <form method='post' action='/wifi/connect'>
        <input id='s' name='n' length=32 placeholder='SSID'>
        <br>
        <input id='p' name='p' length=64 type='password' placeholder='password'>
        <br><br>
        <button type='submit'>Connect</button>
      </form>
      <br><br>
      <button onclick="p('Reboot device?','/wifi/rst')">Reboot</button>
      <br>
      <a href="javascript:history.back()">Back</a> |<a href="/">Home</a>
    </div>
  </body>
</html>
)=====";
#endif

#if defined(ESP8266)
PersWiFiManager::PersWiFiManager(ESP8266WebServer& s, DNSServer& d) {
#elif defined(ESP32)
PersWiFiManager::PersWiFiManager(WebServer& s, DNSServer& d) {
#endif
  _server = &s;
  _dnsServer = &d;
  _apPass = "";
  _freshConnectionAttempt = false;
} //PersWiFiManager

bool PersWiFiManager::attemptConnection(const String& ssid, const String& pass) {
  //attempt to connect to wifi
  WiFi.mode(WIFI_STA);
  if (ssid.length()) {
    resetSettings(); // To avoid issues (experience from WiFiManager)
    if (pass.length()) WiFi.begin(ssid.c_str(), pass.c_str());
    else WiFi.begin(ssid.c_str());
  } else {
    if((getSsid() == "") && (WiFi.status() != WL_CONNECTED)) { // No saved credentials, so skip trying to connect
      _connectStartTime = millis();
      _freshConnectionAttempt = true;
      return false;
    } else {
      WiFi.begin();
    }
  }

  //if in nonblock mode, skip this loop
  _connectStartTime = millis();// + 1;
  while (!_connectNonBlock && _connectStartTime) {
    handleWiFi();
    delay(10);
  }

  return (WiFi.status() == WL_CONNECTED);

} //attemptConnection

void PersWiFiManager::handleWiFi() {
  if (!_connectStartTime) return;

  if (WiFi.status() == WL_CONNECTED) {
    _connectStartTime = 0;
    if (_connectHandler) _connectHandler();
    return;
  }

  //if failed or no saved SSID or no WiFi credentials were found or not connected and time is up
  if ((WiFi.status() == WL_CONNECT_FAILED) || _freshConnectionAttempt || ((WiFi.status() != WL_CONNECTED) && ((millis() - _connectStartTime) > (1000 * WIFI_CONNECT_TIMEOUT)))) {
    startApMode();
    _connectStartTime = 0; //reset connect start time
    _freshConnectionAttempt = false;
  }

} //handleWiFi

void PersWiFiManager::startApMode(){
  //start AP mode
  IPAddress apIP(192, 168, 4, 1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  _apPass.length() ? WiFi.softAP(getApSsid().c_str(), _apPass.c_str()) : WiFi.softAP(getApSsid().c_str());
  if (_apHandler) _apHandler();  
}//startApMode

void PersWiFiManager::setConnectNonBlock(bool b) {
  _connectNonBlock = b;
} //setConnectNonBlock

void PersWiFiManager::setupWiFiHandlers() {
  IPAddress apIP(192, 168, 4, 1);
  _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  _dnsServer->start((byte)53, "*", apIP); //used for captive portal in AP mode

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
#if defined(ESP8266)
        s += String(i ? "\n" : "") + ((constrain(WiFi.RSSI(ix[i]), -100, -50) + 100) * 2) + ","
             + ((WiFi.encryptionType(ix[i]) == ENC_TYPE_NONE) ? 0 : 1) + "," + WiFi.SSID(ix[i]);
#elif defined(ESP32)
        s += String(i ? "\n" : "") + ((constrain(WiFi.RSSI(ix[i]), -100, -50) + 100) * 2) + ","
             + ((WiFi.encryptionType(ix[i]) == WIFI_AUTH_OPEN) ? 0 : 1) + "," + WiFi.SSID(ix[i]);
#endif
      }
    }

    //send string to client
    _server->send(200, "text/plain", s);
  }); //_server->on /wifi/list

  _server->on("/wifi/connect", [&]() {
    _server->send(200, "text/html", "connecting...");
    attemptConnection(_server->arg("n"), _server->arg("p"));
  }); //_server->on /wifi/connect

  _server->on("/wifi/ap", [&](){
    _server->send(200, "text/html", "access point: "+getApSsid());
    startApMode();
  }); //_server->on /wifi/ap

  _server->on("/wifi/rst", [&]() {
    _server->send(200, "text/html", "Rebooting...");
    delay(100);
    //ESP.restart();
	// Adding Safer Restart method
#if defined(ESP8266)
	ESP.wdtDisable();
	ESP.reset();
#elif defined(ESP32)
    ESP.restart();
#endif
	delay(2000);
  });

#ifdef WIFI_HTM_PROGMEM
  _server->on("/wifi.htm", [&]() {
    _server->sendHeader("Cache-Control", " no-cache, no-store, must-revalidate");
    _server->sendHeader("Expires", " 0");
    _server->send(200, "text/html", wifi_htm);
  });
#endif

}//setupWiFiHandlers

bool PersWiFiManager::begin(const String& ssid, const String& pass) {
#if defined(ESP32)
    WiFi.mode(WIFI_STA);  // ESP32 needs this before setupWiFiHandlers(). Might be good for ESP8266 too?
#endif
  setupWiFiHandlers();
  return attemptConnection(ssid, pass); //switched order of these two for return
} //begin

void PersWiFiManager::resetSettings() {
#if defined(ESP8266)
  WiFi.disconnect();
#elif defined(ESP32)
  wifi_mode_t m = WiFi.getMode();
  if(!(m & WIFI_MODE_STA)) WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, true);
  if(!(m & WIFI_MODE_STA)) WiFi.mode(m);
#endif
} // resetSettings

String PersWiFiManager::getApSsid() {
#if defined(ESP8266)
  return _apSsid.length() ? _apSsid : "ESP8266";
#elif defined(ESP32)
  return _apSsid.length() ? _apSsid : "ESP32";
#endif
} //getApSsid

String PersWiFiManager::getSsid() {
#if defined(ESP8266)
  return WiFi.SSID();
#elif defined(ESP32)
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);  // load wifi settings to struct comf
  const char *SSID = reinterpret_cast<const char*>(conf.sta.ssid);
  return String(SSID);
#endif
} //getSsid

void PersWiFiManager::setApCredentials(const String& apSsid, const String& apPass) {
  if (apSsid.length()) _apSsid = apSsid;
  if (apPass.length() >= 8) _apPass = apPass;
} //setApCredentials

void PersWiFiManager::onConnect(WiFiChangeHandlerFunction fn) {
  _connectHandler = fn;
}

void PersWiFiManager::onAp(WiFiChangeHandlerFunction fn) {
  _apHandler = fn;
}


