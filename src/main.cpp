#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "vehicle/vehicle_nissan_leaf.h"

const char* ssid = "ESP8266";
const char* password = "password";

IPAddress localIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Vehicle* vehicle;

/*
void handleRequest() {
  DynamicJsonDocument doc(1024);
  doc["gps_position"][0] = -53.753251;
  doc["gps_position"][1] = 127.385923;

  char json[256];
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}
*/

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
{
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        client->text("[\"test\"]");
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
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
    Serial.print("Total Metrics: ");
    Serial.println(vehicle->totalMetrics);

    vehicle->logMetrics();
    
    request->send(200, "text/html", "<h1>CANdle is running!</h1>");
  });
  server.begin();
}

void loop() 
{
  //server.handleClient();
  vehicle->update();
  ws.cleanupClients();
}