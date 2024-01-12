#pragma once

#include "metric/metric.h"
#include "can/can_bus.h"
#include "can/poll_task.h"
#include <mcp_can.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

#define METRICS_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define COMMAND_CHARACTERISTIC_UUID "5a1d631e-66de-4f28-8927-bf28ee1969da"
#define DESCRIPTOR_UUID 0x8C19
#define MAX_CHARACTERISTIC_VALUE_SIZE 64
#define MAX_DESCRIPTOR_VALUE_SIZE 100

class Vehicle 
{
  public:
    Vehicle();

    void init(BLEServer *bleServer);
    void registerBus(CanBus *bus);
    void registerMetric(Metric *metric);
    void registerTask(PollTask *task);
    void registerCharacteristic(BLECharacteristic *characteristic);
    void update();
    void readAndProcessBusData();
    void sendUpdatedMetrics(uint32_t sinceMillis);
    void handleTasks();
    virtual void registerAll();
    virtual void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    virtual void processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]);
    virtual void updateExtraMetrics();
    virtual void metricUpdated(Metric *metric);

    MetricInt *awake;
    MetricFloat *tripDistance;

    BLECharacteristic *metricCharacteristics[16];
    uint8_t totalMetricCharacteristics = 0;
    uint8_t characteristicValueBuffer[MAX_CHARACTERISTIC_VALUE_SIZE];

    CanBus *busses[8];
    uint8_t totalBusses = 0;

    Metric *metrics[64];
    uint8_t totalMetrics = 0;

    PollTask *tasks[16];
    uint8_t totalTasks = 0;

  protected:
    BLEServer *bleServer = NULL;
    BLEService *bleService = NULL;
    PollTask *currentTask = NULL;
    uint8_t currentTaskIndex = 0;
};
