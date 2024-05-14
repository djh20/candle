#pragma once

#include <Preferences.h>

namespace Config
{
  void begin();

  void writeBluetoothMode(uint8_t mode);
  uint8_t getBluetoothMode();

  void writeBluetoothName(const char *name);
  char *getBluetoothName();

  void writeBluetoothPin(uint32_t pin);
  uint32_t getBluetoothPin();

  void writeVehicleId(uint16_t id);
  uint16_t getVehicleId();
}