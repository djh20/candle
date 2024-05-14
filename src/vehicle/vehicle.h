#pragma once

#include "metric/metric.h"
#include "can/can_bus.h"
#include "can/poll_task.h"
#include <mcp_can.h>

#define VEHICLE_MAX_BUSSES 4
#define VEHICLE_MAX_METRICS 64
#define VEHICLE_MAX_TASKS 16

class Vehicle 
{
  public:
    Vehicle();

    void begin();
    void loop();

    MetricInt *awake;
    MetricFloat *tripDistance;

    CanBus *busses[VEHICLE_MAX_BUSSES];
    uint8_t totalBusses = 0;

    Metric *metrics[VEHICLE_MAX_METRICS];
    uint8_t totalMetrics = 0;

    PollTask *tasks[VEHICLE_MAX_TASKS];
    uint8_t totalTasks = 0;

  protected:
    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void registerTask(PollTask *task);
    void processBusData();
    void handleTasks();
    virtual void registerAll();
    virtual void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    virtual void processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]);
    virtual void updateExtraMetrics();
    virtual void metricUpdated(Metric *metric);
    virtual void testCycle();

    PollTask *currentTask = NULL;
    uint8_t currentTaskIndex = 0;

  private:
    uint32_t lastTestCycleMillis = 0;
};