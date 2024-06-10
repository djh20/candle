#pragma once

#include "metric/metric.h"
#include "can/can_bus.h"
#include "can/poll_task.h"
#include <mcp_can.h>

#define VEHICLE_MAX_BUSES 4
#define VEHICLE_MAX_METRICS 64
#define VEHICLE_MAX_TASKS 16

class Vehicle 
{
  public:
    Vehicle();

    void begin();
    void loop();

    void registerTask(PollTask *task);
    void setMonitoredMessageId(uint16_t id);

    MetricInt *awake;

    CanBus *buses[VEHICLE_MAX_BUSES];
    uint8_t totalBuses = 0;

    Metric *metrics[VEHICLE_MAX_METRICS];
    uint8_t totalMetrics = 0;

    PollTask *tasks[VEHICLE_MAX_TASKS];
    uint8_t totalTasks = 0;

  protected:
    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void processBusData();
    void processTasks();
    virtual void registerAll();
    virtual void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    virtual void processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames);
    virtual void updateExtraMetrics();
    virtual void metricUpdated(Metric *metric);
    virtual void testCycle();

    PollTask *currentTask = NULL;
    uint8_t currentTaskIndex = 0;

  private:
    uint32_t lastTestCycleMillis = 0;
    uint16_t monitoredMessageId = 0xFFFE;
};