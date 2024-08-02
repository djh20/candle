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
    static void onVersionRequest(AsyncWebServerRequest *request);
    static void onMetricsRequest(AsyncWebServerRequest *request);
    static void onConsoleRequest(AsyncWebServerRequest *request);
    static void onTaskRequest(AsyncWebServerRequest *request);

    AsyncWebServer server = AsyncWebServer(80);
    static JsonDocument doc;
};

extern WiFiWebServer GlobalWiFiWebServer;