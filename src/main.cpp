#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "vehicle/vehicle_nissan_leaf.h"

#define JSON_DOC_SIZE 1024

const char* ssid = "Leaf";
const char* password = "candle123";

IPAddress localIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Vehicle* vehicle;

DynamicJsonDocument doc(JSON_DOC_SIZE);
char jsonBuffer[JSON_DOC_SIZE];

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
  delay(50);

  Serial.println();

  vehicle = new VehicleNissanLeaf();

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(ssid, password, 1, 1);

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
  server.begin();
}

void loop() 
{
  doc.clear();
  vehicle->update(doc);

  if (!doc.isNull()) 
  {
    memset(jsonBuffer, 0, JSON_DOC_SIZE);
    serializeJson(doc, jsonBuffer);
    ws.textAll(jsonBuffer);
  }

  ws.cleanupClients();
}