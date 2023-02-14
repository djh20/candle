#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "vehicle/vehicle_nissan_leaf.h"
#include "config.h"

#define JSON_DOC_SIZE 1024
#define SEND_INTERVAL 50

IPAddress localIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Vehicle* vehicle;

DynamicJsonDocument doc(JSON_DOC_SIZE);
char jsonBuffer[JSON_DOC_SIZE];

uint32_t lastConnectMillis = 0;
uint32_t lastSendMillis = 0;
uint32_t sendCounter = 0;

void reconnectToWifi()
{
  uint32_t now = millis();

  // Only connect again after 10 seconds.
  if (now - lastConnectMillis < 10000) return;

  Serial.printf("Reconnecting to %s...\n", WIFI_HOME_SSID);

  lastConnectMillis = now;
  WiFi.reconnect();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
{
  if (type == WS_EVT_CONNECT) 
  {
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());

    memset(jsonBuffer, 0, JSON_DOC_SIZE);
    doc.clear();
    vehicle->metricsToJson(doc);
    serializeJson(doc, jsonBuffer);

    client->text(jsonBuffer);
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
  }
}

void setup() 
{
  Serial.begin(9600);
  delay(200);

  Serial.println();

  vehicle = new VehicleNissanLeaf();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(false);

  Serial.printf("Connecting to %s...\n", WIFI_HOME_SSID);
  WiFi.begin(WIFI_HOME_SSID, WIFI_HOME_PASSWORD);

  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, 1);

  ws.onEvent(onEvent);
  server.addHandler(&ws);

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

    leaf->powered->setValue(1);
    leaf->gear->setValue(4);

    float speed = leaf->speed->value + 1;

    leaf->speed->setValue(speed);
    leaf->leftSpeed->setValue(speed);
    leaf->rightSpeed->setValue(speed);

    request->send(200, "text/plain", "Test");
  });
  server.begin();
}

void loop() 
{
  if (vehicle->idle && !WiFi.isConnected()) reconnectToWifi();
  
  vehicle->readAndProcessBusData();
  
  uint32_t now = millis();

  if (now - lastSendMillis > SEND_INTERVAL)
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