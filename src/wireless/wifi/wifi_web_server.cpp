#include "wifi_web_server.h"
#include "../../vehicle/vehicle_manager.h"
#include "../../metric/metric_manager.h"
#include <console.h>

JsonDocument WiFiWebServer::doc;

void WiFiWebServer::begin()
{
  server.on("/api/version", HTTP_GET, onVersionRequest);
  server.on("/api/metrics", HTTP_GET, onMetricsRequest);
  server.on("/api/console", HTTP_POST, onConsoleRequest);
  server.on("/api/vehicle/task", HTTP_POST, onTaskRequest);

  server.begin();
}

void WiFiWebServer::end()
{
  server.end();
}

void WiFiWebServer::onVersionRequest(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");

  doc["firmware"] = FIRMWARE_VERSION;
  doc["hardware"] = HARDWARE_MODEL;

  serializeJson(doc, *response);
  request->send(response);
  doc.clear();
}

void WiFiWebServer::onMetricsRequest(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");

  for (uint8_t i = 0; i < GlobalMetricManager.totalMetrics; i++)
  {
    Metric *metric = GlobalMetricManager.metrics[i];
    metric->getState(doc);
  }

  serializeJson(doc, *response);
  request->send(response);
  doc.clear();
}

void WiFiWebServer::onConsoleRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam("command"))
  {
    AsyncWebParameter *commandParam = request->getParam("command");
    GlobalConsole.processString(commandParam->value().c_str());
    request->send(200, "OK");
  }
  else
  {
    request->send(400, "ERROR: Missing parameter");
  }
}

void WiFiWebServer::onTaskRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam("id"))
  {
    AsyncWebParameter *idParam = request->getParam("id");
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();

    if (vehicle)
    {
      Task *task = vehicle->getTask(idParam->value().c_str());
      if (task)
      {
        vehicle->runTask(task);
        request->send(200, "text/plain", "OK");
      }
      else
      {
        request->send(400, "text/plain", "ERROR: Failed to find task");
      }
    }
    else
    {
      request->send(500, "text/plain", "ERROR: Failed to find vehicle");
    }
  }
  else
  {
    request->send(400, "text/plain", "ERROR: Missing parameter");
  }
}

WiFiWebServer GlobalWiFiWebServer;