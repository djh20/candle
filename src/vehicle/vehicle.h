#pragma once

#include "../metric/metric.h"
#include "../metric/metric_int.h"
#include "can/can_bus.h"
#include "can/task.h"
// #include "can/poll_task.h"

#define VEHICLE_MAX_BUSES 4
#define VEHICLE_MAX_TASKS 16

class Vehicle 
{
  public:
    Vehicle(const char* domain);

    virtual void begin();
    void loop();
    
    void runTask(Task *task);
    virtual void performAction(uint8_t action);

    IntMetric *ignition;

    CanBus *buses[VEHICLE_MAX_BUSES];
    uint8_t totalBuses = 0;

    Task *taskQueue[VEHICLE_MAX_TASKS];
    uint8_t totalTasksInQueue = 0;

  protected:
    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void registerMetrics(std::initializer_list<Metric*> metrics);

    void handleBuses();
    void handleTasks();
    virtual void processFrame(CanBus *bus, const uint32_t &id, uint8_t *data);
    // virtual void processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames);
    virtual void updateExtraMetrics();
    virtual void metricUpdated(Metric *metric);
    virtual void testCycle();

    Task *currentTask = NULL;
    const char *domain;

  private:
    uint32_t lastTestCycleMillis = 0;
};