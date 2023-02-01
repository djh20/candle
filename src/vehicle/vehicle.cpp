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
  Serial.print(metric->id);
  Serial.print(" ... ");

  metrics[metric->id] = metric;
  totalMetrics++;

  Serial.println("[OK]");
}

void Vehicle::logMetrics() 
{
  for (metricsIt = metrics.begin(); metricsIt != metrics.end(); metricsIt++)
  {
    Serial.println(metricsIt->second->id);
  }  
}

void Vehicle::update() 
{
  for (int i = 0; i < totalBusses; i++) {
    CanBus* bus = busses[i];
    bool gotFrame = bus->readFrame();
    if (gotFrame) processFrame(bus->id, bus->frameId, bus->frameData);
  }
}

void Vehicle::processFrame(uint8_t &busId, long unsigned int &frameId, unsigned char *frameData) {}