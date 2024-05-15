#pragma once

#include <Arduino.h>

namespace BluetoothOTA
{
  void begin();
  void processCommand(uint8_t *data, size_t length);
}