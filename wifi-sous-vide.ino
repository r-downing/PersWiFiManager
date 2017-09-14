#include <ESP8266WiFi.h>
#include <ESP8266SSDP.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include "AutoPID.h"
//#include "AutoPID.cpp"
#define WIFI_CONNECT_TIMEOUT 30

#define RELAY_PIN D7
#define PULSEWIDTH 5000

//temperature sensor libraries and variables
#include <OneWire.h>
#include <DallasTemperature.h>
#define TEMP_SENSOR_PIN D4
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature temperatureSensors(&oneWire);
#define TEMP_READ_DELAY 800
double temperature, setTemp;// = 5, actual_temperature = 0, previous_temperature = 0, setTemp, setP;
//double P, I, D, PID, error, differential = 0, integral = 0;
//double k_p = .155, k_i = .0003;

ESP8266WebServer server(80);

bool relayControl;
AutoPIDRelay myPID(&temperature, &setTemp, &relayControl, 5000, .12, .0003, 0);

unsigned long lastTempUpdate;
void updateTemperature() {
  if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
    temperature = temperatureSensors.getTempFByIndex(0);
    lastTempUpdate = millis();
    temperatureSensors.requestTemperatures();
  }
}

int scannedNetworks, scanssid;
DNSServer dnsServer;
IPAddress apIP(192, 168, 1, 1);

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.htm";
  String contentType;
  if (server.hasArg("download")) contentType = "application/octet-stream";
  else if (filename.endsWith(".htm")) contentType = "text/html";
  else if (filename.endsWith(".html")) contentType = "text/html";
  else if (filename.endsWith(".css")) contentType = "text/css";
  else if (filename.endsWith(".js")) contentType = "application/javascript";
  else if (filename.endsWith(".png")) contentType = "image/png";
  else if (filename.endsWith(".gif")) contentType = "image/gif";
  else if (filename.endsWith(".jpg")) contentType = "image/jpeg";
  else if (filename.endsWith(".ico")) contentType = "image/x-icon";
  else if (filename.endsWith(".xml")) contentType = "text/xml";
  else if (filename.endsWith(".pdf")) contentType = "application/x-pdf";
  else if (filename.endsWith(".zip")) contentType = "application/x-zip";
  else if (filename.endsWith(".gz")) contentType = "application/x-gzip";
  else contentType = "text/plain";
  String pathGz = path + ".gz";
  if (SPIFFS.exists(pathGz) || SPIFFS.exists(path)) {
    File file = SPIFFS.open(SPIFFS.exists(pathGz)?pathGz:path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }//if SPIFFS.exists
  return false;
}//bool handleFileRead

void setup() {
  Serial.begin(115200); //for terminal debugging

  //set up temperature sensors and relay output
  temperatureSensors.begin();
  temperatureSensors.requestTemperatures();
  myPID.setBangBang(4);
  myPID.setTimeStep(4000);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  //attempt to connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  unsigned long connectTime = millis();
  while ((millis() - connectTime) < 1000 * WIFI_CONNECT_TIMEOUT && WiFi.status() != WL_CONNECTED) delay(10);
  //if timed out, switch to AP mode
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP8266 WiFi Setup");
  }
  dnsServer.start((byte)53, "*", apIP);

  SPIFFS.begin();
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      if (WiFi.status() != WL_CONNECTED) {
        handleFileRead("/");
      } else server.send(200, "text/plain", "FileNotFound");
  }); //server.onNotFound

  scannedNetworks = WiFi.scanNetworks();
  server.on("/wifi", []() {
    if (server.hasArg("list")) {
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200, "application/json", "");
      String cssid = WiFi.SSID();
      cssid.replace("\"", "\\\"");
      server.sendContent("{" + ((WiFi.status() == WL_CONNECTED) ? ("\"connected\":\"" + cssid + "\",") : "") + "\"networks\":[\n");
      for (int i = 0; i < scannedNetworks; i++) {
        String ssid = WiFi.SSID(i);
        ssid.replace("\"", "\\\"");
        server.sendContent("{\"ssid\":\"" + ssid + "\","
                           + "\"signal\":" + (WiFi.RSSI(i) + 100) + ","
                           + "\"encrypted\":" + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "false" : "true")
                           + "}" + ((i < scannedNetworks - 1) ? ",\n" : "]}")
                          );
      }//for
      server.client().stop();
    } else if (server.hasArg("wps")) {
      server.send(200, "text/html", "WPS");
      WiFi.mode(WIFI_STA);
      WiFi.beginWPSConfig();
      delay(100);
      ESP.restart();
    } else if (server.hasArg("rescan")) {
      scannedNetworks = WiFi.scanNetworks();
      server.send(200, "text/html", "rescan");
    } else if (server.hasArg("ssid") || server.hasArg("ssidn")) {
      WiFi.mode(WIFI_STA);
      String ssid = server.hasArg("ssid") ? server.arg("ssid") : WiFi.SSID(server.arg("ssidn").toInt());
      server.hasArg("password") ? WiFi.begin(ssid.c_str(), server.arg("password").c_str()) : WiFi.begin(ssid.c_str());
      delay(100);
      ESP.restart();
    }//endif
  });//server.on wifi

  server.on("/io", []() {
    if (server.hasArg("setTemp")) {
      setTemp = server.arg("setTemp").toFloat();
      //myPID.reset();
    }
    server.send(200, "application/json", String("") + "{\"temperature\":" + temperature + ",\"setTemp\":" + setTemp + ",\"power\":" + myPID.getPulseValue() + "}");
    //String("{'temperature':") + temperature + "}");
    Serial.println(server.uri());
    for (int i = 0; i < server.args(); i++)
      Serial.println(server.argName(i) + ":::" + server.arg(i));
  }); //server.on io
  
  SSDPSetup(server, "NodeMCU (" + WiFi.localIP().toString() + ")");  //SSDP allows device to show up on windows network
  server.begin();
  Serial.println("setup complete.");
}//void setup

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  updateTemperature();
  if (setTemp) {
    myPID.run();
    digitalWrite(RELAY_PIN, !relayControl);
    Serial.println(!relayControl);
  } else {
    myPID.stop();
    digitalWrite(RELAY_PIN, HIGH);
  }
}//void loop

