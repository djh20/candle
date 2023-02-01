#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "vehicle/vehicle_nissan_leaf.h"

const char* ssid = "Leaf";
const char* password = "candle123";

IPAddress localIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Vehicle* vehicle;

/*
void handleRequest() {
  DynamicJsonDocument doc(1024);
  doc["gps_position"][0] = -53.753251;+
  doc["gps_position"][1] = 127.385923;

  char json[256];
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}
*/

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
{
  if (type == WS_EVT_CONNECT) 
  {
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    char json[1024];
    vehicle->metricsToJson(json, 1024);
    client->text(json);
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

  WiFi.begin("HannWiFi", "yellowyellow21");

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(ssid, password, 1, 1);
  //Serial.println(WiFi.softAPIP());

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  //server.on("/", handleRequest);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    //AsyncWebParameter* idParam = request->getParam("id");
    //AsyncWebParameter* valueParam = request->getParam("value");

    //Serial.print("Total Metrics: ");
    //Serial.println(vehicle->totalMetrics);

    //for (int i = 0; i < vehicle->totalMetrics; i++)
    //{
    //  Metric* metric = vehicle->metrics[i];
    //}

    //MetricFloat* powerOutput = (MetricFloat*) vehicle->metrics[2];
    //powerOutput->setValue(powerOutput->value + 0.05);

    request->send(200, "application/json", "[]");
  });
  server.begin();
}

void loop() 
{
  char json[1024];
  bool jsonNotEmpty = vehicle->update(json, 1024);

  if (jsonNotEmpty) 
  {
    //Serial.println(json);
    ws.textAll(json);
  }

  ws.cleanupClients();
}