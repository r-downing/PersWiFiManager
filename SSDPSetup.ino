void SSDPSetup(ESP8266WebServer &webServer, String deviceName) {
  webServer.on("/description.xml", HTTP_GET, [&]() {
    SSDP.schema(webServer.client());
  });
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(deviceName);
  SSDP.setURL("/");
  SSDP.begin();
  SSDP.setDeviceType("upnp:rootdevice");
}

