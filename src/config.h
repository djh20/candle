#pragma once

#include "metric/metric_int.h"
#include "metric/metric_string.h"

class Config
{
  public:
    void begin();
    
    StringMetric *hostname;
    StringMetric *vehicleId;
    IntMetric *blePin;

    uint8_t getBluetoothMode();
};

extern Config GlobalConfig;