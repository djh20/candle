#pragma once

#include "vehicle/vehicle.h"
#include "bluetooth_manager.h"
#include <BLECharacteristic.h>
#include <BLEDescriptor.h>

// #define GROUPED_METRIC_DATA_LEN 64
// #define GROUPED_METRIC_INFO_LEN 96
// #define GROUPED_METRIC_INFO_LEN 256

struct MetricGroupInfo
{
  uint8_t id = 0;
  uint8_t metricType = 0;
  uint8_t metricIndex = 0;
  Metric *metric = nullptr;
  uint8_t characteristicSize = 0;
  uint8_t descriptorSize = 0;
};

class BluetoothMetrics
{
  public:
    void begin();
    void loop();

  private:
    bool newGroup(MetricGroupInfo &groupInfo);
    bool nextMetric(MetricGroupInfo &groupInfo);

    BLECharacteristic *characteristics[8] = {nullptr};
    uint8_t totalCharacteristics = 0;
    
    uint8_t attributeBuffer[BLE_MAX_ATTRIBUTE_SIZE];
    uint8_t attributeBufferIndex = 0;
    uint32_t lastUpdateMillis = 0;
};

extern BluetoothMetrics GlobalBluetoothMetrics;