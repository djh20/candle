#pragma once

#include "metric/metric.h"
#include "can/can_bus.h"
#include "can/poll_task.h"
#include "gps/gps.h"
#include <mcp_can.h>
#include <ArduinoJson.h>

class Vehicle 
{
  public:
    Vehicle();

    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void registerGps(Gps *gps);
    void registerTask(PollTask *task);
    void metricsToJson(DynamicJsonDocument &doc);
    void update();
    void readAndProcessBusData();
    void getUpdatedMetrics(DynamicJsonDocument &updatedMetrics, uint32_t sinceMillis);
    void handleTasks();
    virtual void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    virtual void processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]);
    virtual void updateExtraMetrics();
    virtual void metricUpdated(Metric *metric);

    bool active = false;
    bool moving = false;
    
    CanBus *busses[8];
    uint8_t totalBusses = 0;

    Metric *metrics[64];
    uint8_t totalMetrics = 0;

    PollTask *tasks[16];
    uint8_t totalTasks = 0;

    Gps *gps = NULL;

  protected:
    PollTask *currentTask = NULL;
    uint8_t currentTaskIndex = 0;
};
