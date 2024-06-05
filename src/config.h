#pragma once

#include <Preferences.h>

#define CONFIG_DEVICE_NAME_MAX_LEN 29

class Config
{
  public:
    void begin();

    void writeBluetoothMode(uint8_t mode);
    uint8_t getBluetoothMode();

    void writeDeviceName(const char *name);
    char *getDeviceName();

    void writeBluetoothPin(uint32_t pin);
    uint32_t getBluetoothPin();

    void writeVehicleId(uint16_t id);
    uint16_t getVehicleId();
  
  private:
    Preferences prefs;
    uint16_t vehicleId;
    char deviceName[CONFIG_DEVICE_NAME_MAX_LEN] = "CANDLE-";
    uint8_t bluetoothMode;
    uint32_t bluetoothPin;
};

extern Config GlobalConfig;