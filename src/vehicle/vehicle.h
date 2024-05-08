#pragma once

#include "metric/metric.h"
#include "can/can_bus.h"
#include "can/poll_task.h"
#include "../ble/uuid.h"
#include <mcp_can.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

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
    uint8_t characteristicValueBuffer[BLE_LEN_GROUPED_METRIC_DATA];

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
