#pragma once

#include "metric/metric_int.h"
#include "metric/metric_string.h"

class Config
{
  public:
    void begin();
    
    StringMetric<1> *hostname;
    StringMetric<1> *vehicleId;
    IntMetric<1> *blePin;
    StringMetric<4> *wifiNetworks;
    StringMetric<4> *wifiPasswords;

    uint8_t getBluetoothMode();
};

extern Config GlobalConfig;