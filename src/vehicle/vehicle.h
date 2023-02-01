#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#include "metric/metric.h"
#include "can/can_bus.h"
#include <map>
#include <mcp_can.h>

class Vehicle 
{
  public:
    Vehicle();

    void registerBus(CanBus* bus);
    void registerMetric(Metric* metric);
    void logMetrics();
    void update();
    virtual void processFrame(uint8_t &busId, long unsigned int &frameId, unsigned char *frameData);
    
    //Metric* metrics[64] = {};

    CanBus* busses[8];
    //std::map<const char*, MCP_CAN*> busses;
    int totalBusses;

    std::map<const char*, Metric*> metrics;
    std::map<const char*, Metric*>::iterator metricsIt;
    int totalMetrics;
};

#endif
