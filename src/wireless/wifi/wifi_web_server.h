#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncHTTPUpdateServer.h>
#include <ArduinoJson.h>

class WiFiWebServer
{
  public:
    void begin();

  private:
    static void onVersionRequest(AsyncWebServerRequest *request);
    static void onMetricsRequest(AsyncWebServerRequest *request);
    static void onConsoleRequest(AsyncWebServerRequest *request);
    static void onTaskRequest(AsyncWebServerRequest *request);
    static void onCanRequest(AsyncWebServerRequest *request);

    AsyncWebServer server = AsyncWebServer(80);
    ESPAsyncHTTPUpdateServer updateServer;
    static JsonDocument doc;
};

extern WiFiWebServer GlobalWiFiWebServer;