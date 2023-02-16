#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include "vehicle/vehicle_nissan_leaf.h"
#include "utils/logger.h"
#include "config.h"

#define JSON_DOC_SIZE 1024U
#define WIFI_SCAN_INTERVAL 10000U
#define SEND_INTERVAL 50U

IPAddress localIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Vehicle* vehicle;

DynamicJsonDocument doc(JSON_DOC_SIZE);
char jsonBuffer[JSON_DOC_SIZE];

uint32_t lastScanMillis = 0;
uint32_t lastSendMillis = 0;
uint32_t sendCounter = 0;

WiFiEventHandler wifiHandle;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
{
  if (type == WS_EVT_CONNECT) 
  {
    Logger.log(
      Debug, 
      "ws", 
      "WebSocket client #%u connected from %s", 
      client->id(), 
      client->remoteIP().toString().c_str()
    );

    memset(jsonBuffer, 0, JSON_DOC_SIZE);
    doc.clear();
    vehicle->metricsToJson(doc);
    serializeJson(doc, jsonBuffer);

    client->text(jsonBuffer);
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Logger.log(Debug, "ws", "WebSocket client #%u disconnected", client->id());
  }
}

void onStationModeConnected(const WiFiEventStationModeConnected& event)
{
  Logger.log(Debug, "wifi", "Connected to %s", event.ssid);
}

void setup() 
{
  Serial.begin(115200);
  delay(1000);

  // Print splash art & info
  Serial.println();
  Serial.println(
    "____ ____ _  _ ___  _    ____\n"
    "|    |__| |\\ | |  \\ |    |___\n"
    "|___ |  | | \\| |__/ |___ |___"
  );
  Serial.println("Version: DEV");
  Serial.println("Disclaimer: This is a work in progress.");

  Serial.println();

  vehicle = new VehicleNissanLeaf();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(false);
  wifiHandle = WiFi.onStationModeConnected(onStationModeConnected);

  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, 1);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/api/vehicle/metrics/set", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (!request->hasParam("id") || !request->hasParam("value"))
    {
      request->send(400);
      return;
    }

    AsyncWebParameter *idParam = request->getParam("id");
    AsyncWebParameter *valueParam = request->getParam("value");
    
    for (int i = 0; i < vehicle->totalMetrics; i++)
    {
      Metric* metric = vehicle->metrics[i];
      if (strcmp(metric->id, idParam->value().c_str()) == 0)
      {
        metric->setValueFromString(valueParam->value());
        break;
      }
    }

    request->send(200);
  });

  server.on("/api/vehicle/metrics", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    // Reply with all metrics.
    memset(jsonBuffer, 0, JSON_DOC_SIZE);
    doc.clear();
    vehicle->metricsToJson(doc);
    serializeJson(doc, jsonBuffer);

    request->send(200, "application/json", jsonBuffer);
  });

  server.on("/api/test", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    VehicleNissanLeaf* leaf = (VehicleNissanLeaf*) vehicle;

    leaf->socGids->setValue(80);
    leaf->socPercent->setValue(50);
    leaf->soh->setValue(60);
    leaf->pluggedIn->setValue(1);

    if (leaf->powerOutput->value > -4) 
    {
      leaf->powerOutput->setValue(leaf->powerOutput->value - 0.5);
    }
    else
    {
      leaf->powerOutput->setValue(0);
    }

    /*
    leaf->powered->setValue(1);
    leaf->gear->setValue(4);

    float speed = leaf->speed->value + 1;

    leaf->speed->setValue(speed);
    leaf->leftSpeed->setValue(speed);
    leaf->rightSpeed->setValue(speed);
    */

    request->send(200, "text/plain", "Test");
    //WiFi.disconnect();
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
}

void loop() 
{
  vehicle->readAndProcessBusData();
  vehicle->updateExtraMetrics();
  
  uint32_t now = millis();

  if (now - lastScanMillis >= WIFI_SCAN_INTERVAL && !WiFi.isConnected() && vehicle->idle)
  {
    Logger.log(Debug, "wifi", "Scanning for home network...");
    WiFi.scanNetworks(true, true, 0U, (uint8_t*) WIFI_HOME_SSID);
    lastScanMillis = now;
  }

  int8_t totalNetworks = WiFi.scanComplete();
  if (totalNetworks >= 0)
  {
    Logger.log(Debug, "wifi", "Found %d network(s)", totalNetworks);
    Logger.log(Debug, "wifi", "Connecting to %s...", WIFI_HOME_SSID);
    WiFi.begin(WIFI_HOME_SSID, WIFI_HOME_PASSWORD);
    WiFi.scanDelete();
  }

  if (now - lastSendMillis >= SEND_INTERVAL)
  {
    doc.clear();

    if (++sendCounter >= 50)
    {
      vehicle->metricsToJson(doc);
      sendCounter = 0;
    } 
    else 
    {
      vehicle->getUpdatedMetrics(doc, now - SEND_INTERVAL);
    }
   
    if (!doc.isNull())
    {
      memset(jsonBuffer, 0, JSON_DOC_SIZE);
      serializeJson(doc, jsonBuffer);
      ws.textAll(jsonBuffer);
    }

    lastSendMillis = now;
  }

  ws.cleanupClients();
}