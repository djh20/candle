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

  updateServer.setup(&server);
  
  server.begin();
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
    const char* command = request->getParam("command")->value().c_str();
    GlobalConsole.processString(command);
    request->send(200, "text/plain", "OK");
  }
  else
  {
    request->send(400, "text/plain", "ERROR: Missing command parameter");
  }
}

void WiFiWebServer::onTaskRequest(AsyncWebServerRequest *request)
{
  if (request->hasParam("id"))
  {
    const char* taskId = request->getParam("id")->value().c_str();
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();

    if (vehicle)
    {
      Task *task = vehicle->getTask(taskId);
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
    request->send(400, "text/plain", "ERROR: Missing id parameter");
  }
}

WiFiWebServer GlobalWiFiWebServer;