#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WiFiWebServer
{
  public:
    void begin();
    void end();

  private:
    AsyncWebServer server = AsyncWebServer(80);
    JsonDocument doc;
};

extern WiFiWebServer GlobalWiFiWebServer;