#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#include "metric/metric.h"
#include "can/can_bus.h"
#include <mcp_can.h>
#include <ArduinoJson.h>

class Vehicle 
{
  public:
    Vehicle();

    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void metricsToJson(DynamicJsonDocument &doc);
    void readAndProcessBusData();
    void getUpdatedMetrics(DynamicJsonDocument &updatedMetrics, uint32_t sinceMillis);
    virtual void processFrame(uint8_t &busId, long unsigned int &frameId, byte *frameData);

    bool idle = true;
    
    CanBus *busses[8];
    int totalBusses;

    Metric *metrics[64];
    int totalMetrics;
};

#endif
