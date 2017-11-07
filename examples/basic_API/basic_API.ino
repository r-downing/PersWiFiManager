//#define DEBUG

#ifdef DEBUG
#define DEBUG_START Serial.begin(115200)
#define DEBUG_PRINT(x) Serial.println(x)
#else
#define DEBUG_PRINT(x) 
#define DEBUG_START
#endif


#include <PersWiFiManager.h>
#include <AutoPID.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266SSDP.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>

#define RELAY_PIN D7
#define PULSEWIDTH 5000

#define DEVICE_NAME "Sous Vide"


//temperature sensor libraries and variables
#include <OneWire.h>
#include <DallasTemperature.h>
#define TEMP_SENSOR_PIN D4
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature temperatureSensors(&oneWire);
#define TEMP_READ_DELAY 800
double temperature, setTemp;
unsigned long timeAtTemp;
bool relayControl, powerOn;
AutoPIDRelay myPID(&temperature, &setTemp, &relayControl, 5000, .12, .0003, 0);

const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"1; url=/\" /></head><body><a href=\"/\">redirecting...</a></body>";

unsigned long lastTempUpdate;
void updateTemperature() {
  if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
    temperature = temperatureSensors.getTempFByIndex(0);
    lastTempUpdate = millis();
    temperatureSensors.requestTemperatures();
  }
} //void updateTemperature

ESP8266WebServer server(80);
DNSServer dnsServer;
PersWiFiManager persWM(server, dnsServer);


//code from fsbrowser example, consolidated.
bool handleFileRead(String path) {
  DEBUG_PRINT("handlefileread" + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType;
  if (path.endsWith(".htm") || path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".gif")) contentType = "image/gif";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";
  else if (path.endsWith(".xml")) contentType = "text/xml";
  else if (path.endsWith(".pdf")) contentType = "application/x-pdf";
  else if (path.endsWith(".zip")) contentType = "application/x-zip";
  else if (path.endsWith(".gz")) contentType = "application/x-gzip";
  else if (path.endsWith(".json")) contentType = "application/json";
  else contentType = "text/plain";

  //split filepath and extension
  String prefix = path, ext = "";
  int lastPeriod = path.lastIndexOf('.');
  if (lastPeriod >= 0) {
    prefix = path.substring(0, lastPeriod);
    ext = path.substring(lastPeriod);
  }

  //look for smaller versions of file
  //minified file, good (myscript.min.js)
  if (SPIFFS.exists(prefix + ".min" + ext)) path = prefix + ".min" + ext;
  //gzipped file, better (myscript.js.gz)
  if (SPIFFS.exists(prefix + ext + ".gz")) path = prefix + ext + ".gz";
  //min and gzipped file, best (myscript.min.js.gz)
  if (SPIFFS.exists(prefix + ".min" + ext + ".gz")) path = prefix + ".min" + ext + ".gz";

  if (SPIFFS.exists(path)) {
    DEBUG_PRINT("sending file " + path);
    File file = SPIFFS.open(path, "r");
    if (server.hasArg("download"))
      server.sendHeader("Content-Disposition", " attachment;");
    if (server.uri().indexOf("nocache") < 0)
      server.sendHeader("Cache-Control", " max-age=172800");

    //optional alt arg (encoded url), server sends redirect to file on the web
    if (WiFi.status() == WL_CONNECTED && server.hasArg("alt")) {
      server.sendHeader("Location", server.arg("alt"), true);
      server.send ( 302, "text/plain", "");
    } else {
      //server sends file
      size_t sent = server.streamFile(file, contentType);
    }
    file.close();
    return true;
  } //if SPIFFS.exists
  return false;
} //bool handleFileRead


void setup() {
  DEBUG_START; //for terminal debugging
  DEBUG_PRINT();

  //allows serving of files from SPIFFS
  SPIFFS.begin();

  //set up temperature sensors and relay output
  temperatureSensors.begin();
  temperatureSensors.requestTemperatures();
  myPID.setBangBang(4);
  myPID.setTimeStep(4000);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  persWM.begin();

  //serve files from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.sendHeader("Cache-Control", " max-age=172800");
      server.send(302, "text/html", metaRefreshStr);
    }
  }); //server.onNotFound

  //handles commands from webpage, sends live data in JSON format
  server.on("/io", []() {
    DEBUG_PRINT("server.on /io");////////////////////////
    if (server.hasArg("setTemp")) {
      powerOn = true;
      setTemp = server.arg("setTemp").toFloat();
    } //if
    if (server.hasArg("powerOff")) {
      powerOn = false;
    } //if

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["temperature"] = temperature;
    json["setTemp"] = setTemp;
    json["power"] = myPID.getPulseValue();
    json["running"] = powerOn;
    json["upTime"] = ((timeAtTemp) ? (millis() - timeAtTemp) : 0);
    JsonArray& errors = json.createNestedArray("errors");
    if (temperature < -190) errors.add("sensor fault");
    char jsonchar[200];
    json.printTo(jsonchar);
    server.send(200, "application/json", jsonchar);

  }); //server.on io

  server.on("/esp8266-project.json", [] () {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["device_name"] = DEVICE_NAME;
    char jsonchar[200];
    json.printTo(jsonchar);
    server.send(200, "application/json", jsonchar);
  });

  //SSDP makes device visible on windows network
  server.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(server.client());
  });
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(DEVICE_NAME);
  SSDP.setURL("/");
  SSDP.begin();
  SSDP.setDeviceType("upnp:rootdevice");

  server.begin();
  DEBUG_PRINT("setup complete.");
} //void setup

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  updateTemperature();
  if (powerOn) {
    myPID.run();
    digitalWrite(RELAY_PIN, !relayControl);
    if (myPID.atSetPoint(2)) {
      if (!timeAtTemp)
        timeAtTemp = millis();
    } else {
      timeAtTemp = 0;
    }
  } else { //!powerOn
    timeAtTemp = 0;
    myPID.stop();
    digitalWrite(RELAY_PIN, HIGH);
  } //endif
  //Serial.println(millis());
} //void loop
