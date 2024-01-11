#pragma once

#include "metric.h"
#include <BLECharacteristic.h>

class MetricBLECallbacks: public BLECharacteristicCallbacks 
{
  public:
    MetricBLECallbacks(Metric *metric);

    void onWrite(BLECharacteristic *pCharacteristic);

  private:
    Metric *metric;
};