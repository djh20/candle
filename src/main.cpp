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

VehicleNissanLeaf* vehicle; // Change back to Vehicle*

uint32_t lastUpdateMillis = 0;

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
  delay(200);

  Serial.println();

  vehicle = new VehicleNissanLeaf();
  vehicle->powered->setValue(1);
  vehicle->gear->setValue(4);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(ssid, password, 1, 0);

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
    /*
    memset(jsonBuffer, 0, JSON_DOC_SIZE);
    doc.clear();
    vehicle->metricsToJson(doc);
    serializeJson(doc, jsonBuffer);

    request->send(200, "application/json", jsonBuffer);
    */
    //i = 0;
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      //Write up to "maxLen" bytes into "buffer" and return the amount written.
      //index equals the amount of bytes that have been already sent
      //You will be asked for more data until 0 is returned
      //Keep in mind that you can not delay or yield waiting for more data!
      //if (i >= 32) return 0;
      //i++;
      if (index >= 524288) return 0;
      memset(buffer, 97, maxLen);
      return maxLen;
    });
    request->send(response);

  });
  server.begin();
}

void loop() 
{
  int32_t now = millis();
  
  if ((now - lastUpdateMillis) >= 20)
  {
    float speed = vehicle->rearWheelSpeed->value + 1;
    if (speed >= 100) {
      speed = 0;
    }
    vehicle->rearWheelSpeed->setValue(speed);
    vehicle->leftWheelSpeed->setValue(speed);
    vehicle->rightWheelSpeed->setValue(speed);
    lastUpdateMillis = now;
  }

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