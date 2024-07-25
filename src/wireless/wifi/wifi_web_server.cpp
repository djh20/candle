#include "wifi_web_server.h"
#include "../../vehicle/vehicle_manager.h"

void WiFiWebServer::begin()
{
  server.on("/api/vehicle/task", HTTP_POST, [](AsyncWebServerRequest *request)
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
  });

  server.begin();
}

void WiFiWebServer::end()
{
  server.end();
}

WiFiWebServer GlobalWiFiWebServer;