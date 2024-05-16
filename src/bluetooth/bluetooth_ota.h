#pragma once

#include <Arduino.h>

namespace BluetoothOTA
{
  void begin();
  void loop();
  
  void processCommand(uint8_t *data, size_t length);
  void writeFirmware(uint8_t *data, size_t length);
}