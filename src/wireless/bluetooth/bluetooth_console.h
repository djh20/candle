#pragma once

#include <BLEService.h>

class BluetoothConsole: public BLECharacteristicCallbacks
{
  public:
    void begin();

  private:
    void onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param);

    BLECharacteristic *commandCharacteristic;
    BLECharacteristic *resultCharacteristic;
};

extern BluetoothConsole GlobalBluetoothConsole;