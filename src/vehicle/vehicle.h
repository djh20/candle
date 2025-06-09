#pragma once

#include "../metric/metric.h"
#include "../metric/metric_int.h"
#include "can/can_bus.h"
#include "can/task.h"
#include "can/poll_task.h"

#define VEHICLE_MAX_BUSES 4
#define VEHICLE_MAX_TASKS 16

class Vehicle: protected TaskCallbacks
{
  public:
    Vehicle(const char* domain);

    virtual void begin();
    virtual void loop();
    
    void runTask(Task *task);
    Task *getTask(const char *id);
    bool isTaskInQueue(Task *task);

    virtual void runHomeTasks();

    IntMetric<1> *ignition;

    CanBus *buses[VEHICLE_MAX_BUSES];
    uint8_t totalBuses = 0;

    Task *tasks[VEHICLE_MAX_TASKS];
    uint32_t taskIntervals[VEHICLE_MAX_TASKS] = {};
    uint8_t totalTasks = 0;

    Task *taskQueue[VEHICLE_MAX_TASKS];
    uint8_t totalTasksInQueue = 0;

  protected:
    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void registerMetrics(std::initializer_list<Metric*> metrics);
    void registerTask(Task *task);
    void setTaskInterval(Task *task, uint32_t interval);
    void clearTaskInterval(Task *task);

    void handleBuses();
    void handleTasks();
    virtual void processFrame(CanBus *bus, const uint32_t &id, uint8_t *data) {}
    virtual void onTaskRun(Task *task) {}
    virtual void onTaskEnd(Task *task) {}
    virtual bool onPollResponse(Task *task, uint8_t **frames) = 0;
    virtual void metricUpdated(Metric *metric) {}
    virtual void testCycle() {}

    Task *currentTask = NULL;
    const char *domain;

  private:
    uint32_t lastTestCycleMillis = 0;
};