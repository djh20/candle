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

void Vehicle::update(DynamicJsonDocument &doc)
{
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
}

void Vehicle::metricsToJson(DynamicJsonDocument &doc)
{
  for (int i = 0; i < totalMetrics; i++)
  {
    Metric* metric = metrics[i];
    metric->addToJsonDoc(doc);
  }
}

void Vehicle::processFrame(uint8_t &busId, long unsigned int &frameId, byte *frameData) {}
