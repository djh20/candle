#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class WiFiWebServer
{
  public:
    void begin();
    void end();

  private:
    AsyncWebServer server = AsyncWebServer(80);
};

extern WiFiWebServer GlobalWiFiWebServer;