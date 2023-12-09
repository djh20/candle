#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <ArduinoJson.h>
#include "vehicle/vehicle_nissan_leaf.h"
#include "utils/logger.h"
#include "config.h"

#define JSON_DOC_SIZE 1024U
#define WIFI_SCAN_INTERVAL 10000U
#define SEND_INTERVAL 60U

IPAddress localIP(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 
 
AsyncWebServer server(80);
AsyncEventSource events("/events");

Vehicle* vehicle;

DynamicJsonDocument doc(JSON_DOC_SIZE);
char jsonBuffer[JSON_DOC_SIZE];

uint32_t nextScanMillis = 0;
uint32_t lastSendMillis = 0;
uint32_t sendCounter = 0;

bool testing = false;

void onConnect(AsyncEventSourceClient *client) 
{
  Logger.log(Debug, "sse", "New client connected");
  memset(jsonBuffer, 0, JSON_DOC_SIZE);
  doc.clear();
  vehicle->metricsToJson(doc);
  serializeJson(doc, jsonBuffer);

  client->send(jsonBuffer, NULL, millis());
}

void onStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Logger.log(Debug, "wifi", "Connected to %s", info.wifi_sta_connected.ssid);
}

void setup() 
{
  Serial.begin(115200);
  delay(500);

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
  
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(false);
  WiFi.onEvent(onStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

  WiFi.softAPConfig(localIP, localIP, subnet);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, 1);

  events.onConnect(onConnect);
  server.addHandler(&events);

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

  server.on("/api/test/stress", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    testing = !testing;

    request->send(200, "text/plain", testing ? "Stress Test Started" : "Stress Test Stopped");
  });

  ElegantOTA.begin(&server);
  server.begin();
  
  // Wait for everything to initialize.
  delay(100);
}

void loop() 
{
  vehicle->update();
  
  uint32_t now = millis();
  
  /* TEMPORARILY REMOVED TO FIX QUEUE ERROR
  if (now >= nextScanMillis && !WiFi.isConnected() && !vehicle->active)
  {
    Logger.log(Debug, "wifi", "Scanning for home network...");
    WiFi.scanNetworks(true, true, true, 300U, 0U, WIFI_HOME_SSID);
    nextScanMillis = now + WIFI_SCAN_INTERVAL;
  }

  int8_t totalNetworks = WiFi.scanComplete();
  if (totalNetworks > 0)
  {
    Logger.log(Debug, "wifi", "Found %d network(s)", totalNetworks);
    Logger.log(Debug, "wifi", "Connecting to %s...", WIFI_HOME_SSID);
    WiFi.begin(WIFI_HOME_SSID, WIFI_HOME_PASSWORD);
    WiFi.scanDelete();
  }
  */

  if (now - lastSendMillis >= SEND_INTERVAL && events.count() > 0)
  {
    doc.clear();

    if (testing)
    {
      VehicleNissanLeaf* leaf = (VehicleNissanLeaf*) vehicle;

      leaf->powered->setValue(1);
      leaf->gear->setValue(3);
      
      float speed = leaf->speed->value + 1;
      float power = leaf->batteryPower->value + 1;
      int32_t range = leaf->range->value + 1;

      if (speed > 100) speed = 0;
      if (power > 80) power = -20;
      if (range > 80) range = 0;

      leaf->speed->setValue(speed);
      leaf->batteryPower->setValue(power);
      leaf->range->setValue(range);
    }

    if (++sendCounter >= 50)
    {
      vehicle->metricsToJson(doc);
      sendCounter = 0;
      //Logger.log(Debug, "ram", "Free Heap: %u", ESP.getFreeHeap());
    } 
    else 
    {
      vehicle->getUpdatedMetrics(doc, now - SEND_INTERVAL);
    }
   
    if (!doc.isNull())
    {
      memset(jsonBuffer, 0, JSON_DOC_SIZE);
      serializeJson(doc, jsonBuffer);
      events.send(jsonBuffer, NULL, millis());
    }

    lastSendMillis = now;
  }

  //ws.cleanupClients();
}