#pragma once

#include <BLECharacteristic.h>

class BluetoothConfig
{
  public:
    void begin();
  
  private:
    BLECharacteristic *vehicleIdCharacteristic;
};

extern BluetoothConfig GlobalBluetoothConfig;