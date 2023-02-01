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
    bool update(char *jsonBuffer, uint16_t jsonBufferSize);
    void metricsToJson(char *jsonBuffer, uint16_t jsonBufferSize);
    virtual void processFrame(uint8_t &busId, long unsigned int &frameId, byte *frameData);
    
    CanBus *busses[8];
    int totalBusses;

    Metric *metrics[64];
    int totalMetrics;
};

#endif
