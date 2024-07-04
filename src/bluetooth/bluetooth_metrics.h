#pragma once

#include "vehicle/vehicle.h"
#include <BLECharacteristic.h>

#define GROUPED_METRIC_DATA_LEN 64
// #define GROUPED_METRIC_INFO_LEN 96
#define GROUPED_METRIC_INFO_LEN 256

class BluetoothMetrics
{
  public:
    void begin();
    void loop();

  private:
    Vehicle *vehicle;
    BLECharacteristic *characteristics[16];
    uint8_t totalCharacteristics = 0;
    uint8_t characteristicValueBuffer[GROUPED_METRIC_DATA_LEN];
    uint32_t lastUpdateMillis = 0;
};

extern BluetoothMetrics GlobalBluetoothMetrics;