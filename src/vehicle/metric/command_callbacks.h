#pragma once

#include "metric.h"
#include "../vehicle.h"
#include <BLECharacteristic.h>

class CommandCallbacks: public BLECharacteristicCallbacks 
{
  public:
    CommandCallbacks(Vehicle *vehicle);

    void onWrite(BLECharacteristic *pCharacteristic);

  private:
    Vehicle *vehicle;
};