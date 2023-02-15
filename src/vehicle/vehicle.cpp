#include "vehicle.h"
#include <Arduino.h>
#include "../utils/logger.h"

Vehicle::Vehicle() {}

void Vehicle::registerBus(CanBus *bus)
{
  bool initialized = bus->init();

  if (!initialized) {
    Logger.log(Error, "vehicle", "Failed to register bus %u", bus->id);
    return;
  }

  busses[totalBusses++] = bus;
  Logger.log(Info, "vehicle", "Registered bus %u", bus->id);
}

void Vehicle::registerMetric(Metric *metric) 
{
  metrics[totalMetrics++] = metric;

  metric->onUpdate([this, metric]() {
    metricUpdated(metric);
  });

  Logger.log(Info, "vehicle", "Registered metric: %s", metric->id);
}

void Vehicle::readAndProcessBusData()
{
  for (int i = 0; i < totalBusses; i++) 
  {
    CanBus* bus = busses[i];
    bool gotFrame = bus->readFrame();
    if (gotFrame) processFrame(bus->id, bus->frameId, bus->frameData);
  }
}

void Vehicle::getUpdatedMetrics(DynamicJsonDocument &updatedMetrics, uint32_t sinceMillis)
{
  for (int i = 0; i < totalMetrics; i++)
  {
    Metric* metric = metrics[i];
    bool updated = (metric->lastUpdateMillis >= sinceMillis); 
    if (updated) 
    {
      metric->addToJsonDoc(updatedMetrics);
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
void Vehicle::updateExtraMetrics() {}
void Vehicle::metricUpdated(Metric *metric) {}
