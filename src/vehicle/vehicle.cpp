#include "vehicle.h"
#include "Arduino.h"

Vehicle::Vehicle() {}

void Vehicle::registerBus(CanBus* bus)
{
  Serial.print("Registering bus ");
  Serial.print(bus->id);
  Serial.println(":");

  bus->init();
  busses[totalBusses++] = bus;

  Serial.println();
}

void Vehicle::registerMetric(Metric* metric) 
{
  Serial.print("Registering metric: ");
  Serial.println(metric->id);

  metrics[totalMetrics++] = metric;
}

bool Vehicle::update(char *jsonBuffer, uint16_t jsonBufferSize)
{
  DynamicJsonDocument doc(jsonBufferSize);
  
  for (int i = 0; i < totalBusses; i++) 
  {
    CanBus* bus = busses[i];
    bool gotFrame = bus->readFrame();
    if (gotFrame) processFrame(bus->id, bus->frameId, bus->frameData);
  }

  uint32_t now = millis();

  for (int i = 0; i < totalMetrics; i++)
  {
    Metric* metric = metrics[i];
    bool hasUpdated = metric->update(now); 
    if (hasUpdated) 
    {
      metric->addToJsonDoc(doc);
    }
  }

  if (doc.isNull()) return false;

  serializeJson(doc, jsonBuffer, jsonBufferSize);
  return true;
}

void Vehicle::metricsToJson(char *jsonBuffer, uint16_t jsonBufferSize)
{
  DynamicJsonDocument doc(jsonBufferSize);

  for (int i = 0; i < totalMetrics; i++)
  {
    Metric* metric = metrics[i];
    metric->addToJsonDoc(doc);
  }

  serializeJson(doc, jsonBuffer, jsonBufferSize);
}

void Vehicle::processFrame(uint8_t &busId, long unsigned int &frameId, byte *frameData) {}
